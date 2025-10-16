# Flux Design Philosophy

## Overview

Flux is a minimal, declarative UI framework for C++23 featuring automatic reactive state management and immediate mode rendering.

## Core Principles

### 1. Pure C++23

Everything is standard C++23 - no code generation, no preprocessors:

```cpp
#include <Flux.hpp>
using namespace flux;

VStack {
    .children = {
        Text { .value = "Hello" },
        Button { .text = "Click" }
    }
}
```

### 2. Lambda Properties for Reactivity

Properties can be lambdas that capture state:

```cpp
State counter = 0;

window.setRootView(
    VStack {
        .children = {
            Text {
                .value = [&]() {         // Lambda property
                    return std::format("Count: {}", counter);
                }
            }
        }
    }
);
```

**How reactivity works:**
1. State change (e.g., `counter++`) triggers `requestRedraw()`
2. On next frame, renderer evaluates lambda properties
3. Lambda properties read current state values
4. UI renders with fresh data

No builder function needed - lambda properties are sufficient!

### 3. Lambda Properties

Properties accept three forms:

```cpp
Text {
    .value = "Static",                    // Direct value
    .value = username,                    // State reference
    .value = [&]() { return name; }      // Lambda (evaluated fresh)
}
```

**For reactive values, use lambdas:**
```cpp
// ❌ WRONG - captures value once
.value = std::format("Count: {}", counter)

// ✅ CORRECT - evaluates fresh each render
.value = [&]() { return std::format("Count: {}", counter); }
```

### 4. Immediate Mode Rendering

UI is re-rendered every frame with fresh property evaluations:

1. State changes trigger redraw
2. On next frame: all lambda properties evaluated
3. Layout calculated
4. Tree rendered with current values

**Benefits:**
- Simple mental model
- Predictable behavior
- No synchronization issues
- Easy to debug

## Component System

### View Components

All components are plain structs implementing the `ViewComponent` concept:

```cpp
struct Text {
    FLUX_VIEW_PROPERTIES;  // Common properties via macro

    Property<std::string> value;
    Property<float> fontSize = 16;

    LayoutInfo layout(const Rect& bounds) const;
    View body() const;
    void render(RenderContext& ctx, const LayoutInfo& layout) const;
    Size preferredSize() const;
};
```

No inheritance required - just implement the required methods!

### View Type Erasure

Views use concepts and type erasure for polymorphism. All views implement the `ViewComponent` concept and are automatically wrapped via `ViewAdapter` into `View` objects:

```cpp
// ViewComponent concept - compile-time contract
template<typename T>
concept ViewComponent = requires(const T& ct, ...) {
    { ct.layout(bounds) } -> std::same_as<LayoutInfo>;
    { ct.body() } -> std::same_as<std::vector<View>>;
    { ct.render(ctx, layout) } -> std::same_as<void>;
    { ct.preferredSize() } -> std::same_as<Size>;
};
```

Users compose views naturally:

```cpp
VStack {
    .children = {
        Text { ... },      // ViewComponent -> View
        Button { ... }     // ViewComponent -> View
    }
}
```

## State vs Property

### State<T> - Application State

Mutable state that triggers updates:

```cpp
State counter = 0;
State name = "Alice";

counter++;         // Triggers requestRedraw()
name = "Bob";     // Triggers requestRedraw()
```

### Property<T> - Component Properties

Component configuration that accepts three forms:

```cpp
Property<std::string> value;

// Can be assigned:
.value = "Static"              // Direct value
.value = username              // State reference
.value = [&]() { return ...; }  // Lambda
```

### Components Have Both

```cpp
struct Button {
    FLUX_VIEW_PROPERTIES;

    // Properties - user configures
    Property<std::string> text;
    Property<bool> disabled = false;

    // Internal state - component manages (mutable for const methods)
    mutable bool isHovered = false;
    mutable bool isPressed = false;
};
```

## Minimal Components

### Layout Components (3)
- **VStack** - Vertical stack with spacing and alignment
- **HStack** - Horizontal stack with spacing and alignment
- **Spacer** - Flexible space filler

### UI Components (2)
- **Text** - Text display with formatting and alignment
- **Button** - Interactive button with states and styling

## Architecture

```
┌─────────────────────────────────┐
│      User Code (main.cpp)       │
│  • State declarations           │
│  • View builder function        │
│  • Event handlers               │
├─────────────────────────────────┤
│      Flux Core Framework        │
│  • View system                  │
│  • State management             │
│  • Property system              │
│  • Layout engine                │
├─────────────────────────────────┤
│      Renderer                   │
│  • Immediate mode rendering     │
│  • Fresh tree each frame        │
├─────────────────────────────────┤
│      RenderContext              │
│  • Drawing primitives           │
│  • Text, shapes, etc.           │
└─────────────────────────────────┘
```

## Example Application

```cpp
#include <Flux.hpp>
using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    State counter = 0;

    Window window({.size = {400, 300}, .title = "Counter"});

    window.setRootView(
        VStack {
            .children = {
                Text {
                    .value = [&]() {
                        return std::format("Count: {}", counter);
                    },
                    .fontSize = 32
                },
                HStack {
                    .children = {
                        Button {
                            .text = "−",
                            .onClick = [&]() { counter--; }
                        },
                        Button {
                            .text = "+",
                            .onClick = [&]() { counter++; }
                        }
                    }
                }
            }
        }
    );

    return app.exec();
}
```

## Design Goals

1. **Minimal** - Only essential features
2. **Simple** - Easy to understand and implement
3. **Reactive** - Automatic UI updates from state via lambda properties
4. **Type-safe** - Full compile-time checking
5. **Standard** - Pure C++23, no extensions

## What Makes Flux Unique

- Lambda properties for automatic reactivity (no builder functions needed)
- Three property forms (value, State ref, lambda)
- Concept-based View system (no inheritance required)
- Plain structs with designated initializers
- Flexible layout system (expansionBias, compressionBias)
- ViewHelpers for unified rendering
- TitleCase convention
- Absolutely minimal (17 headers, 5 view components)

