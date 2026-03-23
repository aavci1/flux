# Debugging with Address Sanitizer (ASan)

Concise reference for reproducing and fixing heap issues in Flux (used to track down real crashes in `svg_demo` / overlays / fonts).

## 1. Build with ASan

Use a separate build directory so normal builds stay fast:

```bash
cmake -S . -B build-asan -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="-fsanitize=address -fno-omit-frame-pointer -g -O1" \
  -DCMAKE_C_FLAGS="-fsanitize=address -g -O1" \
  -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address"
cmake --build build-asan --target svg_demo -j
```

Optional: `detect_leaks=0` reduces noise if you only care about UAF/heap bugs:

```bash
export ASAN_OPTIONS=detect_leaks=0
```

For **deterministic abort** on first error (good for CI / scripts):

```bash
export ASAN_OPTIONS=abort_on_error=1:halt_on_error=1:detect_leaks=0
```

## 2. Run the app under test-mode

See [TEST_MODE.md](TEST_MODE.md) for flags and the IPC protocol. Typical pattern:

```bash
./build-asan/svg_demo --test-mode --test-socket /tmp/flux-asan.sock
```

Drive UI from Python (`tests/ui/flux_test_client.py`): `get_ui`, `click`, `scroll`, etc.

**Important:** If you spawn the process with `subprocess` and **`stderr=subprocess.PIPE`**, a verbose ASan report can **fill the pipe** and **deadlock** the child. Prefer logging stderr to a file:

```bash
./build-asan/svg_demo ... 2>/tmp/asan.log
```

## 3. Read the report

On failure, ASan prints to **stderr**:

- **`ERROR: AddressSanitizer: heap-use-after-free`** (or `heap-buffer-overflow`, etc.)
- **Allocation stack** — where the memory was allocated
- **Free stack** — where it was freed
- **Faulting stack** — where the bad read/write happened

Use the **faulting** frame to see the immediate bug (e.g. `Property::operator=`, `strlen`, overlay click). Use **alloc/free** to see **lifetime** mistakes (e.g. pointer captured across layout teardown, FreeType pathname).

## 4. Patterns we hit (for faster triage)

| Symptom / clue | Likely area |
|----------------|-------------|
| UAF in `strlen` / `std::string` ctor from a **path** after font load | `GlyphAtlas` / `FT_New_Face`: pathname must stay valid; store paths in `GlyphAtlas`, don’t rely on `face->stream->pathname.pointer` alone if the source string was temporary. |
| UAF in **`Property::operator=`** during overlay / select | Main layout was rebuilt while overlay handlers still reference **`this`** on views in the old tree. Fix: defer main layout while overlays are open; defer `hideOverlay` until after pointer dispatch; avoid rebuilding main `cachedLayoutTree_` in `renderFrame` while overlays exist. |
| Crash only under **stress** or **test-mode** automation | Often ordering: synthetic input + render + overlay. Reproduce with ASan + scripted clicks, not only manual UI. |

## 5. Minimal checklist

1. `build-asan` + `ASAN_OPTIONS=abort_on_error=1:halt_on_error=1`
2. Run target with `--test-mode` + Unix socket
3. Stress with `flux_test_client` (or a small loop of `click` / `scroll`)
4. Capture **stderr to a file** if using subprocess
5. Fix using **alloc / free / fault** stacks, then re-run until clean
