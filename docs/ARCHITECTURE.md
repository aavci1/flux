# Flux Architecture

## Overview

Flux is a minimal immediate mode UI framework built on C++23 with automatic reactivity.

## Directory Structure

```
flux/
├── include/
│   ├── Flux.hpp                    Main header
│   └── Flux/
│       ├── Core/                   Core system (10 headers)
│       │   ├── Types.hpp          Geometry, colors, enums
│       │   ├── State.hpp          Reactive state
│       │   ├── Property.hpp       Flexible properties
│       │   ├── View.hpp           Base view class
│       │   ├── ViewHelpers.hpp    View rendering helpers
│       │   ├── Application.hpp    App lifecycle
│       │   ├── Window.hpp         Window management
│       │   ├── LayoutTree.hpp     Layout management
│       │   ├── TextUtils.hpp      Text utilities
│       │   └── Utilities.hpp      Common utilities
│       ├── Views/                  Components + Layouts (5 headers)
│       │   ├── Text.hpp
│       │   ├── Button.hpp
│       │   ├── VStack.hpp
│       │   ├── HStack.hpp
│       │   └── Spacer.hpp
│       └── Graphics/               Rendering (2 headers)
│           ├── RenderContext.hpp
│           └── Renderer.hpp
├── src/
│   ├── core/
│   │   ├── application.cpp        Event loop, window management
│   │   ├── window.cpp             Window implementation
│   │   └── textutils.cpp          Text utilities
│   └── graphics/
│       └── SDL2RenderContext.cpp      SDL2 hardware-accelerated renderer
└── examples/
    ├── 01-hello-world/
    ├── 02-stack-alignment-demo/
    ├── 03-counter/
    ├── 04-todo-app/
    ├── 05-colors-and-theming/
    ├── 07-custom-drawing/
    ├── 08-dashboard/
    ├── 09-flexbox-demo/
    └── 10-justify-content-demo/
```

## Core Types

### View Hierarchy

```cpp
// Concept - compile-time contract (no inheritance)
template<typename T>
concept ViewComponent = requires(const T& ct, ...) {
    { ct.layout(bounds) } -> std::same_as<LayoutInfo>;
    { ct.body() } -> std::same_as<std::vector<View>>;
    { ct.render(ctx, layout) } -> std::same_as<void>;
    { ct.preferredSize() } -> std::same_as<Size>;
};

// ViewInterface - runtime polymorphism
class ViewInterface {
    virtual LayoutInfo layout(const Rect&) const = 0;
    virtual View body() const = 0;
    virtual void render(RenderContext&, const LayoutInfo&) const = 0;
    virtual Size preferredSize() const = 0;
    virtual std::unique_ptr<ViewInterface> clone() const = 0;
};

// ViewAdapter - bridges concepts to interface
template<ViewComponent T>
class ViewAdapter : public ViewInterface {
    T component;
    // Forwards all calls to component
};

// View - type-erased container (public API)
class View {
    std::unique_ptr<ViewInterface> component_;
    // Supports any ViewComponent via templated constructor
};
```

### State System

```cpp
template<typename T>
class State {
    T value;
    std::shared_mutex mutex;

    void notifyChange() {
        Application::instance().requestRedraw();
    }
};
```

### Property System

```cpp
template<typename T>
class Property {
    std::variant<T, ref<State<T>>, function<T()>> storage;

    T get() const;  // Evaluates current value
};
```

## Rendering Pipeline

### Frame Cycle

```
1. State Change
   counter++ → notifyChange() → requestRedraw()

2. Application Event Loop
   if (needsRedraw) {
       for (window : windows) {
           window->render();
       }
   }

3. Window Render
   renderer_->renderFrame(bounds);

4. Renderer
   layout = rootView_.layout(bounds);
   rootView_.render(context, layout);
   context.present();

5. View Rendering
   Each view evaluates its properties:
   - Lambda properties → execute (gets current state!)
   - State refs → read current value
   - Direct values → use as-is
```

### Immediate Mode Flow

```
State Change
    ↓
requestRedraw()
    ↓
Next Frame
    ↓
Evaluate Lambda Properties (fresh state!)
    ↓
Layout Calculated
    ↓
Views Rendered
    ↓
(Next frame repeats)
```

## Component Implementation

### Leaf Component (Text)

```cpp
struct Text {  // Plain struct, no inheritance
    FLUX_VIEW_PROPERTIES;  // Common properties via macro

    Property<std::string> value;
    Property<float> fontSize = 16;

    LayoutInfo layout(const Rect& bounds) const {
        return LayoutInfo(bounds, preferredSize());
    }

    View body() const { return View{}; }

    void render(RenderContext& ctx, const LayoutInfo& layout) const {
        std::string text = value;  // Evaluate property
        float size = fontSize;      // Evaluate property
        ctx.drawText(text, layout.bounds.origin(), size, _.text);
    }

    Size preferredSize() const {
        return ctx.measureText(value, fontSize);
    }
};
```

### Container Component (VStack)

```cpp
struct VStack {  // Plain struct, no inheritance
    FLUX_VIEW_PROPERTIES;

    Property<std::vector<View>> children = {};
    Property<float> spacing = 0;
    mutable std::vector<Rect> childBounds_ = {};  // Cache

    LayoutInfo layout(const Rect& bounds) const {
        float y = bounds.y;
        std::vector<View> kids = children;  // Evaluate

        childBounds_.clear();
        for (auto& child : kids) {
            Size childSize = child.preferredSize();
            Rect childRect = {bounds.x, y, childSize.width, childSize.height};
            childBounds_.push_back(childRect);
            child.layout(childRect);
            y += childSize.height + spacing;
        }
        return LayoutInfo(bounds, preferredSize());
    }

    View body() const { return children; }

    void render(RenderContext& ctx, const LayoutInfo& layout) const {
        View child = body();
        if (child) {
            LayoutInfo childLayout = child.layout(childBounds_[0]);
            child.render(ctx, childLayout);
        }
    }

    Size preferredSize() const { /* calculate from children */ }
};
```

## Threading Model

- **State<T>**: Thread-safe with read-write locks
- **Rendering**: Single-threaded (main thread)
- **State modifications**: Can happen from any thread
- **Redraws**: Batched per frame (multiple state changes = one redraw)

## Memory Model

- **View tree**: Created fresh each frame, destroyed after render
- **State**: Lives across frames
- **View**: Type-erased container using unique_ptr<ViewInterface> for polymorphism
- **Properties**: Small variant (value, ref, or function)
- **ViewAdapter**: One per concrete type, stores component and forwards calls

## Key Decisions

### Why Immediate Mode?

- Simpler than retained mode
- No synchronization between logical/visual trees
- Predictable memory usage
- Easier to reason about

### Why View Builder Function?

- Captures state by reference
- Called fresh each frame
- Always shows current state
- No manual binding needed

### Why Lambda Properties?

- Evaluated during rendering
- Captures latest state
- Works with Property system
- Type-safe with concepts

### Why View (Type-Erased Container)?

- Can't store different component types by value in vectors
- View provides value semantics with type erasure
- Automatic via templated constructor (works with any ViewComponent)
- Supports designated initializers (components are plain structs)
- No inheritance required for components

## File Naming

- **Headers**: TitleCase.hpp (Types.hpp, View.hpp)
- **Sources**: TitleCase.cpp (Application.cpp, Window.cpp)
- **Main files**: lowercase main.cpp
- **Directories**: TitleCase (Core/, Views/, Graphics/)

## Include Convention

```cpp
// Full framework
#include <Flux.hpp>

// Selective
#include <Flux/Views/VStack.hpp>
#include <Flux/Core/State.hpp>
```

## Implementation Status

✅ Complete:
- Core type system
- State management
- Property system
- View system
- View builder pattern
- 5 view components (Text, Button, VStack, HStack, Spacer)
- Flexible layout system (expansionBias, compressionBias)
- ViewHelpers for unified rendering
- SDL2 backend with hardware-accelerated rendering
- 8 comprehensive examples

🚧 To Implement:
- Real RenderContext backend (Vulkan/Skia for Linux)
- Window backend (Wayland)
- Full mouse/keyboard event propagation to views

✅ SDL2 Backend (Cross-platform):
- SDL2RenderContext for hardware-accelerated rendering
- SDL2Window for cross-platform windowing (Linux, macOS, Windows)
- Full event handling (mouse, keyboard, resize, close)
- SDL2 as required dependency

## Summary

Flux achieves automatic reactivity through:
1. **Lambda properties** - Evaluate fresh each frame
2. **State triggers** - Request redraws on changes
3. **Frame batching** - Efficient updates
4. **Immediate evaluation** - Properties read current state

This creates a simple, predictable, and powerful reactive system with minimal complexity. No builder functions or complex binding needed - just lambda properties!

