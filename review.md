# Flux v2: Remaining Work & Optimization Plan

## 1. CPU & Memory Optimization (Critical)

The application currently burns ~50% CPU when idle. The root causes are:

### 1.1 Busy-Loop Main Loop

The main loop in `Runtime::run()` sleeps only 1ms between iterations, polling at ~1000 Hz even when nothing has changed:

```cpp
while (running_) {
    processEvents();
    // ...
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}
```

**Fix:** Replace `SDL_PollEvent` + `sleep(1ms)` with `SDL_WaitEvent` when the application is idle. Only switch to `SDL_PollEvent` when a redraw is pending or animations are active:

```cpp
int Runtime::run() {
    while (running_) {
        SDL_Event event;
        if (needsRedraw_.load(std::memory_order_relaxed) || animating_) {
            while (SDL_PollEvent(&event))
                routeEvent(event);
        } else {
            // Block until an event arrives — zero CPU when idle
            if (SDL_WaitEvent(&event))
                routeEvent(event);
            // Drain any additional queued events
            while (SDL_PollEvent(&event))
                routeEvent(event);
        }
        // ...
    }
}
```

### 1.2 Unconditional Redraws on Every Mouse Event

`Renderer::handleEvent()` calls `requestApplicationRedraw()` after **every** mouse event, including mouse moves that don't change anything:

```cpp
// End of Renderer::handleEvent():
requestApplicationRedraw();  // called unconditionally
```

**Fix:** Only request a redraw when an event handler actually changed state. Return a `bool` from `handleEvent()` and only redraw when `true`.

### 1.3 Cursor Blink Forces Continuous Rendering

When any text input is focused, `Renderer::renderFrame()` unconditionally requests a redraw on **every frame**:

```cpp
if (window_ && !window_->focus().getFocusedKey().empty()) {
    requestApplicationRedraw();
}
```

This means the app renders at full speed (~1000 fps with 1ms sleep) whenever a text field has focus.

**Fix:** Use a timer-based approach for cursor blink. Post a custom SDL event every 500ms to toggle cursor visibility, instead of forcing continuous redraws. Only redraw on the toggle event:

```cpp
// In Runtime or Window:
SDL_AddTimer(500, [](void* param, SDL_TimerID, Uint32) -> Uint32 {
    auto* self = static_cast<Runtime*>(param);
    self->toggleCursorBlink();
    self->requestRedraw();
    return 500;  // repeat
}, this);
```

### 1.4 Full Layout Tree Rebuilt Every Frame

Every render frame calls `rootView_->layout(*renderContext_, bounds)` which rebuilds the entire layout tree from scratch, including calling `body()` on every component.

**Fix:** Only rebuild the layout tree when dirty. The `layoutCacheValid_` flag exists but is always invalidated by `requestRedraw()` → `invalidateLayoutCache()`. Separate layout invalidation from redraw requests. Only invalidate layout when state actually changes.

### 1.5 body() Called Multiple Times Per Frame Per Component

`ViewAdapter` does not cache the result of `body()`. Layout, rendering, and event handling each call `body()` independently.

**Fix:** Cache `body()` results in `ViewAdapter` keyed by a frame generation counter:

```cpp
template<ViewComponent T>
class ViewAdapter : public ViewInterface {
    mutable std::optional<View> cachedBody_;
    mutable uint64_t cacheFrame_ = 0;

    const View& getBody() const {
        uint64_t gen = currentBodyGeneration();
        if (cacheFrame_ != gen) {
            if constexpr (has_body<T>::value)
                cachedBody_ = component.body();
            cacheFrame_ = gen;
        }
        return *cachedBody_;
    }
};
```

### 1.6 Summary of Expected Impact

| Fix | CPU impact |
|-----|-----------|
| SDL_WaitEvent when idle | ~50% → ~0% when idle |
| Conditional redraw on events | Eliminates redundant frames on hover |
| Timer-based cursor blink | 2 redraws/sec instead of 1000/sec when text focused |
| Conditional layout rebuild | Avoids full tree rebuild when only rendering |
| body() caching | 1 call per component per frame instead of 15+ |

---

## 2. State Management: Property<T> Targeted Dirty Marking

**Current problem:** `Property<T>` mutations call the global `requestApplicationRedraw()`. The entire UI is redrawn on any change. There is no per-element dirty tracking from Property changes.

**Design:** `Property<T>` in stateful mode should hold a back-pointer to its owning `Element`. When mutated, it marks that element dirty. The framework re-evaluates only dirty elements' `body()` calls, diffs the results, and updates the subtree.

```cpp
struct StatefulValue {
    T value;
    Element* owner = nullptr;   // set by element tree during mount

    void notifyChange() {
        if (owner) {
            owner->markDirty();  // targeted, not global
        }
    }
};
```

**Batching:** Multiple property mutations between frames all mark elements dirty. On the next frame, the runtime collects all dirty elements, re-evaluates them top-down, and diffs.

**Thread safety:** Cross-thread mutations should post to the UI thread's event queue via `SDL_PushEvent`.

---

## 3. Element Tree: Key-Based Identity

The current reconciliation matches children by `typeName` + `structuralIndex` only. There is no support for explicit key-based identity.

**Design:** Add a `key` field to `Element`. When a view specifies `.key = "some-key"`, the reconciler uses it for matching instead of structural index:

```cpp
Button { .key = "submit-button", .text = "Submit" }
```

Update `reconcileChildren()` to prefer key-based matching when keys are present, falling back to structural identity.

---

## 4. Event System: Unified Pipeline

### 4.1 Current Problems

1. **Split dispatch:** Mouse events go through `Renderer::findAndDispatchEvent()`. Keyboard events go through `KeyboardInputHandler` → `FocusState`. These two paths don't compose.
2. **No bubbling:** A parent can't intercept a child's click.
3. **Raw View* in FocusState:** Can dangle between frames.
4. **Drag events not implemented.**

### 4.2 Unified Event Pipeline

```
1. HIT TEST     → Walk the element tree, find deepest element under the pointer.
2. CAPTURE      → Root → target, calling capture handlers. Any can stop propagation.
3. TARGET       → Call the target element's handler.
4. BUBBLE       → Target → root, calling bubble handlers. Any can stop propagation.
```

For keyboard events, the target is the focused element. Same pipeline.

### 4.3 Event Types

Replace the current C-union `Event` struct in `Renderer.hpp` with proper types:

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
```

### 4.4 State Machines

**Drag detection:** `mouseDown` + movement beyond threshold → `onDragStart` → `onDrag` → `onDragEnd`/`onDrop`.

**Focus:** Migrate `FocusState` from `View*` to `Element*` (stable across frames since elements persist).

---

## 5. Rendering: Wire Command Buffer Into Main Pipeline

`RenderCommandBuffer` and `NanoVGBackend` exist but are not used in the main render path. The renderer still makes immediate NanoVG calls via `RenderContext`.

**Fix:** Replace the immediate-mode `RenderContext` path with the command buffer:

1. Tree traversal produces `RenderCommandBuffer` commands.
2. `NanoVGBackend::execute()` consumes them.
3. Remove the `RenderContext` downcast to `NanoVGRenderContext*` in `Renderer::renderFrame()`.

---

## 6. Platform Polish

### 6.1 Font Discovery

Font paths are hardcoded per platform in `NanoVGRenderContext.cpp`. Implement a `FontDiscovery` utility:

- **macOS:** CoreText (`CTFontDescriptorCreateMatchingFontDescriptors`)
- **Windows:** DirectWrite (`IDWriteFontCollection::FindFamilyName`)
- **Linux:** Fontconfig (`FcFontMatch`)

### 6.2 Clipboard: Copy/Cut

Paste works via SDL, but `CopyCommand` and `CutCommand` are stubs. Wire `SDL_SetClipboardText()` into them.

### 6.3 Headless Backend

A headless platform + software renderer for testing. No SDL, no GPU, no display server.

```cpp
class HeadlessPlatformFactory : public PlatformFactory {
public:
    std::unique_ptr<PlatformWindow> createWindow(WindowConfig config) override;
    std::unique_ptr<RenderBackend> createRenderBackend() override;
};
```

### 6.4 Resource Management

Fonts, images, and SVGs are cached in global `std::map`s with no eviction. Implement a `ResourceManager` owned by `Runtime`:

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

---

## 7. Environment (Value Propagation)

Propagate values down the element tree without prop drilling:

```cpp
struct ThemeKey : EnvironmentKey<Theme> {
    static Theme defaultValue() { return Theme::light(); }
};

EnvironmentProvider<ThemeKey>{
    .value = Theme::dark(),
    .child = VStack { .children = { ... } }
}
```

---

## 8. Quality & Testing

### 8.1 Header Hygiene

- Forward-declare aggressively.
- Split `View.hpp` into `ViewInterface.hpp`, `ViewAdapter.hpp`, `View.hpp`.
- Add precompiled header support.

### 8.2 Accessibility

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

Platform mapping:
- **Linux:** AT-SPI2
- **macOS:** NSAccessibility
- **Windows:** UI Automation

### 8.3 CI/CD

Add `.github/workflows/ci.yml` for all three platforms, `.clang-format`, and `.clang-tidy`.

### 8.4 Portability: demangleTypeName MSVC Fallback

`Demangle.cpp` only handles GCC/Clang. Add MSVC support:

```cpp
#elif defined(_MSC_VER)
#include <DbgHelp.h>
std::string demangleTypeName(const char* name) {
    char buf[1024];
    if (UnDecorateSymbolName(name, buf, sizeof(buf), UNDNAME_COMPLETE))
        return buf;
    return name;
}
#endif
```

---

## 9. Quick Fixes (Independent)

| Fix | File(s) | Effort |
|-----|---------|--------|
| Remove `const_cast` throughout Views (SelectInput, TextInput, TextArea, DropdownMenu) | `Views/*.hpp` | 1 hour |
| Fix examples README listing nonexistent examples (02, 07, 08, etc.) and outdated Wayland-only requirements | `examples/README.md` | 15 min |
| Add `.clang-format` for consistent formatting | Project root | 15 min |
| Add `.clang-tidy` for static analysis | Project root | 15 min |
| Make `EdgeInsets` constructors `constexpr` | `Types.hpp` | 10 min |
| Move `CursorType` out of `Types.hpp` into platform layer | `Types.hpp`, `PlatformWindow.hpp` | 30 min |

---

## 10. Implementation Priority

### Immediate (CPU fix)
1. Replace `sleep(1ms)` loop with `SDL_WaitEvent` when idle.
2. Remove unconditional `requestApplicationRedraw()` from `Renderer::handleEvent()`.
3. Replace continuous cursor-blink redraw with timer-based approach.
4. Cache `body()` results in `ViewAdapter`.

### Short-term
5. Wire `Property<T>` dirty notification to `Element*` back-pointer.
6. Wire render command buffer into main pipeline.
7. Implement clipboard copy/cut.
8. Fix `const_cast` usage in Views.
9. Fix examples README.

### Medium-term
10. Unified event pipeline with capture/bubble.
11. Migrate `FocusState` from `View*` to `Element*`.
12. Key-based identity in reconciler.
13. Font discovery per platform.
14. Resource manager.

### Long-term
15. Environment value propagation.
16. Headless backend.
17. Accessibility.
18. CI/CD on all platforms.
19. Header hygiene.
