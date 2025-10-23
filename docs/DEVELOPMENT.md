# Development Guide

## Building Flux

### Prerequisites

- C++23 compiler (GCC 13+, Clang 17+)
- CMake 3.25+
- Make or Ninja

### Quick Build

```bash
mkdir build && cd build
cmake ..
make

# Run examples
./hello_world
./counter
./custom_drawing
```

## Project Structure

```
flux/
├── include/Flux/          17 headers
│   ├── Core/              10 files
│   ├── Views/             5 files
│   └── Graphics/          2 files
├── src/                   5 cpp files
│   ├── core/              3 files
│   └── graphics/          2 files
└── examples/              8 examples
```

## Code Style

### Naming Conventions

```cpp
// Headers and sources: TitleCase
Types.hpp, Property.hpp, Application.cpp

// Classes: TitleCase
class View, class Property, class VStack

// Variables: camelCase
int counter, std::string userName

// Constants: PascalCase or UPPER_CASE
const float MaxValue

// Private members: trailing underscore
private:
    std::unique_ptr<Renderer> renderer_;
```

### Include Order

```cpp
// 1. Corresponding header (for .cpp files)
#include <Flux/Core/Window.hpp>

// 2. Flux headers
#include <Flux/Core/Application.hpp>
#include <Flux/Graphics/Renderer.hpp>

// 3. Standard library
#include <iostream>
#include <memory>
```

## Implementing New Components

### Step 1: Create Header

```cpp
// include/Flux/Views/MyComponent.hpp
#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/ViewHelpers.hpp>

namespace flux {

struct MyComponent {
    FLUX_VIEW_PROPERTIES;  // Standard view properties

    Property<std::string> title;
    Property<int> value = 0;

    LayoutInfo layout(const Rect& bounds) const;
    View body() const;
    void render(RenderContext& ctx, const LayoutInfo& layout) const;
    Size preferredSize() const;
};

} // namespace flux
```

### Step 2: Add to Flux.hpp

```cpp
#include <Flux/Views/MyComponent.hpp>
```

### Step 3: Implement Methods

See COMPONENT_GUIDE.md for details.

## Implementing Render Backend

The framework now uses SDL2 for cross-platform rendering. For additional rendering backends, implement new RenderContext classes:

### 1. Create New RenderContext

```cpp
// src/graphics/VulkanRenderContext.cpp
#include <Flux/Graphics/RenderContext.hpp>

class VulkanRenderContext : public RenderContext {
public:
    void drawText(...) override {
        // Real Vulkan/Skia implementation
    }
    // ... other methods
};

std::unique_ptr<RenderContext> createRenderContext() {
    return std::make_unique<VulkanRenderContext>();
}
```

### 2. Update Window.cpp

```cpp
// Add your new render context factory function
std::unique_ptr<RenderContext> createYourRenderContext();

// In constructor, add backend selection logic:
if (backendToUse == WindowBackend::YourBackend) {
    renderContext_ = createYourRenderContext();
    renderer_ = std::make_unique<ImmediateModeRenderer>(renderContext_.get());
}
```

## Testing

### Manual Testing

Build and run examples:

```bash
cd build
./counter

# Default: SDL2 window with hardware-accelerated rendering
# All examples now use SDL2 for visual output
```

### Syntax Testing

```bash
g++ -std=c++23 -I./include -fsyntax-only examples/01-hello-world/main.cpp
```

## Debugging

### Enable Debug Output

SDL2RenderContext can be extended to print drawing commands to console for debugging.

### Adding Logging

```cpp
#include <iostream>

void render(...) {
    std::cout << "[DEBUG] Rendering " << typeid(*this).name() << "\n";
    // ...
}
```

## Contributing

### Adding a Component

1. Create `include/Flux/Views/YourComponent.hpp`
2. Define as plain struct with `FLUX_VIEW_PROPERTIES`
3. Add to `include/Flux.hpp`
4. Implement the three required methods (layout, body, render)
5. Implement preferredSize() method
6. Add example in `examples/`
7. Test

### Code Review Checklist

- [ ] TitleCase naming for files
- [ ] Plain struct (no inheritance)
- [ ] FLUX_VIEW_PROPERTIES macro included
- [ ] Includes updated in Flux.hpp
- [ ] Properties for user config
- [ ] Internal state marked mutable if needed
- [ ] Three-method pattern implemented correctly
- [ ] Uses ViewHelpers::renderView() in render()
- [ ] Example created
- [ ] Documentation updated

## Current Implementation Tasks

### Phase 1: Framework Core ✅
- View system
- Property system (unified stateful + configuration)
- Example code

### Phase 2: Render Backend ✅
- SDL2 backend implemented
- Hardware-accelerated rendering
- Text rendering with SDL_ttf
- Shape primitives

### Phase 3: Window Backend 🚧
- Window creation
- Event loop
- Input handling
- Mouse/keyboard events

### Phase 4: Platform Integration 📋
- Wayland protocol
- Surface management
- Display integration

## Performance Considerations

### Immediate Mode Efficiency

- UI tree rebuilt each frame (~60 fps)
- Keep builder functions fast
- Avoid heavy computation in builders
- Use lambdas only when needed

### Property Changes

- Multiple property changes batched per frame
- requestRedraw() is cheap (sets atomic flag)
- Only one render per frame regardless of changes

### Property Evaluation

- Direct values: zero overhead
- Stateful properties: shared_ptr + mutex overhead
- Lambdas: function call overhead

Use the simplest form that works!

## Summary

Flux development is straightforward:
- Follow TitleCase convention for files
- Use plain structs with FLUX_VIEW_PROPERTIES
- Use Properties for both configuration and reactivity
- Implement the three-method pattern (layout, body, render)
- Use ViewHelpers::renderView() for unified rendering

The framework handles the rest!


