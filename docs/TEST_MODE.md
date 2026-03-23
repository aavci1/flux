# Flux `--test-mode`: UI testing and debugging

This document describes how to run Flux applications in **test mode**, what it enables, and how to drive the app remotely for automated tests, screenshots, and debugging.

## What test mode does

When you pass `--test-mode` on the command line, the `Application` constructor sets an internal flag. After the first window is created, that window calls `Window::enableTestMode`, which:

1. **Enables GPU readback** on the platform window so frames can be copied for screenshots (on backends that support it, this avoids the cheaper framebuffer path that skips readback).
2. **Starts a binary IPC server** (`TestServer`) that listens on either a **Unix domain socket** or a **TCP port**.
3. Installs **screen capture** used to fulfill screenshot requests.

The main event loop processes **synthetic input** (queued by the test server) before each render, so clicks, keys, scrolls, etc. behave like real user input.

On startup, the process prints where the IPC server is listening, for example:

```text
  FLUX TEST IPC (unix): /tmp/flux-test-12345-abc.sock
  Binary ops: GetUi GetScreenshot Click Type Key Scroll Hover Drag
```

or, when using TCP:

```text
  FLUX TEST IPC (tcp): localhost:8435
  Binary ops: GetUi GetScreenshot Click Type Key Scroll Hover Drag
```

---

## Command-line flags

| Flag | Meaning |
|------|---------|
| `--test-mode` | Enable test IPC and screenshot/UI snapshot support for windows created by this process. |
| `--test-port <n>` | TCP port for the test server (default **8435**). Ignored if `--test-socket` is set. |
| `--test-socket <path>` | Unix domain socket path to listen on. If set, **TCP is not used** for the test server. |
| `--backend <name>` | Graphics backend (e.g. `metal`). Same as non-test usage; tests can set `FLUX_TEST_BACKEND` when launching from Python (see below). |

**Order of operations:** Parse `Application` arguments first; when `createWindow` runs and `testMode_` is true, `enableTestMode(testPort_, testSocketPath_)` is invoked. Use `--test-socket` for isolated runs (unique path per process) to avoid bind collisions.

---

## Wire protocol (binary IPC)

The server accepts **one request per connection**: connect, send one request, read one response, then the server closes the client side. Each new operation should use a **new connection** (this matches the reference Python client).

### Request format

All little-endian.

| Field | Size | Description |
|-------|------|-------------|
| Magic | 4 bytes | `0x58554C46` (`'FLUX'` in memory on little-endian machines) |
| Version | 2 bytes | Must be **1** |
| Opcode | 2 bytes | Operation id (see below) |
| Body length | 4 bytes | Length of JSON body in bytes (0 if none) |
| Body | *variable* | UTF-8 JSON for ops that need parameters |

Maximum body size is **4 MiB**; larger requests receive an error response.

### Response format

| Field | Size | Description |
|-------|------|-------------|
| Status | 1 byte | **0** = success, **non-zero** = error |
| Payload type | 1 byte | **0** = UTF-8 JSON, **1** = raw PNG bytes (screenshot success only) |
| Reserved | 2 bytes | Zero |
| Body length | 4 bytes | Length of payload |
| Body | *variable* | JSON or PNG data |

On error, the body is typically JSON such as `{"error":"..."}`.

---

## Opcodes and behavior

| Opcode | Name | Request body | Success payload |
|--------|------|----------------|-----------------|
| 1 | **GetUi** | empty | JSON: accessibility-oriented **UI tree** snapshot (see below). |
| 2 | **GetScreenshot** | empty | **Payload type 1**: PNG image of the last rendered frame area. Triggers a redraw and waits for capture (see timeouts). |
| 3 | **Click** | `{"x": float, "y": float}` | JSON acknowledgment with coordinates. Synthesizes mouse down + up at `(x,y)`. |
| 4 | **Type** | `{"text": "..."}` | JSON acknowledgment. Injects **text input** (Unicode string) into the focused control. |
| 5 | **Key** | `{"key": "Name", "modifiers": [...]?}` | JSON acknowledgment. See [Key names](#key-names-for-op-key) and [Modifiers](#modifier-strings-for-op-key). |
| 6 | **Scroll** | `{"x", "y", "deltaX", "deltaY"}` (floats) | JSON acknowledgment. Synthetic scroll at position. |
| 7 | **Hover** | `{"x", "y"}` | JSON acknowledgment. Mouse move without click. |
| 8 | **Drag** | `{"startX", "startY", "endX", "endY", "steps"}` | JSON acknowledgment. Mouse down at start, interpolated hovers, mouse up at end. `steps` defaults to 10 (minimum 1). |

### Event ordering and synchronization

Synthetic events are queued with a monotonic sequence number. The server **blocks each RPC** until a frame has been processed that includes that event (`enqueueAndWait`), up to a **3 second** wait per operation.

Screenshot requests increment a sequence counter, request a redraw, and wait up to **5 seconds** for the PNG to be produced. Failures return JSON errors such as `screenshot timeout` or `no screenshot`.

---

## UI tree JSON (`GetUi`)

The tree mirrors the layout tree plus optional overlay entries. Each node is a JSON object with:

| Field | Meaning |
|-------|---------|
| `id` | String path id (e.g. `"0"`, `"0/1/2"`). |
| `type` | View type name from `getTypeName()`. |
| `bounds` | `{ "x", "y", "w", "h" }` in window coordinates (floats, serialized with one decimal place in the C++ serializer). |
| `text` | Present if `getTextContent()` is non-empty. |
| `value` | Present if `getAccessibleValue()` is non-empty. |
| `interactive` | `true` if the view reports as interactive. |
| `focusable` | `true` if the view can be focused. |
| `focusKey` | Optional accessibility focus key string when focusable. |
| `children` | Array of child nodes. |

**Overlays:** If any overlays are open, the root is serialized as a single synthetic root whose `children` include both the normal layout children and overlay layout trees (see `TestServer::serializeUITree` with `OverlayManager`).

**Empty tree:** Before the first valid layout, `GetUi` may return `{}`.

**Profiling:** Set environment variable `FLUX_TEST_PROFILE` to print stderr timing for `serializeUITree` on each frame (useful when debugging slow snapshots).

---

## Key names (for op Key)

Recognized names include:

- **Navigation / editing:** `Enter`, `Return`, `Tab`, `Backspace`, `Escape`, `Space`, `Delete`, `Insert`
- **Arrows:** `Left`, `Right`, `Up`, `Down`
- **Paging:** `Home`, `End`, `PageUp`, `PageDown`
- **Symbols:** `Equal`, `Plus`, `Minus`
- **Letters:** `A`–`Z` or `a`–`z` (mapped to `Key::A` …)
- **Digits:** `0`–`9` (mapped to `Key::Num0` …)

If the key string does not map (returns 0), behavior may be a no-op.

### Modifier strings (for op Key)

Optional `"modifiers": ["ctrl", "shift", …]` (JSON array of strings). Parsing is substring-based; recognized tokens include (case variants listed in code): `ctrl`/`control`, `shift`, `alt`/`option`, `super`/`meta`/`cmd`/`Command`.

When **modifiers are present** in the JSON, the key path uses full `KeyEvent` semantics (shortcut manager, focus navigation for Tab, synthetic key queue). When **modifiers are omitted**, the server uses a simpler down/up path via `handleKeyDown` / `handleKeyUp` with the raw code.

---

## Python reference client

The module `tests/ui/flux_test_client.py` implements the protocol and helpers used by the UI test suite.

### `FluxTestClient`

- **Constructor:** `FluxTestClient(port=8435, unix_socket=None)` — use **either** TCP (`port`) **or** `unix_socket`, not both for a given client instance.
- **Core methods:** `get_ui()`, `get_screenshot()`, `click(x, y)`, `type_text(text)`, `press_key(key, modifiers=None)`, `scroll(x, y, delta_x=0, delta_y=0)`, `hover(x, y)`, `drag(start_x, start_y, end_x, end_y, steps=10)`.
- **`wait_ready(timeout=15, interval=0.3)`** — polls until `get_ui()` returns a dict with a non-empty `children` list (retries `get_ui` up to 3 times with backoff on connection errors).

### Tree helpers

`find_all`, `find_first`, `find_by_type`, `find_by_text`, `find_by_text_containing`, `find_by_focus_key`, `get_bounds`, `center_of`, `get_text_value` — see docstrings in `flux_test_client.py`.

### `FluxAppProcess`

Launches: `executable --test-mode --test-socket <path>` with a generated socket under the system temp directory by default. Optional `extra_args` append after that. If `FLUX_TEST_BACKEND` is set in the environment, `--backend <value>` is added.

Use as a context manager or call `start()` / `stop()`. `stop()` sends SIGTERM, waits, then SIGKILL if needed, and unlinks the socket file.

---

## Environment variables

| Variable | Purpose |
|----------|---------|
| `FLUX_TEST_BACKEND` | When using `FluxAppProcess`, selects `--backend` for the child process. |
| `FLUX_TEST_PROFILE` | When set, logs `serializeUITree` duration to stderr each frame in test mode. |
| `FLUX_BUILD_DIR` | Used by `tests/ui/run_tests.py` and scripts like `save_svg_demo_screenshot.py` to locate built executables. |

---

## Running the UI test suite

`tests/ui/run_tests.py` configures CMake with UI tests and examples, builds targets, and runs Python `unittest` modules. Each module typically starts its own app with `--test-mode` and a private Unix socket.

Examples:

```bash
python3 tests/ui/run_tests.py                  # all suites
python3 tests/ui/run_tests.py test_button      # one module
python3 tests/ui/run_tests.py --build          # configure + build first
python3 tests/ui/run_tests.py --list           # list modules
```

### Screenshot helper

`tests/ui/save_svg_demo_screenshot.py` launches `svg_demo` in test mode, waits for UI readiness, saves `examples/svg-demo/verify-screenshot.png`.

---

## Implementation notes (for contributors)

- **Main loop:** `Application::exec` calls `window->processSyntheticEvents()` before `window->render()` when a redraw is pending, so injected events are applied before the next frame.
- **Screenshots:** After rendering, if a screenshot was requested, `ScreenCapture` runs, PNG is stored in `TestServer`, and frame completion is signaled for waiters.
- **Security:** Test mode opens a local socket or port; do not expose untrusted networks to it. It is intended for CI and developer machines.

---

## Quick start (manual TCP)

1. Build your app (e.g. `svg_demo`).
2. Run: `./svg_demo --test-mode --test-port 8435`
3. From Python (same machine):

```python
from tests.ui.flux_test_client import FluxTestClient

c = FluxTestClient(port=8435)
c.wait_ready()
print(c.get_ui())
open("shot.png", "wb").write(c.get_screenshot())
```

Or use a Unix socket for a single stable pattern:

```bash
./svg_demo --test-mode --test-socket /tmp/flux-test.sock
```

```python
c = FluxTestClient(unix_socket="/tmp/flux-test.sock")
```

---

## Related source files

| Area | Location |
|------|----------|
| CLI parsing | `src/Core/Application.cpp` |
| Test mode wiring / synthetic events | `src/Core/Window.cpp` |
| IPC server, protocol, UI serialization | `src/Testing/TestServer.hpp` |
| Python client & process helper | `tests/ui/flux_test_client.py` |
| Test runner | `tests/ui/run_tests.py` |
