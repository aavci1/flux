# Flux Architecture

## Overview

Flux is a minimal immediate mode UI framework built on C++23 with automatic reactivity, featuring comprehensive event handling and focus management.

## Directory Structure

```
flux/
├── include/
│   ├── Flux.hpp                    Main header
│   └── Flux/
│       ├── Core/                   Core system (15 headers)
│       │   ├── Types.hpp          Geometry, colors, enums
│       │   ├── Property.hpp       Reactive properties & state
│       │   ├── View.hpp           Base view class & event system
│       │   ├── ViewHelpers.hpp    View rendering helpers
│       │   ├── Application.hpp    App lifecycle
│       │   ├── Window.hpp         Window & focus management
│       │   ├── LayoutTree.hpp     Layout management
│       │   ├── KeyEvent.hpp       Keyboard event types
│       │   ├── Utilities.hpp      Common utilities
│       │   ├── FocusState.hpp     Focus state management
│       │   ├── KeyboardInputHandler.hpp  Keyboard input handling
│       │   ├── MouseInputHandler.hpp     Mouse input handling
│       │   ├── ShortcutManager.hpp       Global shortcuts
│       │   ├── PlatformWindowFactory.hpp Platform abstraction
│       │   └── WindowEventObserver.hpp   Window event observation
│       ├── Views/                  Components + Layouts (16 headers)
│       │   ├── Text.hpp           Text display
│       │   ├── Button.hpp         Interactive button
│       │   ├── VStack.hpp         Vertical stack layout
│       │   ├── HStack.hpp         Horizontal stack layout
│       │   ├── Spacer.hpp         Flexible space
│       │   ├── Grid.hpp           Grid layout
│       │   ├── StackLayout.hpp    Stack layout utilities
│       │   ├── Image.hpp          Image display
│       │   ├── SVG.hpp            SVG rendering
│       │   ├── Slider.hpp         Value slider
│       │   ├── Checkbox.hpp       Checkbox input
│       │   ├── RadioButton.hpp    Radio button input
│       │   ├── Toggle.hpp         Toggle switch
│       │   ├── Badge.hpp          Badge indicator
│       │   ├── Divider.hpp        Visual divider
│       │   └── ProgressBar.hpp    Progress indicator
│       ├── Graphics/               Rendering (4 headers)
│       │   ├── RenderContext.hpp
│       │   ├── Renderer.hpp
│       │   ├── NanoVGRenderer.hpp
│       │   └── Path.hpp
│       └── Platform/               Platform support (4 headers)
│           ├── PlatformWindow.hpp
│           ├── WaylandWindow.hpp
│           ├── WaylandProtocols.hpp
│           └── WaylandCursor.hpp
├── src/
│   ├── Core/
│   │   ├── Application.cpp        Event loop, window management
│   │   ├── Window.cpp             Window & focus management
│   │   ├── KeyboardInputHandler.cpp  Keyboard input processing
│   │   ├── MouseInputHandler.cpp     Mouse input processing
│   │   ├── FocusState.cpp            Focus state management
│   │   ├── ShortcutManager.cpp       Shortcut handling
│   │   └── PlatformWindowFactory.cpp Platform factory
│   ├── Graphics/
│   │   ├── NanoVGRenderContext.cpp  NanoVG rendering implementation
│   │   ├── NanoVGRenderer.cpp       Renderer implementation
│   │   ├── Renderer.cpp             Base renderer
│   │   └── Path.cpp                 Path utilities
│   ├── Platform/
│   │   └── WaylandWindow.cpp       Wayland window implementation
│   └── Views/
│       ├── NanoSVG.cpp             SVG support
│       └── SVG.cpp                 SVG view
└── examples/                       32+ examples
    ├── 01-hello-world/
    ├── 03-counter/
    ├── 04-todo-app/
    ├── 11-automotive-dashboard/
    ├── 24-calculator/
    ├── 27-focus-keyboard-demo/
    ├── 28-event-system-demo/
    ├── 29-color-picker/
    ├── 33-scroll-area-demo/
    ├── 31-component-showcase/
    └── 32-label-position-demo/
```

## Core Types

### View Hierarchy

```cpp
// Concept - compile-time contract (no inheritance)
template<typename T>
concept ViewComponent = (!std::is_same_v<std::remove_cvref_t<T>, View>);

// ViewInterface - runtime polymorphism
class ViewInterface {
public:
    virtual ~ViewInterface() = default;
    
    // Core methods
    virtual LayoutNode layout(RenderContext& ctx, const Rect& bounds) const = 0;
    virtual View body() const = 0;
    virtual void render(RenderContext& ctx, const Rect& bounds) const = 0;
    virtual Size preferredSize(TextMeasurement& textMeasurer) const = 0;
    virtual std::unique_ptr<ViewInterface> clone() const = 0;
    
    // Property access
    virtual bool isVisible() const = 0;
    virtual bool shouldClip() const = 0;
    virtual float getExpansionBias() const = 0;
    virtual float getCompressionBias() const = 0;
    
    // Event handling
    virtual bool handleMouseDown(float x, float y, int button) { return false; }
    virtual bool handleMouseUp(float x, float y, int button) { return false; }
    virtual bool handleMouseMove(float x, float y) { return false; }
    virtual bool isInteractive() const { return false; }
    
    // Keyboard events
    virtual bool handleKeyDown(const KeyEvent& event) { return false; }
    virtual bool handleKeyUp(const KeyEvent& event) { return false; }
    virtual bool handleTextInput(const TextInputEvent& event) { return false; }
    
    // Focus management
    virtual bool canBeFocused() const { return false; }
    virtual std::string getFocusKey() const { return ""; }
    virtual void notifyFocusGained() {}
    virtual void notifyFocusLost() {}
    
    // Cursor management
    virtual CursorType getCursor() const = 0;
};

// ViewAdapter - bridges concepts to interface
template<ViewComponent T>
class ViewAdapter : public ViewInterface {
    mutable T component;  // mutable for layout modifications
    // Forwards all calls to component with smart defaults
};

// View - type-erased container (public API)
class View {
    std::unique_ptr<ViewInterface> component_;
    // Supports any ViewComponent via templated constructor
};
```

### Property System

`Property<T>` is a unified system that handles both stateful reactive values and component configuration:

```cpp
template<typename T>
class Property {
    struct StatefulValue {
        T value;
        std::shared_mutex mutex;
        void notifyChange() { requestApplicationRedraw(); }
    };
    
    std::variant<
        std::shared_ptr<StatefulValue>,  // Stateful mode (default)
        T,                                // Direct value mode
        std::function<T()>                // Lambda mode
    > storage;

    T get() const;  // Evaluates current value
    operator T() const { return get(); }
};
```

**Three modes:**
1. **Stateful** (default): Thread-safe reactive values that trigger redraws
2. **Direct value**: Static values for configuration
3. **Lambda**: Computed values evaluated each frame

### Common View Properties (FLUX_VIEW_PROPERTIES)

All views have these properties via the `FLUX_VIEW_PROPERTIES` macro:

```cpp
#define FLUX_VIEW_PROPERTIES \
    Property<EdgeInsets> margin = {}; \
    Property<EdgeInsets> padding = {}; \
    Property<Color> backgroundColor = Colors::transparent; \
    Property<BackgroundImage> backgroundImage = BackgroundImage{}; \
    Property<Color> borderColor = Colors::transparent; \
    Property<float> borderWidth = 0; \
    Property<float> cornerRadius = 0; \
    Property<float> opacity = 1.0; \
    Property<bool> visible = true; \
    Property<bool> clip = false; \
    Property<float> rotation = 0; \
    Property<float> scaleX = 1.0; \
    Property<float> scaleY = 1.0; \
    Property<Point> offset = Point{0, 0}; \
    Property<float> expansionBias = 0.0f; \
    Property<float> compressionBias = 1.0f; \
    Property<int> colspan = 1; \
    Property<int> rowspan = 1; \
    Property<CursorType> cursor = CursorType::Default; \
    Property<bool> focusable = false; \
    Property<std::string> focusKey = ""; \
    /* 17 event callbacks - see Event System section */
```

## Event System

All views have access to comprehensive event handling through the `FLUX_VIEW_PROPERTIES` macro:

### Mouse Events (7)
- `std::function<void()> onClick`
- `std::function<void(float x, float y, int button)> onMouseDown`
- `std::function<void(float x, float y, int button)> onMouseUp`
- `std::function<void(float x, float y)> onMouseMove`
- `std::function<void()> onMouseEnter`
- `std::function<void()> onMouseLeave`
- `std::function<void()> onDoubleClick`

### Keyboard Events (3)
- `std::function<bool(const KeyEvent&)> onKeyDown`
- `std::function<bool(const KeyEvent&)> onKeyUp`
- `std::function<void(const std::string&)> onTextInput`

### Focus Events (2)
- `std::function<void()> onFocus`
- `std::function<void()> onBlur`

### Value Change (1)
- `std::function<void()> onChange`

### Drag & Drop (4, for future use)
- `std::function<void(float x, float y)> onDragStart`
- `std::function<void(float x, float y)> onDrag`
- `std::function<void(float x, float y)> onDragEnd`
- `std::function<void(float x, float y)> onDrop`

**Total: 17 event callbacks available on every view**

See `docs/EVENT_SYSTEM.md` for complete event documentation.

## Focus Management

The `Window` class handles keyboard focus and navigation:

### Features
- **Tab Navigation**: Cycle through focusable views with Tab/Shift+Tab
- **Focus Keys**: Unique identifiers for persistent focus state
- **Auto-registration**: Focusable views automatically registered during layout
- **Event Dispatch**: Routes keyboard events to focused view

### Focus Flow
```
Layout Phase:
  → Window clears previous focus registrations
  → Each focusable view registers itself with Window
  → Focus restored to previous focused view (if still present)

Keyboard Event:
  → Window receives key event
  → Window dispatches to focused view
  → View's onKeyDown/onKeyUp/onTextInput called
  → Event propagates if not handled
```

## Rendering Pipeline

### Frame Cycle

```
1. Property Change (Stateful Mode)
   counter++ → notifyChange() → requestRedraw()

2. Application Event Loop
   - Process platform events
   - If needsRedraw:
       for each window:
           window.render()

3. Window Render
   - Dispatch pending keyboard events with layout tree
   - Build layout tree (registers focusable views)
   - Render tree with immediate mode renderer
   - Present frame

4. Layout Phase
   - Evaluate lambda properties (fresh state!)
   - Calculate bounds for all views
   - Register focusable views with Window
   - Cache layout information

5. Render Phase
   - Apply visual properties (transforms, opacity)
   - Draw backgrounds, borders, shadows
   - Render view content
   - Render children recursively
```

### Immediate Mode Flow

```
Property Change
    ↓
requestRedraw()
    ↓
Next Frame
    ↓
Evaluate Lambda Properties (fresh state!)
    ↓
Build Layout Tree
    ↓
Register Focus (Window)
    ↓
Render Tree (RenderContext)
    ↓
Present Frame
    ↓
(Next frame repeats)
```

## Component Implementation

### Leaf Component (Text)

```cpp
struct Text {  // Plain struct, no inheritance
    FLUX_VIEW_PROPERTIES;  // Common properties + events

    Property<std::string> value;
    Property<float> fontSize = 16;
    Property<Color> color = Colors::black;

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);
        ctx.drawText(static_cast<std::string>(value), bounds.center());
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        return textMeasurer.measureText(value, TextStyle::regular("default", fontSize));
    }
};
```

### Container Component (VStack)

```cpp
struct VStack {
    FLUX_VIEW_PROPERTIES;

    Property<std::vector<View>> children = {};
    Property<float> spacing = 0;
    Property<AlignItems> alignItems = AlignItems::stretch;

    // Layout calculates child positions
    LayoutNode layout(RenderContext& ctx, const Rect& bounds) const {
        // Calculate positions for all children
        // Return LayoutNode with positioned children
    }

    // No custom rendering needed - children handled by renderer
};
```

### Interactive Component (Button)

```cpp
struct Button {
    FLUX_VIEW_PROPERTIES;  // Includes onClick from macro

    Property<std::string> text;
    
    void init() {
        focusable = true;  // Enable keyboard focus
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        bool hasFocus = ctx.isCurrentViewFocused();
        // Render with focus indicator if focused
        ViewHelpers::renderView(*this, ctx, bounds);
        ctx.drawText(text, bounds.center());
    }

    // Custom keyboard handler (called before onKeyDown callback)
    bool handleKeyDown(const KeyEvent& event) const {
        if ((event.key == Key::Enter || event.key == Key::Space) && onClick) {
            onClick();
            return true;
        }
        return false;
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        return textMeasurer.measureText(text, ...) + padding;
    }
};
```

## Threading Model

- **Property<T>** (stateful mode): Thread-safe with read-write locks
- **Rendering**: Single-threaded (main thread)
- **Property modifications**: Can happen from any thread
- **Redraws**: Batched per frame (multiple property changes = one redraw)
- **Event handling**: All events processed on main thread

## Memory Model

- **View tree**: Created fresh each frame via builder lambda
- **Layout tree**: Built during layout phase, used for rendering
- **Properties** (stateful): Live across frames with shared ownership
- **View**: Type-erased container using unique_ptr<ViewInterface>
- **ViewAdapter**: Template instantiation per component type
- **FocusManager**: Owned by Window, lives across frames

## Wayland Backend (Linux-only)

### Platform Support
- **WaylandWindow**: Native Wayland window management
- **NanoVG**: Hardware-accelerated 2D rendering with OpenGL ES 2
- **Protocols**: xdg-shell, xdg-decoration, wayland-cursor
- **Input**: Mouse, keyboard, text input
- **Cursor**: Dynamic cursor changes based on interactive elements

### Window Features
- Resize with automatic layout recalculation
- Fullscreen mode
- Window decorations (server-side)
- High-DPI support
- VSync for smooth rendering

## Key Architecture Decisions

### Why Immediate Mode?
- Simpler than retained mode (no separate logical/visual trees)
- No synchronization issues
- Predictable memory usage
- Easier to reason about
- UI always reflects current state

### Why Lambda Properties?
- Evaluated fresh each frame
- Captures latest state automatically
- No manual binding needed
- Type-safe with Property system
- Works seamlessly with reactivity

### Why View (Type-Erased Container)?
- Can't store different component types by value in vectors
- View provides value semantics with type erasure
- Automatic via templated constructor
- Supports designated initializers (components are plain structs)
- No inheritance required for components

### Why FLUX_VIEW_PROPERTIES Macro?
- Provides consistent API across all components
- Enables designated initializers (C++23 feature)
- Avoids repetitive declarations
- Easy to extend with new common properties
- Zero runtime overhead

### Why FocusManager?
- Centralizes focus state management
- Automatic registration during layout
- Supports tab navigation out of the box
- Persistent focus across frames (via focus keys)
- Clean separation of concerns

## File Naming Conventions

- **Headers**: TitleCase.hpp (Types.hpp, View.hpp)
- **Sources**: TitleCase.cpp (Application.cpp, Window.cpp)
- **Main files**: lowercase main.cpp
- **Directories**: TitleCase (Core/, Views/, Graphics/)

## Include Convention

```cpp
// Full framework
#include <Flux.hpp>

// Selective includes
#include <Flux/Views/VStack.hpp>
#include <Flux/Core/Property.hpp>
```

## Implementation Status

### ✅ Implemented

**Core Systems:**
- Type system with Property<T> (stateful + configuration)
- View system with type erasure
- Immediate mode rendering pipeline
- Layout system with flexible sizing (expansionBias, compressionBias)
- ViewHelpers for unified rendering
- Focus management with tab navigation
- Comprehensive event system (17 events)

**Platform:**
- Wayland backend with NanoVG
- OpenGL ES 2 hardware acceleration
- Window management (resize, fullscreen, decorations)
- Mouse input (move, click, scroll)
- Keyboard input (keys, text, modifiers)
- Cursor management with context-sensitive shapes

**Components (16):**
- Layout: VStack, HStack, Grid, Spacer, StackLayout
- Basic UI: Text, Button
- Form Inputs: Checkbox, RadioButton, Toggle, Slider
- Visual: Image, SVG, Badge, Divider, ProgressBar
- Custom rendering support

**Examples (32+):**
- Hello World, Counter, Todo App
- Calculator, Dashboard, Clock
- Focus & Keyboard Demo
- Event System Demo
- Color Picker, ScrollArea Demo
- Component Showcase
- And many more!

### 🚧 Not Yet Implemented

See `docs/ROADMAP.md` for detailed feature roadmap.

## Summary

Flux achieves automatic reactivity through:
1. **Lambda properties** - Evaluate fresh each frame
2. **Stateful properties** - Request redraws on changes
3. **Immediate mode** - UI rebuilt every frame
4. **Focus management** - Keyboard events routed to focused view
5. **Event system** - Declarative event handling on all views

This creates a simple, predictable, and powerful reactive system with minimal complexity. No complex binding, no retained tree synchronization - just pure C++23 with modern UI patterns!
