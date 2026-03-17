# Flux v2: Architecture & Implementation Plan

## 1. Introduction

Flux has strong foundations: C++23, designated initializers for component construction, concept-based views without inheritance, and a clean `Property<T>` API. These are worth preserving. The user-facing syntax --

```cpp
VStack {
    .spacing = 10,
    .children = {
        Text { .value = "Hello", .fontSize = 24 },
        Button { .text = "Click", .onClick = [&] { counter++; } }
    }
}
```

-- is the identity of the framework and should not change.

What needs to change is the **internal architecture** behind this syntax. The current implementation has problems that cannot be fixed incrementally: views are deep-cloned every frame, `body()` is evaluated 15+ times per component per frame, state changes trigger a global redraw of the entire tree, the event system is split across unrelated subsystems, the framework only runs on Linux/Wayland, and there are no tests. Patching these individually leads to a web of workarounds; addressing them together as a coherent redesign yields a framework that is simpler, faster, and runs everywhere.

This document is an implementation plan. It is meant to be followed top-to-bottom. Each phase builds on the previous one. Code sketches are illustrative, not prescriptive.

---

## 2. Target Architecture

### 2.1 High-Level Layers

```
┌─────────────────────────────────────────────────────┐
│  User Code                                          │
│  Structs with designated initializers, Property<T>, │
│  body(), render(), event callbacks                  │
├─────────────────────────────────────────────────────┤
│  View Descriptions (lightweight, per-frame)         │
│  Value types. No heap allocation. No state.         │
│  Produced by body(). Diffed against previous frame. │
├─────────────────────────────────────────────────────┤
│  Element Tree (persistent, cross-frame)             │
│  Owns state (Property<T> storage). Owns identity.   │
│  Matched to descriptions via structural identity.   │
│  Only dirty subtrees re-evaluate body().            │
├─────────────────────────────────────────────────────┤
│  Layout Engine (standalone, testable)               │
│  Constraints in, bounds out. No view dependencies.  │
│  Caches results. Invalidated per-element.           │
├─────────────────────────────────────────────────────┤
│  Event Dispatcher (unified pipeline)                │
│  Hit test → capture → target → bubble.              │
│  Hover, click, drag, focus state machines.          │
├─────────────────────────────────────────────────────┤
│  Render Backend (pluggable)                         │
│  Command buffer consumed by NanoVG / Skia / etc.    │
├─────────────────────────────────────────────────────┤
│  Platform (SDL3)                                    │
│  macOS, Windows, Linux (Wayland + X11)              │
│  Windowing, GL context, input, clipboard, DPI       │
└─────────────────────────────────────────────────────┘
```

### 2.2 Cross-Platform Strategy

The current Flux is welded to Wayland: ~1500 lines of `WaylandWindow.cpp` handling display connection, surface creation, EGL context, XKB keyboard mapping, pointer input, cursor themes, and DPI scaling. Replicating this natively for macOS (Cocoa + NSOpenGLContext) and Windows (Win32 + WGL) would triple the platform code and require deep expertise in three unrelated windowing APIs.

**The pragmatic solution: SDL3.**

SDL3 is the industry standard for cross-platform windowing in C/C++. It provides, on all three platforms:

| Capability | Current (Wayland-only) | SDL3 |
|-----------|----------------------|------|
| Window creation | `wl_compositor`, `xdg_surface` | `SDL_CreateWindow()` |
| OpenGL context | EGL manually configured | `SDL_GL_CreateContext()` |
| Keyboard input | Raw XKB keycodes, manual modifier tracking | `SDL_EVENT_KEY_DOWN/UP` with scancodes and keysyms |
| Mouse input | `wl_pointer` listeners | `SDL_EVENT_MOUSE_*` |
| Text input | XKB compose, manual | `SDL_EVENT_TEXT_INPUT` with IME support |
| Clipboard | Not implemented | `SDL_GetClipboardText()` / `SDL_SetClipboardText()` |
| Cursor shapes | `wl_cursor_theme` | `SDL_CreateSystemCursor()` |
| DPI / HiDPI | `wl_output` scale factor | `SDL_GetWindowDisplayScale()` |
| File dialogs | Not implemented | `SDL_ShowOpenFileDialog()` (SDL3) |
| Timers | `std::this_thread::sleep_for` | `SDL_AddTimer()` or keep chrono |
| Event loop | `wl_display_dispatch` | `SDL_PollEvent()` |

This replaces `WaylandWindow.cpp` (~1500 lines), the Wayland protocol generation, `wayland-scanner`, and all EGL setup with a single ~300-line `SDLWindow.cpp` that works on macOS, Windows, and Linux (both Wayland and X11, automatically). NanoVG continues to render via OpenGL, which SDL provides the context for.

**What about NanoVG + OpenGL on macOS?** OpenGL is deprecated on macOS since Catalina but still works and ships with the OS. For the foreseeable future, NanoVG + OpenGL via SDL3 runs fine on macOS. Long-term options if Apple removes OpenGL:
- ANGLE (translates GL → Metal, used by Chrome and Firefox on macOS)
- nanovg_metal (community Metal backend for NanoVG)
- Switch to a Skia backend (see Phase 5 render command buffer)

None of these need to block the initial cross-platform release.

### 2.3 SwiftUI Parallels

| SwiftUI | Flux v2 |
|---------|---------|
| `struct MyView: View` | `struct MyView { FLUX_VIEW_PROPERTIES; ... }` |
| `@State var count = 0` | `Property<int> count = 0;` (backed by element tree) |
| `@Binding var value: Int` | `Property<int>& value` (reference to parent's property) |
| `@Environment(\.colorScheme)` | `Environment<ColorScheme>` (propagated down tree) |
| `var body: some View` | `View body() const` |
| Attribute graph (internal) | Element tree (internal) |
| View identity (structural + `.id()`) | Structural identity + `.key()` |
| `GeometryReader` | Layout constraints passed to `body()` or `layout()` |
| `.modifier(...)` | Designated initializer fields (`.padding`, `.backgroundColor`, ...) |

### 2.4 What Stays

- Designated initializer syntax for component construction.
- `Property<T>` as the reactive primitive (API surface preserved).
- Plain structs as components, no inheritance required.
- `FLUX_VIEW_PROPERTIES` injecting common properties (reworked to be smaller).
- `body()` returning a `View` for composition.
- `render()` for custom drawing.
- NanoVG as the primary render backend.
- C++23 as the language standard.
- The existing component set (Text, Button, VStack, HStack, Grid, etc.).

### 2.5 What Changes

- `View` becomes a lightweight description, not a deep-copied type-erased object.
- A new **Element Tree** persists across frames and owns state.
- `Property<T>` mutations mark the owning element dirty (not global redraw).
- Layout becomes a standalone engine operating on constraint data.
- Event dispatch is unified into a single capture/bubble pipeline.
- `Application` singleton is replaced with an explicit `Runtime`.
- A render command buffer decouples tree traversal from GPU calls.
- **The entire platform layer is replaced with SDL3**, making Flux cross-platform (macOS, Windows, Linux).
- The Wayland-specific code (`WaylandWindow.cpp`, protocol generation, EGL setup) is removed.

---

## 3. Phase 1: Platform & Foundation

*Goal: Replace the Wayland-only platform layer with SDL3, replace the singleton, add logging. This is the scaffolding everything else builds on. Doing the platform switch first means every subsequent phase is cross-platform from the start.*

### 3.1 SDL3 Integration

**Current problem:** Flux only runs on Linux/Wayland. `WaylandWindow.cpp` is ~1500 lines of Wayland-specific code. `CMakeLists.txt` hard-fails on non-Linux. There is no clipboard, no IME, no file dialogs, and no way to run on macOS or Windows.

**Design:** Replace the Wayland backend with an SDL3 backend.

```cpp
class SDLWindow : public PlatformWindow {
    SDL_Window* window_ = nullptr;
    SDL_GLContext glContext_ = nullptr;

public:
    SDLWindow(WindowConfig config) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
        SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

        window_ = SDL_CreateWindow(
            config.title.c_str(),
            config.size.width, config.size.height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
        );
        glContext_ = SDL_GL_CreateContext(window_);
        SDL_GL_MakeCurrent(window_, glContext_);
        SDL_GL_SetSwapInterval(1);  // vsync
    }

    ~SDLWindow() {
        SDL_GL_DestroyContext(glContext_);
        SDL_DestroyWindow(window_);
    }

    void processEvents() override {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            translateAndDispatch(event);
        }
    }

    bool shouldClose() const override { return shouldClose_; }
    Size currentSize() const override { /* SDL_GetWindowSize */ }

    float dpiScale() const override {
        return SDL_GetWindowDisplayScale(window_);
    }

    void setCursor(CursorType cursor) override {
        SDL_SetCursor(SDL_CreateSystemCursor(toSDLCursor(cursor)));
    }

    void swapBuffers() override {
        SDL_GL_SwapWindow(window_);
    }

private:
    void translateAndDispatch(const SDL_Event& event);
    static SDL_SystemCursor toSDLCursor(CursorType cursor);
};
```

**Key mapping** is handled inside `SDLWindow::translateAndDispatch()`. SDL provides platform-independent scancodes (`SDL_SCANCODE_A`, etc.) and keysyms (`SDLK_a`, etc.). The translation from SDL keycodes to Flux's `Key` enum happens here -- the core framework never sees platform-specific codes.

```cpp
void SDLWindow::translateAndDispatch(const SDL_Event& sdlEvent) {
    switch (sdlEvent.type) {
        case SDL_EVENT_KEY_DOWN: {
            KeyEvent event;
            event.key = sdlKeyToFluxKey(sdlEvent.key.scancode);
            event.modifiers = sdlModToFluxMod(sdlEvent.key.mod);
            event.isRepeat = sdlEvent.key.repeat;
            fluxWindow_->handleKeyDown(event);
            break;
        }
        case SDL_EVENT_MOUSE_MOTION: {
            PointerEvent event;
            event.kind = PointerEvent::Kind::Move;
            event.windowPosition = {sdlEvent.motion.x, sdlEvent.motion.y};
            fluxWindow_->handlePointerEvent(event);
            break;
        }
        case SDL_EVENT_TEXT_INPUT: {
            KeyEvent event;
            event.kind = KeyEvent::Kind::TextInput;
            event.text = sdlEvent.text.text;
            fluxWindow_->handleTextInput(event);
            break;
        }
        // ... mouse button, scroll, window resize, close, etc.
    }
}
```

**SDL3 as a dependency:** Add SDL3 via CMake's FetchContent or as a system dependency:

```cmake
find_package(SDL3 REQUIRED)
# or:
FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.x
)
FetchContent_MakeAvailable(SDL3)

target_link_libraries(flux PRIVATE SDL3::SDL3)
```

**What gets deleted:**
- `src/Platform/WaylandWindow.cpp` (~1500 lines)
- `include/Flux/Platform/WaylandWindow.hpp`
- `include/Flux/Platform/WaylandProtocols.hpp`
- `include/Flux/Platform/WaylandCursor.hpp`
- Wayland protocol XML generation in `CMakeLists.txt`
- All `pkg_check_modules` for `wayland-client`, `wayland-egl`, `wayland-cursor`
- The `find_program(WAYLAND_SCANNER)` block
- EGL setup code

**What gets added:**
- `src/Platform/SDLWindow.cpp` (~300 lines)
- `include/Flux/Platform/SDLWindow.hpp`

Net reduction: ~1200 lines of platform code, plus the entire Wayland build dependency chain.

### 3.2 NanoVG with SDL3

NanoVG initializes against an OpenGL context. It does not care who created that context. Currently, Flux creates the GL context via EGL on Wayland. With SDL3, the context comes from `SDL_GL_CreateContext()`. The NanoVG init code barely changes:

```cpp
// Before (EGL):
eglMakeCurrent(display, surface, surface, context);
nvgContext_ = nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES);

// After (SDL):
SDL_GL_MakeCurrent(window, glContext);
// On macOS/Windows: use GL2 or GL3 (not GLES2)
#if defined(__APPLE__) || defined(_WIN32)
nvgContext_ = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
#else
nvgContext_ = nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
#endif
```

SDL3 can also request a GLES2 context on desktop (via `SDL_GL_CONTEXT_PROFILE_ES`), which would keep the NanoVG backend uniform. Either approach works.

### 3.3 Replace Application Singleton with Runtime

**Current problem:** `Application` is a singleton. `Property<T>` calls the global `requestApplicationRedraw()` on every mutation, coupling the entire property system to the singleton. You cannot instantiate `Property<T>` in tests, utilities, or embedded contexts without `Application` existing.

**Design:**

```cpp
class Runtime {
public:
    explicit Runtime(int argc = 0, char** argv = nullptr);
    ~Runtime();

    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;

    Window& createWindow(WindowConfig config);
    int run();
    void quit();

    // Internal: called by Property<T> when state changes
    void requestRedraw(Element* dirtyElement);

    // Internal: platform factory (SDL by default, headless for tests)
    void setPlatformFactory(std::unique_ptr<PlatformFactory> factory);

private:
    std::unique_ptr<PlatformFactory> platform_;
    std::vector<std::unique_ptr<Window>> windows_;
    ResourceManager resources_;
};
```

The main event loop uses SDL's event pump:

```cpp
int Runtime::run() {
    while (running_) {
        // SDL handles platform events for all windows
        for (auto& window : windows_) {
            window->platformWindow()->processEvents();
            if (window->platformWindow()->shouldClose()) {
                // handle close
            }
        }

        // Re-evaluate dirty elements, diff, re-layout, re-render
        if (hasDirtyElements()) {
            for (auto& window : windows_) {
                window->updateAndRender();
            }
        }

        // VSync handles frame pacing (SDL_GL_SetSwapInterval(1))
        // No manual sleep_for needed
    }
    return 0;
}
```

**Usage:**

```cpp
int main(int argc, char* argv[]) {
    Runtime runtime(argc, argv);
    auto& window = runtime.createWindow({.title = "Counter", .size = {400, 300}});
    window.setRootView(Counter{});
    return runtime.run();
}
```

### 3.4 Logging

**Current problem:** `std::cout` scattered throughout hot paths. No way to disable, filter, or redirect.

**Design:** A minimal compile-time-eliminable logging macro:

```cpp
enum class LogLevel { Trace, Debug, Info, Warn, Error, Off };

#ifdef NDEBUG
#define FLUX_LOG_TRACE(...)
#define FLUX_LOG_DEBUG(...)
#else
#define FLUX_LOG_TRACE(...) flux::detail::log(LogLevel::Trace, __VA_ARGS__)
#define FLUX_LOG_DEBUG(...) flux::detail::log(LogLevel::Debug, __VA_ARGS__)
#endif

#define FLUX_LOG_INFO(...)  flux::detail::log(LogLevel::Info, __VA_ARGS__)
#define FLUX_LOG_WARN(...)  flux::detail::log(LogLevel::Warn, __VA_ARGS__)
#define FLUX_LOG_ERROR(...) flux::detail::log(LogLevel::Error, __VA_ARGS__)
```

Replace every `std::cout` in the codebase with the appropriate macro.

### 3.5 Core Types Cleanup

The existing `Types.hpp` (Point, Size, Rect, Color, EdgeInsets, CornerRadius, enums) is solid. Minor changes:

- Make `EdgeInsets` and `CornerRadius` constructors `constexpr`.
- Move `BackgroundImage` out of `Types.hpp` into a separate header (it's rendering-specific).
- Move `CursorType` into the platform layer.
- Move `TextMeasurement` into the graphics layer.
- Remove the forward declaration of `TextStyle` at the top of `Types.hpp` -- `Types.hpp` should have zero dependencies on other Flux headers.

### 3.6 Build System

The `CMakeLists.txt` is rewritten to be platform-agnostic:

```cmake
cmake_minimum_required(VERSION 3.25)
project(flux LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Dependencies ---
find_package(SDL3 REQUIRED)
find_package(Freetype REQUIRED)
find_package(OpenGL REQUIRED)

# --- NanoVG (vendored) ---
add_library(nanovg STATIC third_party/nanovg/src/nanovg.c)
target_include_directories(nanovg PUBLIC third_party/nanovg/src)
target_link_libraries(nanovg PRIVATE Freetype::Freetype OpenGL::GL)
if(NOT WIN32 AND NOT APPLE)
    # Linux: use GLES2
    target_compile_definitions(nanovg PRIVATE NANOVG_GLES2)
endif()

# --- Flux library ---
add_library(flux STATIC
    src/Core/Application.cpp
    src/Core/Window.cpp
    # ...
    src/Platform/SDLWindow.cpp    # single platform file, all OSes
    src/Graphics/NanoVGRenderer.cpp
    src/Graphics/NanoVGRenderContext.cpp
    src/Graphics/Renderer.cpp
    src/Graphics/Path.cpp
    src/Views/SVG.cpp
    src/Views/NanoSVG.cpp
)

target_include_directories(flux PUBLIC include)
target_link_libraries(flux
    PUBLIC SDL3::SDL3 Freetype::Freetype OpenGL::GL nanovg
)

# No more: wayland-client, wayland-egl, wayland-cursor, wayland-scanner,
#          EGL, GLES2, xdg-shell protocol generation

# --- Examples ---
option(BUILD_EXAMPLES "Build examples" ON)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# --- Tests ---
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
```

**Platform-specific notes:**
- **macOS:** Clang ships with Xcode. SDL3 is available via Homebrew (`brew install sdl3`) or FetchContent. NanoVG uses GL2 or GL3 (not GLES2).
- **Windows:** MSVC (2022+) or Clang-cl. SDL3 available via vcpkg or FetchContent. NanoVG uses GL2 or GL3. `OpenGL::GL` finds `opengl32.lib`.
- **Linux:** Clang or GCC. SDL3 handles Wayland vs X11 automatically at runtime. NanoVG uses GLES2 (same as current).

---

## 4. Phase 2: Element Tree & State

*Goal: Build the persistent backing store that makes everything else possible. This is the heart of the rewrite.*

### 4.1 View Descriptions (Lightweight)

**Current problem:** `View` is a type-erased wrapper holding a `unique_ptr<ViewInterface>`. Constructing a `View` allocates on the heap. Copying a `View` deep-clones via `clone()`. Every frame, the entire tree is constructed, cloned into `LayoutNode`, and discarded. Each `View` is ~2KB from `FLUX_VIEW_PROPERTIES` alone.

**Design:** The user-facing syntax does not change. `Text{.value = "Hello"}` still works. But internally, the struct produced by a designated initializer is a **description** -- a lightweight value that says *what* to render, not *how* to render it.

The key insight from SwiftUI: **descriptions are cheap and ephemeral; elements are expensive and persistent.** Descriptions are produced by `body()` every time an element is dirty. The element tree diffs old descriptions against new ones and only updates what changed.

To make descriptions cheap:
1. `View` stores a `shared_ptr<const ViewInterface>` instead of `unique_ptr<ViewInterface>`. Copying a `View` increments a refcount instead of deep-cloning. This is the single highest-leverage change for performance.
2. Long-term, explore storing descriptions in a flat arena, but shared_ptr is the right first step because it preserves the existing API.

```cpp
class View {
    std::shared_ptr<const ViewInterface> component_;

public:
    View() = default;

    template<typename T>
    requires ViewComponent<std::remove_cvref_t<T>>
    View(T&& component)
        : component_(std::make_shared<const ViewAdapter<std::remove_cvref_t<T>>>(
              std::forward<T>(component))) {}

    // Copies are now cheap: shared refcount, no deep clone
    View(const View&) = default;
    View& operator=(const View&) = default;
    View(View&&) = default;
    View& operator=(View&&) = default;

    // ... delegate methods unchanged ...
};
```

This single change eliminates the `clone()` overhead that currently dominates per-frame cost.

### 4.2 FLUX_VIEW_PROPERTIES Rework

**Current problem:** The macro injects ~20 `Property<T>` fields and ~17 `std::function` callbacks into every component. A Spacer carries 17 event callbacks it will never use. Total size: ~1.5-2KB per component.

**Design:** Split into tiers:

```cpp
// Tier 1: Every component gets these (visual + layout essentials)
#define FLUX_VIEW_PROPERTIES \
    Property<EdgeInsets> padding = {}; \
    Property<Color> backgroundColor = Colors::transparent; \
    Property<Color> borderColor = Colors::transparent; \
    Property<float> borderWidth = 0; \
    Property<CornerRadius> cornerRadius = CornerRadius{0, 0, 0, 0}; \
    Property<float> opacity = 1.0f; \
    Property<bool> visible = true; \
    Property<bool> clip = false; \
    Property<float> expansionBias = 0.0f; \
    Property<float> compressionBias = 1.0f; \
    Property<std::optional<float>> minWidth = std::nullopt; \
    Property<std::optional<float>> maxWidth = std::nullopt; \
    Property<std::optional<float>> minHeight = std::nullopt; \
    Property<std::optional<float>> maxHeight = std::nullopt; \
    Property<bool> focusable = false; \
    Property<std::string> focusKey = ""

// Tier 2: Interactive components add this
#define FLUX_INTERACTIVE_PROPERTIES \
    std::function<void()> onClick = nullptr; \
    std::function<void(float, float, int)> onMouseDown = nullptr; \
    std::function<void(float, float, int)> onMouseUp = nullptr; \
    std::function<void(float, float)> onMouseMove = nullptr; \
    std::function<void()> onMouseEnter = nullptr; \
    std::function<void()> onMouseLeave = nullptr; \
    std::function<void()> onDoubleClick = nullptr; \
    std::function<void()> onFocus = nullptr; \
    std::function<void()> onBlur = nullptr; \
    std::function<bool(const KeyEvent&)> onKeyDown = nullptr; \
    std::function<bool(const KeyEvent&)> onKeyUp = nullptr; \
    std::function<void(const std::string&)> onTextInput = nullptr; \
    std::function<void(float, float)> onScroll = nullptr

// Tier 3: Transform properties (most components don't need these)
#define FLUX_TRANSFORM_PROPERTIES \
    Property<float> rotation = 0; \
    Property<float> scaleX = 1.0f; \
    Property<float> scaleY = 1.0f; \
    Property<Point> offset = Point{0, 0}
```

Components opt in:

```cpp
struct Spacer {
    FLUX_VIEW_PROPERTIES;
    // ~400 bytes instead of ~2KB
};

struct Button {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;
    Property<std::string> text;
};
```

The `ViewAdapter` SFINAE traits already detect which fields exist, so components without `onClick` simply won't be interactive.

### 4.3 Element Tree

**Current problem:** There is no persistent tree. Every frame, `body()` is called, a `LayoutNode` tree is built from scratch, and it is discarded after rendering. State survives only because it lives in captured closures or external variables.

**Design:** The element tree is the framework's internal representation of the UI. Each element corresponds to a view description and persists across frames.

```cpp
class Element {
public:
    // Identity
    TypeId typeId;                    // typeid of the component (e.g., typeid(Button))
    std::string key;                  // explicit key, or "" for structural identity
    size_t structuralIndex;           // position among siblings of the same type

    // State
    View currentDescription;          // the description from the last body() call
    std::vector<std::unique_ptr<Element>> children;

    // Layout (cached)
    LayoutConstraints constraints;
    Rect computedBounds;
    bool layoutDirty = true;

    // Dirty tracking
    bool bodyDirty = true;

    // Lifecycle
    bool isMounted = false;

    // Back-pointer
    Runtime* runtime = nullptr;

    void markDirty() {
        bodyDirty = true;
        layoutDirty = true;
        if (runtime) runtime->requestRedraw(this);
    }
};
```

### 4.4 Identity and Diffing

SwiftUI uses two kinds of identity:
1. **Structural identity:** position in the tree + type. The first `Text` child of a `VStack` is matched to the first `Text` child in the next frame's `VStack`.
2. **Explicit identity:** `.id(item.id)` overrides structural identity.

Flux v2 uses the same approach:

```cpp
// Structural identity: (parent element, child index, component type)
// Explicit identity via key:
Button { .key = "submit-button", .text = "Submit" }
```

**Diffing algorithm:**

```
function reconcile(element, newChildren):
    oldChildren = element.children
    for each (index, newChild) in newChildren:
        match = findMatch(oldChildren, newChild, index)
        if match exists:
            update match with newChild's description
            if description changed: mark match.bodyDirty = true
        else:
            create new Element from newChild → call onMount()
    for each unmatched old child:
        call onUnmount() → destroy element

function findMatch(oldChildren, newChild, index):
    if newChild has key:
        return oldChildren.find(c => c.key == newChild.key && c.typeId == newChild.typeId)
    return oldChildren.find(c => c.structuralIndex == index && c.typeId == newChild.typeId)
```

### 4.5 State Management (Property<T> Rework)

**Current problem:** `Property<T>` mutations call the global `requestApplicationRedraw()`. The entire UI is redrawn on any change. There is no batching.

**Design:** `Property<T>` in stateful mode holds a back-pointer to its owning `Element`. When mutated, it marks that element dirty. The framework re-evaluates only dirty elements' `body()` calls, diffs the results, and updates the subtree.

```cpp
template<typename T>
class Property {
private:
    struct StatefulValue {
        T value;
        Element* owner = nullptr;   // set by element tree during mount

        void notifyChange() {
            if (owner) {
                owner->markDirty();  // targeted, not global
            }
        }
    };

    std::variant<
        std::shared_ptr<StatefulValue>,
        T,
        std::function<T()>
    > storage;

    // ... rest of the API unchanged ...
};
```

**Batching:** Multiple property mutations between frames all mark elements dirty. On the next frame, the runtime collects all dirty elements, re-evaluates them top-down, and diffs. Multiple mutations naturally coalesce.

**Thread safety:** Remove `shared_mutex` from `StatefulValue`. Cross-thread mutations post to the UI thread's event queue via SDL:

```cpp
void set(T value) {
    if (isOnUIThread()) {
        storage_->value = std::move(value);
        storage_->notifyChange();
    } else {
        // SDL_PushEvent posts to the main thread's event queue
        postToUIThread([this, v = std::move(value)] {
            storage_->value = std::move(v);
            storage_->notifyChange();
        });
    }
}
```

### 4.6 Component Lifecycle

**Current problem:** Components are rebuilt from scratch every frame. No `onMount`, `onUnmount`. No way to run code when a component first appears or is removed.

**Design:** Optional lifecycle methods detected via SFINAE:

```cpp
struct SearchField {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;
    Property<std::string> query = "";

    void onMount() { /* load data, start timers */ }
    void onUnmount() { /* cancel requests, stop timers */ }

    View body() const {
        return HStack { .children = {
            Text { .value = query },
            Button { .text = "Clear", .onClick = [&] { query = ""; } }
        }};
    }
};
```

### 4.7 Environment (Value Propagation)

**Design:** Propagate values down the element tree without prop drilling:

```cpp
struct ThemeKey : EnvironmentKey<Theme> {
    static Theme defaultValue() { return Theme::light(); }
};

// Provide a value
EnvironmentProvider<ThemeKey>{
    .value = Theme::dark(),
    .child = VStack { .children = { ... } }
}

// Read the value (in any descendant)
struct MyComponent {
    FLUX_VIEW_PROPERTIES;
    Environment<ThemeKey> theme;

    View body() const {
        return Text { .value = "Hello", .color = theme->textColor };
    }
};
```

---

## 5. Phase 3: Layout Engine

*Goal: A standalone, testable layout engine decoupled from the view system.*

### 5.1 Separation of Concerns

**Current problem:** Layout is embedded in the view layer. `layoutStack()` interleaves with view construction. It cannot be tested without rendering infrastructure.

**Design:** The layout engine operates on pure data and produces pure data:

```cpp
struct LayoutConstraints {
    float flexGrow = 0;
    float flexShrink = 1;
    std::optional<float> width, height;
    std::optional<float> minWidth, maxWidth, minHeight, maxHeight;
    EdgeInsets padding;
    float gap = 0;

    enum class Display { Flex, Grid, None } display = Display::Flex;
    enum class Direction { Row, Column } direction = Direction::Column;
    JustifyContent justify = JustifyContent::start;
    AlignItems align = AlignItems::stretch;

    int columns = 1;
    int colspan = 1, rowspan = 1;
};

struct LayoutNode {
    LayoutConstraints constraints;
    Size intrinsicSize;
    std::vector<LayoutNode*> children;
    Rect computedBounds;              // OUTPUT
};

class LayoutEngine {
public:
    void compute(LayoutNode& root, Size availableSize);
};
```

### 5.2 Parent-Proposes, Child-Responds

1. **Proposal:** The parent passes available bounds.
2. **Measurement:** Each element reports its intrinsic size.
3. **Layout:** The engine distributes space using `expansionBias`/`compressionBias` (Flux's flexGrow/flexShrink).
4. **Bounds:** Each element receives its computed `Rect`.

The existing `layoutStack()` algorithm is sound. The change is extracting it from the view layer into a standalone function.

### 5.3 Layout Caching

Each element caches its computed bounds. When an element is not dirty and its parent's proposal hasn't changed, the cached bounds are reused. Only dirty subtrees trigger re-layout.

---

## 6. Phase 4: Event System

*Goal: A unified event pipeline that handles mouse, keyboard, and focus through a single path.*

### 6.1 Current Problems

1. **Split dispatch:** Mouse events go through `Renderer::findAndDispatchEvent()`. Keyboard events go through `KeyboardInputHandler` → `FocusState`. These two paths don't compose.
2. **No bubbling:** A parent can't intercept a child's click.
3. **Raw View* in FocusState:** Can dangle between frames.
4. **onClick fires on mouseDown:** Non-standard.
5. **Drag events declared but never dispatched.**
6. **No hover tracking.**

### 6.2 Unified Event Pipeline

```
1. HIT TEST     → Walk the element tree, find deepest element under the pointer.
2. CAPTURE      → Root → target, calling capture handlers. Any can stop propagation.
3. TARGET       → Call the target element's handler.
4. BUBBLE       → Target → root, calling bubble handlers. Any can stop propagation.
```

For keyboard events, the target is the focused element. Same pipeline.

### 6.3 Event Types

```cpp
struct Event {
    enum class Phase { Capture, Target, Bubble };
    Phase phase;
    bool handled = false;
    void stopPropagation() { handled = true; }
};

struct PointerEvent : Event {
    enum class Kind { Move, Down, Up, Scroll, Enter, Leave };
    Kind kind;
    Point windowPosition;
    Point localPosition;
    int button = 0;
    float scrollDeltaX = 0, scrollDeltaY = 0;
    KeyModifier modifiers;
};

struct KeyEvent : Event {
    enum class Kind { Down, Up, TextInput };
    Kind kind;
    Key key;
    KeyModifier modifiers;
    std::string text;
    bool isRepeat = false;
};
```

### 6.4 State Machines

**Hover tracking:** Maintain a stack of elements under the pointer. On move, diff old vs new stack → `Enter`/`Leave` events.

**Click detection:** `mouseDown` records the pressed element. `mouseUp` on the same element fires `onClick`. Movement beyond a threshold cancels (becomes a drag).

**Drag detection:** `mouseDown` + movement beyond threshold → `onDragStart` → `onDrag` → `onDragEnd`/`onDrop`.

**Focus:** Clicking a `focusable` element gives it focus. Tab cycles. Focused element stored as an `Element*` (stable across frames since elements persist).

---

## 7. Phase 5: Rendering

*Goal: Decouple the rendering traversal from GPU calls.*

### 7.1 Render Command Buffer

**Current problem:** `Renderer::renderTree()` makes immediate NanoVG calls. Adding a second backend means duplicating the entire traversal.

**Design:**

```cpp
using RenderCommand = std::variant<
    DrawRect, DrawRoundedRect, DrawCircle, DrawEllipse,
    DrawLine, DrawArc, DrawPath,
    DrawText, DrawImage,
    PushClip, PopClip,
    PushTransform, PopTransform,
    SetOpacity
>;

class RenderCommandBuffer {
    std::vector<RenderCommand> commands_;
public:
    void drawRect(Rect bounds, FillStyle fill, StrokeStyle stroke);
    void drawText(std::string text, Point position, TextStyle style);
    void pushClip(Path path);
    void popClip();
    void pushTransform(float matrix[6]);
    void popTransform();

    const std::vector<RenderCommand>& commands() const;
    void clear();
};
```

**Tree traversal produces commands (backend-agnostic):**

```cpp
void buildRenderCommands(const Element& element, RenderCommandBuffer& buffer) {
    if (!element.isVisible()) return;
    buffer.pushTransform(...);
    if (element.shouldClip()) buffer.pushClip(...);
    element.renderSelf(buffer);
    for (const auto& child : element.children)
        buildRenderCommands(*child, buffer);
    if (element.shouldClip()) buffer.popClip();
    buffer.popTransform();
}
```

**Backend consumes commands:**

```cpp
class NanoVGBackend : public RenderBackend {
    NVGcontext* nvg_;
public:
    void execute(const RenderCommandBuffer& buffer) override {
        for (const auto& cmd : buffer.commands())
            std::visit([this](const auto& c) { draw(c); }, cmd);
    }
};
```

### 7.2 RenderContext Cleanup

**Current problem:** `Renderer.cpp` downcasts `RenderContext*` to `NanoVGRenderContext*` to set `globalFocusedKey_`.

**Fix:** Add `setGlobalFocusKey()` to `RenderContext`. Long-term, `RenderContext` is replaced by `RenderCommandBuffer` as the interface components render to.

---

## 8. Phase 6: Platform Polish

*Goal: Fill in the cross-platform gaps that SDL3 doesn't cover automatically.*

### 8.1 Font Discovery

**Current problem:** `NanoVGRenderContext.cpp` has hardcoded font paths for Linux (`/usr/share/fonts/`), macOS (`/System/Library/Fonts/`), and Windows (`C:\Windows\Fonts\`). The current code only runs on Linux, so the macOS/Windows paths were never tested.

**Design:** A `FontDiscovery` utility per platform:

```cpp
class FontDiscovery {
public:
    // Returns the absolute path to a font matching the request,
    // or empty string if not found.
    static std::string findFont(const std::string& family, FontWeight weight);

    // Returns a list of all available font families on the system.
    static std::vector<std::string> availableFamilies();
};
```

Implementation per platform:
- **macOS:** Use CoreText (`CTFontDescriptorCreateMatchingFontDescriptors`) to resolve font family + weight → file path.
- **Windows:** Use DirectWrite (`IDWriteFontCollection::FindFamilyName`) or the registry (`HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion\Fonts`).
- **Linux:** Use Fontconfig (`FcFontMatch`).

SDL3 does not provide font discovery, so this is the one piece of platform-specific code outside `SDLWindow`. Isolate it behind the `FontDiscovery` interface so the rest of the framework remains platform-agnostic.

### 8.2 Clipboard

**Current problem:** Clipboard is not implemented. The shortcut commands for Copy/Cut/Paste are stubs that only log.

**Fix:** SDL3 provides `SDL_GetClipboardText()` and `SDL_SetClipboardText()`. Wire them into the shortcut commands:

```cpp
class CopyCommand : public ShortcutCommand {
    void execute() override {
        if (auto* focused = runtime->focusedElement()) {
            std::string text = focused->getSelectedText();
            SDL_SetClipboardText(text.c_str());
        }
    }
};
```

### 8.3 Headless Backend

A headless platform + software renderer for testing. No SDL, no GPU, no display server.

```cpp
class HeadlessPlatformFactory : public PlatformFactory {
public:
    std::unique_ptr<PlatformWindow> createWindow(WindowConfig config) override {
        return std::make_unique<HeadlessWindow>(config.size);
    }
    std::unique_ptr<RenderBackend> createRenderBackend() override {
        return std::make_unique<SoftwareBackend>();
    }
};

// Usage in tests:
Runtime runtime;
runtime.setPlatformFactory(std::make_unique<HeadlessPlatformFactory>());
auto& window = runtime.createWindow({.size = {400, 300}});
window.setRootView(MyComponent{});
runtime.renderOneFrame();
auto bitmap = window.capturePixels();
EXPECT_EQ(bitmap, loadBaseline("my_component.png"));
```

### 8.4 Resource Management

**Current problem:** Fonts, images, and SVGs are cached in global `std::map`s with no eviction.

**Design:**

```cpp
class ResourceManager {
public:
    FontHandle loadFont(std::string_view path, FontWeight weight = FontWeight::regular);
    ImageHandle loadImage(std::string_view path);
    SVGHandle loadSVG(std::string_view svgContent);

    void setMaxCacheSize(size_t bytes);
    void collectUnused();
    size_t memoryUsage() const;
};
```

Owned by `Runtime`. No global state.

---

## 9. Phase 7: Components

*Goal: Rebuild the built-in components on top of the new architecture.*

### 9.1 Strengthen ViewComponent Concept

**Current problem:** The concept only checks `!is_same_v<T, View>`. Any type satisfies it.

**Fix:**

```cpp
template<typename T>
concept ViewComponent =
    !std::is_same_v<std::remove_cvref_t<T>, View> &&
    requires(T t) {
        { t.visible } -> std::convertible_to<bool>;
        { t.padding };
    };
```

### 9.2 Fix body() Evaluation

**Current problem:** `ViewAdapter` calls `body()` 15+ times per frame per component.

**Fix with element tree:** The element tree calls `body()` exactly once when dirty. The result is stored in the element. `ViewAdapter` no longer calls `body()`.

**Intermediate fix (before element tree):** Cache on `ViewAdapter`:

```cpp
template<ViewComponent T>
class ViewAdapter : public ViewInterface {
    mutable T component;
    mutable std::optional<View> cachedBody_;
    mutable uint64_t cacheFrame_ = 0;

    const View& getBody(uint64_t currentFrame) const {
        if (cacheFrame_ != currentFrame) {
            if constexpr (has_body<T>::value)
                cachedBody_ = component.body();
            cacheFrame_ = currentFrame;
        }
        return *cachedBody_;
    }
};
```

### 9.3 Built-in Component Rebuild

**Leaf components** (Text, Image, SVG, Badge, Divider, ProgressBar, Spacer):
- `preferredSize()` + `render()`. No `body()` or `layout()`.

**Container components** (VStack, HStack, Grid, ScrollArea):
- Declare children. Provide `LayoutConstraints`. The layout engine positions them.

**Interactive components** (Button, Checkbox, Toggle, RadioButton, Slider):
- `FLUX_INTERACTIVE_PROPERTIES`. `render()` for visual state. `handleKeyDown()` for keyboard. `onClick` fires on mouseUp.

### 9.4 Portability Fix

**Current problem:** `View.hpp` includes `<cxxabi.h>` (GCC/Clang-only).

**Fix:** Move `demangleTypeName()` to a `.cpp` file with platform fallback:

```cpp
#if defined(__GNUC__) || defined(__clang__)
#include <cxxabi.h>
std::string demangleTypeName(const char* name) {
    int status;
    char* demangled = abi::__cxa_demangle(name, nullptr, nullptr, &status);
    std::string result = (status == 0 && demangled) ? demangled : name;
    free(demangled);
    return result;
}
#else
std::string demangleTypeName(const char* name) { return name; }
#endif
```

---

## 10. Phase 8: Quality & Testing

*Goal: Make the framework trustworthy on all platforms.*

### 10.1 Test Infrastructure

Add Catch2 to `CMakeLists.txt`:

```cmake
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(Catch2 GIT_REPOSITORY https://github.com/catchorg/Catch2.git GIT_TAG v3.x)
    FetchContent_MakeAvailable(Catch2)
    enable_testing()
    add_subdirectory(tests)
endif()
```

**Test layers (priority order):**

1. **Core types:** Point, Size, Rect, Color, EdgeInsets arithmetic.
2. **Property<T>:** Stateful mutations, direct values, lambdas, dirty notification.
3. **Layout engine:** Feed constraints, assert on computed bounds.
4. **Event dispatch:** Simulate events, verify handlers, bubbling, focus.
5. **Reconciliation:** Diff descriptions, verify element creation/update/removal.
6. **Components:** Snapshot tests for each component's layout output.
7. **Visual regression:** Headless render to bitmap, compare against baselines.

### 10.2 Header Hygiene

- Forward-declare aggressively.
- Split `View.hpp` into `ViewInterface.hpp`, `ViewAdapter.hpp`, `View.hpp`.
- Add precompiled header support.
- Keep `Flux.hpp` umbrella but document selective includes.

### 10.3 Accessibility

Each element carries an accessibility annotation:

```cpp
struct AccessibilityInfo {
    enum class Role { None, Button, Text, Checkbox, Slider, Group, Image, List, ListItem };
    Role role = Role::None;
    std::string label;
    std::string value;
    std::function<void()> activate;
};
```

The platform layer maps this to:
- **Linux:** AT-SPI2 (via `libatspi`)
- **macOS:** NSAccessibility protocol
- **Windows:** UI Automation (UIA)

### 10.4 CI/CD

Build and test on all three platforms:

```yaml
# .github/workflows/ci.yml
on: [push, pull_request]
jobs:
  build:
    strategy:
      matrix:
        include:
          - os: ubuntu-latest
            deps: sudo apt-get install -y libsdl3-dev libfreetype6-dev clang cmake
          - os: macos-latest
            deps: brew install sdl3 freetype
          - os: windows-latest
            deps: vcpkg install sdl3 freetype
    runs-on: ${{ matrix.os }}
    steps:
      - uses: actions/checkout@v4
        with: { submodules: recursive }
      - run: ${{ matrix.deps }}
      - run: cmake -B build -DBUILD_TESTS=ON
      - run: cmake --build build --config Release -j4
      - run: ctest --test-dir build --output-on-failure
```

---

## 11. Implementation Order

### Sprint 1: SDL3 Migration (1-2 weeks)
1. Add SDL3 dependency to CMakeLists.txt.
2. Implement `SDLWindow` (~300 lines).
3. Wire NanoVG to SDL's GL context.
4. Verify the hello-world example runs on macOS, Windows, and Linux.
5. Delete `WaylandWindow.cpp`, Wayland protocol generation, EGL code.
6. Update CMakeLists.txt to be platform-agnostic.

### Sprint 2: Quick Wins (1 week)
7. Add logging macros, replace all `std::cout`/`std::cerr`.
8. Replace `View`'s deep-copy with `shared_ptr` (4.1).
9. Cache `body()` result in `ViewAdapter` (9.2 intermediate fix).
10. Move `demangleTypeName()` to `.cpp` with MSVC fallback (9.4).
11. Add Catch2, write tests for `Property<T>`, core types, `Path`.

### Sprint 3: Runtime (1 week)
12. Implement `Runtime` to replace `Application` singleton (3.3).
13. Decouple `Property<T>` from global `requestApplicationRedraw()`.
14. Implement `FontDiscovery` per platform (8.1).
15. Wire clipboard via SDL3 (8.2).
16. Update all examples.

### Sprint 4: Element Tree (2-3 weeks)
17. Implement `Element` and the tree structure (4.3).
18. Implement reconciliation/diffing (4.4).
19. Wire `Property<T>` to mark owning elements dirty (4.5).
20. Implement component lifecycle (4.6).
21. Add tests for reconciliation.

### Sprint 5: Layout Engine (1-2 weeks)
22. Extract layout into a standalone engine (5.1).
23. Implement layout caching (5.3).
24. Write exhaustive layout tests.

### Sprint 6: Event System (2 weeks)
25. Implement unified event pipeline with hit testing (6.2).
26. Implement hover, click, drag state machines (6.4).
27. Fix onClick to fire on mouseUp.
28. Rewrite focus management on the element tree.
29. Remove `MouseInputHandler`, `KeyboardInputHandler`, raw `View*` pointers.

### Sprint 7: Rendering (1-2 weeks)
30. Implement render command buffer (7.1).
31. Implement NanoVG backend (consumes command buffer).
32. Clean up `RenderContext` downcasting (7.2).

### Sprint 8: Polish (1-2 weeks)
33. Split `FLUX_VIEW_PROPERTIES` into tiers (4.2).
34. Strengthen `ViewComponent` concept (9.1).
35. Implement headless backend (8.3).
36. Implement `ResourceManager` (8.4).

### Sprint 9: Quality (ongoing)
37. Header hygiene (10.2).
38. CI/CD on all three platforms (10.4).
39. Accessibility annotations (10.3).
40. Environment values (4.7).
41. Visual regression tests using headless backend.

---

## Appendix A: Quick Fixes (Independent of Rewrite)

These can be done today on the current codebase without waiting for any phase:

| Fix | File(s) | Effort |
|-----|---------|--------|
| Replace `std::cout` debug output with conditional logging | All `.cpp` files | 1 hour |
| Fix `onClick` firing on mouseDown → fire on mouseUp | `Renderer.hpp` `dispatchEventToView()` | 30 min |
| Remove `const_cast` in focus registration | `Renderer.cpp`, `FocusState.hpp` | 30 min |
| Fix `Slider` debug `std::cout` in `updateValueFromPosition()` | `Slider.hpp` | 5 min |
| Fix `ImageFit::None` missing `break` in switch | `NanoVGRenderContext.cpp` | 5 min |
| Fix `onWindowClosing` being empty | `Application.cpp` | 15 min |
| Fix `processEvents()` breaking on first closing window | `Application.cpp` | 15 min |
| Remove unused `nanovg-original` submodule reference | `.gitmodules` | 5 min |
| Fix examples README listing nonexistent examples (02, 07, 08) | `examples/README.md` | 10 min |
| Fix `automotive-dashboard` detached thread with no stop condition | `examples/11-automotive-dashboard/main.cpp` | 15 min |
| Remove or implement drag event callbacks | `View.hpp` | 10 min |
| Add `.clang-format` for consistent formatting | Project root | 15 min |
| Add `.clang-tidy` for static analysis | Project root | 15 min |

## Appendix B: What is NOT Changing

- **Designated initializer syntax** for component construction.
- **`Property<T>`** as the reactive primitive (API surface preserved, internals reworked).
- **Plain structs** as components (no inheritance required).
- **`body()` returning `View`** for composition.
- **`render()` for custom drawing.**
- **NanoVG** as the primary render backend.
- **C++23** as the language standard.
- **The existing component set** (Text, Button, VStack, HStack, Grid, etc.).
- **The examples** (updated to use `Runtime` instead of `Application`, otherwise unchanged).

The rewrite is internal. A user reading a Flux v1 example should be able to read a Flux v2 example and see the same patterns with minor syntactic differences.

## Appendix C: Platform Support Matrix

| Feature | Linux | macOS | Windows |
|---------|-------|-------|---------|
| Windowing | SDL3 (Wayland + X11) | SDL3 (Cocoa) | SDL3 (Win32) |
| GL context | SDL3 → GLES2 or GL3 | SDL3 → GL2/GL3 | SDL3 → GL2/GL3 |
| Rendering | NanoVG (GLES2) | NanoVG (GL2) | NanoVG (GL2) |
| Keyboard | SDL3 scancodes | SDL3 scancodes | SDL3 scancodes |
| Mouse / touch | SDL3 | SDL3 | SDL3 |
| Clipboard | SDL3 | SDL3 | SDL3 |
| Cursors | SDL3 | SDL3 | SDL3 |
| DPI scaling | SDL3 | SDL3 | SDL3 |
| Font discovery | Fontconfig | CoreText | DirectWrite / Registry |
| Accessibility | AT-SPI2 | NSAccessibility | UI Automation |
| IME / text input | SDL3 | SDL3 | SDL3 |
| Compiler | Clang 17+ / GCC 13+ | Apple Clang (Xcode 15+) | MSVC 2022+ / Clang-cl |
| Build | CMake 3.25+ | CMake 3.25+ | CMake 3.25+ |
