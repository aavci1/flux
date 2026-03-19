# Flux v2: Remaining Work & Optimization Plan

## 1. State Management: Property<T> Targeted Dirty Marking

**Current problem:** `Property<T>` mutations call the global `requestApplicationRedraw()`. The entire UI is redrawn on any change. There is no per-element dirty tracking from Property changes.

**Recent improvement:** `Property<T>::operator=(T&&)` now checks for equality before calling `notifyChange()`, and `suppressRedrawRequests()` prevents spurious redraws during layout/body evaluation. This dropped idle CPU from ~50% to 0%.

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

## 2. Element Tree: Key-Based Identity

The current reconciliation matches children by `typeName` + `structuralIndex` only. There is no support for explicit key-based identity.

**Design:** Add a `key` field to `Element`. When a view specifies `.key = "some-key"`, the reconciler uses it for matching instead of structural index:

```cpp
Button { .key = "submit-button", .text = "Submit" }
```

Update `reconcileChildren()` to prefer key-based matching when keys are present, falling back to structural identity.

---

## 3. Event System: Unified Pipeline

### 3.1 Current Problems

1. **Split dispatch:** Mouse events go through `Renderer::findAndDispatchEvent()`. Keyboard events go through `KeyboardInputHandler` → `FocusState`. These two paths don't compose.
2. **No bubbling:** A parent can't intercept a child's click.
3. **Raw View* in FocusState:** Can dangle between frames.
4. **Drag events not implemented.**

### 3.2 Unified Event Pipeline

```
1. HIT TEST     → Walk the element tree, find deepest element under the pointer.
2. CAPTURE      → Root → target, calling capture handlers. Any can stop propagation.
3. TARGET       → Call the target element's handler.
4. BUBBLE       → Target → root, calling bubble handlers. Any can stop propagation.
```

For keyboard events, the target is the focused element. Same pipeline.

### 3.3 Event Types

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

### 3.4 State Machines

**Drag detection:** `mouseDown` + movement beyond threshold → `onDragStart` → `onDrag` → `onDragEnd`/`onDrop`.

**Focus:** Migrate `FocusState` from `View*` to `Element*` (stable across frames since elements persist).

---

## 4. Rendering: Wire Command Buffer Into Main Pipeline

`RenderCommandBuffer` and `NanoVGBackend` exist but are not used in the main render path. The renderer still makes immediate NanoVG calls via `RenderContext`.

**Fix:** Replace the immediate-mode `RenderContext` path with the command buffer:

1. Tree traversal produces `RenderCommandBuffer` commands.
2. `NanoVGBackend::execute()` consumes them.
3. Remove the `RenderContext` downcast to `NanoVGRenderContext*` in `Renderer::renderFrame()`.

---

## 5. Platform Polish

### 5.1 Font Discovery

Font paths are hardcoded per platform in `NanoVGRenderContext.cpp`. Implement a `FontDiscovery` utility:

- **macOS:** CoreText (`CTFontDescriptorCreateMatchingFontDescriptors`)
- **Windows:** DirectWrite (`IDWriteFontCollection::FindFamilyName`)
- **Linux:** Fontconfig (`FcFontMatch`)

### 5.2 Clipboard: Copy/Cut

Paste works via SDL. `CopyCommand` and `CutCommand` are stubs (log only, no-op). Wire `SDL_SetClipboardText()` into them, using the focused text input's selection.

### 5.3 Headless Backend

A headless platform + software renderer for testing. No SDL, no GPU, no display server.

```cpp
class HeadlessPlatformFactory : public PlatformFactory {
public:
    std::unique_ptr<PlatformWindow> createWindow(WindowConfig config) override;
    std::unique_ptr<RenderBackend> createRenderBackend() override;
};
```

### 5.4 Resource Management

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

## 6. Environment (Value Propagation)

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

## 7. Quality & Testing

### 7.1 Header Hygiene

- Forward-declare aggressively.
- Split `View.hpp` into `ViewInterface.hpp`, `ViewAdapter.hpp`, `View.hpp`.
- Add precompiled header support.

### 7.2 Accessibility

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

### 7.3 CI/CD

Add `.github/workflows/ci.yml` for all three platforms, `.clang-format`, and `.clang-tidy`.

### 7.4 Portability: demangleTypeName MSVC Fallback

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

## 8. Quick Fixes (Independent)

| Fix | File(s) | Effort | Status |
|-----|---------|--------|--------|
| Remove `const_cast` throughout Views (SelectInput, TextInput, TextArea, DropdownMenu) | `Views/*.hpp` | 1 hour | |
| ~~Fix examples README listing nonexistent examples (02, 07, 08, etc.) and outdated Wayland-only requirements~~ | `examples/README.md` | 15 min | **Done** |
| ~~Add `.clang-format` for consistent formatting~~ | Project root | 15 min | **Done** |
| ~~Add `.clang-tidy` for static analysis~~ | Project root | 15 min | **Done** |
| ~~Make `EdgeInsets` constructors `constexpr`~~ | `Types.hpp` | 10 min | **Done** |
| Move `CursorType` out of `Types.hpp` into platform layer | `Types.hpp`, `PlatformWindow.hpp` | 30 min | |

---

## 9. Implementation Priority

### Short-term
1. Wire `Property<T>` dirty notification to `Element*` back-pointer.
2. Wire render command buffer into main pipeline.
3. Implement clipboard copy/cut.
4. Fix `const_cast` usage in Views.
5. Fix examples README.

### Medium-term
6. Unified event pipeline with capture/bubble.
7. Migrate `FocusState` from `View*` to `Element*`.
8. Key-based identity in reconciler.
9. Font discovery per platform.
10. Resource manager.

### Long-term
11. Environment value propagation.
12. Headless backend.
13. Accessibility.
14. CI/CD on all platforms.
15. Header hygiene.
