# Flux v2: Remaining Work & Optimization Plan

## ~~1. State Management: Property<T> Targeted Dirty Marking~~ **Done**

~~**Current problem:** `Property<T>` mutations call the global `requestApplicationRedraw()`.~~

Implemented: `PropertyfulValue` now holds `Element* owner` back-pointer. `notifyChange()` calls `owner->markDirty()` when set, falling back to global `requestApplicationRedraw()`. `setOwner(Element*)` added to `Property<T>`. `ViewInterface::setPropertyOwner(Element*)` wires all `FLUX_VIEW_PROPERTIES` during mount and reconcile. `Element::markDirty()` sets `bodyDirty = true` and requests redraw.

**Remaining:** Runtime-level optimization to only re-evaluate dirty subtrees (currently marks dirty + redraws full tree). Batching and thread safety deferred.

---

## ~~2. Element Tree: Key-Based Identity~~ **Done**

~~The current reconciliation matches children by `typeName` + `structuralIndex` only. There is no support for explicit key-based identity.~~

Implemented: `key` property added to `FLUX_VIEW_PROPERTIES`, propagated through `ViewInterface` → `ViewAdapter` → `View` → `Element`. `reconcileChildren()` now matches by key first (when present), falling back to typeName+structuralIndex for unkeyed elements.

---

## ~~3. Event System: Unified Pipeline~~ **Done**

Implemented unified capture → target → bubble event pipeline for both pointer and keyboard events.

### 3.1 What was done

1. **New event types** (`EventTypes.hpp`): `EventBase` with `Phase` (Capture/Target/Bubble) and `stopPropagation()`. `PointerEvent` extends it with Kind, positions, button, scroll deltas, and modifiers. Replaces the old C-union `Event` struct for dispatch.
2. **Unified pointer dispatch** (`Renderer::dispatchPointerEvent`): Hit-tests the LayoutNode tree to build root→target path, then runs capture → target → bubble phases. Capture calls `capturePointerEvent()` on ancestors (root→target). Target and bubble call existing `handleMouse*()` methods. Mouse capture (drag continuation) preserved.
3. **Keyboard bubbling** (`FocusState`): `dispatchKeyDownToFocused`, `dispatchKeyUpToFocused`, `dispatchTextInputToFocused` now walk the ancestor chain (via `Element::parent` pointer) through capture → target → bubble. Parents can now intercept keyboard events from focused children.
4. **Element parent pointer**: `Element::parent` set during `buildTree` and `reconcileChildren` for efficient ancestor traversal.
5. **Capture-phase API**: `ViewInterface::capturePointerEvent(PointerEvent&)` with SFINAE-based `ViewAdapter` delegation. Components can implement `capturePointerEvent` to intercept events before they reach the target.

### 3.2 Previous fixes

- ~~**Raw View* in FocusState**~~ **Fixed** — FocusState stores Element*.
- ~~**Split dispatch**~~ **Fixed** — both mouse and keyboard now use capture/target/bubble.
- ~~**No bubbling**~~ **Fixed** — parents can intercept child events.

### 3.3 Remaining

- **Drag detection state machine**: `mouseDown` + movement threshold → `onDragStart` → `onDrag` → `onDragEnd`/`onDrop`.
- **Legacy `Event` struct**: Still defined in `Renderer.hpp` for `MouseInputHandler` bridge; can be removed once `MouseInputHandler` creates `PointerEvent` directly.

---

## ~~4. Rendering: Wire Command Buffer Into Main Pipeline~~ **Done**

Implemented: `RenderCommandBuffer` redesigned with 18 stateful command types covering all operations views use during render (Save/Restore, transforms, styles, shapes, text, images, clipping). `NanoVGRenderContext` now dual-writes: each draw call both executes on NanoVG immediately AND records into an attached `RenderCommandBuffer`. `NanoVGBackend::execute()` can replay the full buffer. `Renderer::renderFrame()` attaches the buffer before tree traversal and detaches after. `static_cast<NanoVGRenderContext*>` removed — focus/hover/pressed state moved to base `RenderContext`. Buffer accessible via `Renderer::lastCommandBuffer()` for testing/inspection.

**Remaining:** Switch from dual-write to record-only mode (currently both executes and records for safety). When record-only is enabled, `NanoVGBackend::execute()` replays the buffer as the sole draw path, completing full backend independence.

---

## 5. Platform Polish

### ~~5.1 Font Discovery~~ **Done**

~~Font paths are hardcoded per platform in `NanoVGRenderContext.cpp`.~~

Implemented `FontDiscovery` utility (`FontDiscovery.hpp`/`.cpp`). macOS uses CoreText `CTFontDescriptorCreateWithAttributes` to resolve family name + weight to a file path. Linux/Windows use file-path fallback (fontconfig/DirectWrite integration deferred). `NanoVGRenderContext::getFont()` now delegates to `FontDiscovery` instead of iterating hardcoded paths.

### ~~5.2 Clipboard: Copy/Cut~~ **Done**

~~Paste works via SDL. `CopyCommand` and `CutCommand` are stubs (log only, no-op). Wire `SDL_SetClipboardText()` into them, using the focused text input's selection.~~

Implemented: `getSelectedText()`, `cutSelectedText()`, `selectAll()` added to `ViewInterface` → `ViewAdapter` → `View` chain with SFINAE detection. `CopyCommand`, `CutCommand`, `SelectAllCommand` now use `SDL_SetClipboardText()` via focused view. Cmd+C/V/X/A/Q shortcuts registered on macOS.

### 5.3 Headless Backend

A headless platform + software renderer for testing. No SDL, no GPU, no display server.

```cpp
class HeadlessPlatformFactory : public PlatformFactory {
public:
    std::unique_ptr<PlatformWindow> createWindow(WindowConfig config) override;
    std::unique_ptr<RenderBackend> createRenderBackend() override;
};
```

### ~~5.4 Resource Management~~ **Done**

~~Fonts, images, and SVGs are cached in global `std::map`s with no eviction.~~

Implemented `ResourceManager` singleton with centralized tracking of fonts, images, and SVGs. Tracks per-resource last-used frame for LRU eviction via `collectUnused()` (300-frame threshold). `memoryUsage()` estimates total cache footprint. `NanoVGRenderContext` font and image loading now registers with ResourceManager.

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

### ~~7.3 CI/CD~~ **Done**

~~Add `.github/workflows/ci.yml` for all three platforms, `.clang-format`, and `.clang-tidy`.~~

Added `ci.yml` for macOS/Linux/Windows, `.clang-format` and `.clang-tidy` added in earlier batch.

### ~~7.4 Portability: demangleTypeName MSVC Fallback~~ **Done**

~~`Demangle.cpp` only handles GCC/Clang.~~ MSVC support added via `UnDecorateSymbolName` from DbgHelp.

---

## 8. Quick Fixes (Independent)

| Fix | File(s) | Effort | Status |
|-----|---------|--------|--------|
| ~~Remove `const_cast` throughout Views (SelectInput, TextInput, TextArea, DropdownMenu)~~ | `Views/*.hpp` | 1 hour | **Done** |
| ~~Fix examples README listing nonexistent examples (02, 07, 08, etc.) and outdated Wayland-only requirements~~ | `examples/README.md` | 15 min | **Done** |
| ~~Add `.clang-format` for consistent formatting~~ | Project root | 15 min | **Done** |
| ~~Add `.clang-tidy` for static analysis~~ | Project root | 15 min | **Done** |
| ~~Make `EdgeInsets` constructors `constexpr`~~ | `Types.hpp` | 10 min | **Done** |
| ~~Move `CursorType` out of `Types.hpp` into own header~~ | `Types.hpp`, `CursorType.hpp` | 30 min | **Done** |

---

## 9. Implementation Priority

### Short-term
1. ~~Wire `Property<T>` dirty notification to `Element*` back-pointer.~~ **Done**
2. ~~Wire render command buffer into main pipeline.~~ **Done**
3. ~~Implement clipboard copy/cut.~~ **Done**
4. ~~Fix `const_cast` usage in Views.~~ **Done**
5. ~~Fix examples README.~~ **Done**

### Medium-term
6. ~~Unified event pipeline with capture/bubble.~~ **Done**
7. ~~Migrate `FocusState` from `View*` to `Element*`.~~ **Done**
8. ~~Key-based identity in reconciler.~~ **Done**
9. ~~Font discovery per platform.~~ **Done**
10. ~~Resource manager.~~ **Done**

### Long-term
11. Environment value propagation.
12. Headless backend.
13. Accessibility.
14. ~~CI/CD on all platforms.~~ **Done**
15. Header hygiene.

---

## 10. Custom GPU Renderer

A native GPU rendering pipeline built from scratch, replacing NanoVG as the sole rendering backend.

### ~~10.1 flux::gpu Abstraction Layer + Metal Backend~~ **Done**

Thin C++ abstraction over Device, Buffer, Texture, RenderPipeline, RenderPassEncoder. Metal backend implemented with drawable management, shader compilation via SPIR-V → MSL cross-compilation pipeline.

### ~~10.2 Vulkan Backend~~ **Done**

Full Vulkan backend with VMA for memory management. Handles instance/device creation, swapchain, render passes, command buffers, synchronization. MoltenVK portability extensions on macOS.

### ~~10.3 SDF Shape Shaders + CommandCompiler + Instanced Batching~~ **Done**

SDF fragment shaders for rounded rects, circles, lines. `CommandCompiler` walks `RenderCommandBuffer` and produces batched `SDFQuadInstance` data. `GPURendererBackend` implements `RenderBackend` using `flux::gpu`. Instanced draw calls for all shape types.

### ~~10.4 GlyphAtlas + FreeType Text Rendering~~ **Done**

FreeType-based glyph rasterization packed into a GPU texture atlas. Instanced glyph rendering with per-glyph screen rect, UV rect, and color. Text measurement and layout (single-line and word-wrapped text box).

### ~~10.5 libtess2 Path Rendering, Images, Clipping~~ **Done**

- **Path rendering**: `PathFlattener` converts `Path` objects (including Bezier curves) into polylines via recursive subdivision, then tessellates using `libtess2` for fill and stroke. Non-instanced triangle rendering.
- **Image rendering**: `ImageCache` loads images via `stb_image`, creates GPU textures, caches by path/id. Instanced textured quad rendering with the image pipeline.
- **Clipping**: Scissor-rect based clipping from `CmdClipPath` bounds.

### ~~10.6 Record-Only Mode, Make Custom Renderer Default~~ **Done**

- `GPURenderContext`: record-only `RenderContext` that writes commands into a `RenderCommandBuffer`, never calls NanoVG. On `present()`, feeds the buffer to `GPURendererBackend::execute()`.
- `GPUPlatformRenderer`: `PlatformRenderer` implementation owning `Device`, `GPURendererBackend`, `GPURenderContext`, `ImageCache`.
- `SDLWindow` constructor accepts `RenderBackendType` enum: `NanoVG` (default, OpenGL window), `GPU_Metal` (Metal window), `GPU_Vulkan` (Vulkan window), `GPU_Auto` (platform default).
- `SDLWindowFactory::setRenderBackend()` for programmatic selection.
- Application CLI flag `--backend metal|vulkan|gpu|nanovg` parsed in `Application` constructor.
- Text measurement cache in `GPURenderContext`: `unordered_map<{text, font, size}, Size>` cleared every 300 frames.
- NanoVG remains as compile-time fallback (default when no `--backend` specified).

**Remaining:** Visible line culling, MSDF text for large sizes.

---

## 11. Architectural Review (Deep Review Session)

Comprehensive architectural improvements implemented across the entire codebase:

### ~~11.1 Structural Refactoring~~ **Done**
- **Split View.hpp** (1,376 → 4 files): ViewInterface.hpp, ViewTraits.hpp, ViewAdapter.hpp, View.hpp umbrella
- **Decompose ViewInterface** (~40 virtuals → ~25): VisualStyle + LayoutConstraints struct batching
- **Mixin base structs**: ViewProperties, InteractiveProperties, TransformProperties alongside macros (full replacement blocked by C++ designated initializer limitation)

### ~~11.2 Performance~~ **Done**
- **Flat Path storage**: Replaced per-command `vector<float>` with flat buffer + offsets
- **Flat command buffer**: Replaced 18-type `std::variant` with `uint32_t` byte-stream + object pools (~10x memory reduction)
- **Dirty-subtree rendering**: `subtreeDirty` flag propagation + per-component body cache invalidation via `markBodyDirty()`/`requestRedrawOnly()`
- **LRU caches**: pathTessCache, measureCache, wrapLineCache all converted from full-clear to proper LRU eviction

### ~~11.3 Resource Management~~ **Done**
- **ImageCache LRU**: GPU textures now evicted via LRU with `unique_ptr` destructor cleanup
- **GPURenderContext::deleteImage()**: Implemented (was a no-op)

### ~~11.4 Code Quality~~ **Done**
- **ODR fixes**: `drawCheckbox`/`drawToggle` marked inline
- **Thread safety**: Removed `std::thread`-based animations from ProgressBar/TypingIndicator
- **Code deduplication**: `drawInputFieldChrome()`, `LabeledControl::build()`/`measure()`
- **Design tokens**: Expanded Theme with semantic tokens, `Colors::inherit` sentinel, views resolve from theme at render time
- **NanoVG removal**: Deleted 6 files (~1,674 lines), eliminated OpenGL dependency

### ~~11.5 Overlay/Portal Layer~~ **Done**
- **OverlayManager**: show/hide/layout/render/hit-test for overlays rendered above main tree
- **Renderer integration**: Overlay rendering after main tree, event dispatch with mouse capture tracking
- **View migration**: Dialog, SelectInput, DropdownMenu use overlays; ViewAdapter calls render() before body() when both exist
- **Test infrastructure**: serializeUITree includes overlay layout trees

### 11.6 Remaining (Postponed)
- **Vulkan backend completion**: Postponed (not a priority)
- **Property<T> inline-default**: Implemented with `Property::shared()` opt-in for shared state
