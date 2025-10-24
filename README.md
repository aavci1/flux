# Flux UI Framework

A minimal, declarative UI framework for Linux/Wayland built with pure C++23.

## Key Features

- Pure C++23 with designated initializers
- Automatic reactive state management
- Flexible properties (stateful values, direct values, or lambdas)
- Immediate mode rendering
- Type-safe with compile-time checking
- Native Wayland support with hardware-accelerated rendering (NanoVG + OpenGL ES 2)

## Quick Start

```cpp
#include <Flux.hpp>
using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    Property counter = 0;

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

**Key pattern:** Use **lambdas in properties** to capture state. When state changes, properties are re-evaluated during rendering, giving you automatic reactivity!

## Framework Structure

```
include/
├── Flux.hpp                    // Main header - includes everything
└── Flux/
    ├── Core/                   // Core types and system (9 headers)
    │   ├── Types.hpp          // Point, Rect, Color, etc.
    │   ├── Property.hpp       // Reactive properties & state
    │   ├── View.hpp           // Base view class
    │   ├── ViewHelpers.hpp    // View rendering helpers
    │   ├── Application.hpp    // App lifecycle
    │   ├── Window.hpp         // Window management
    │   ├── LayoutTree.hpp     // Layout management
    │   ├── TextUtils.hpp      // Text utilities
    │   └── Utilities.hpp      // Common utilities
    ├── Views/                  // UI components and layouts (5 headers)
    │   ├── Text.hpp
    │   ├── Button.hpp
    │   ├── VStack.hpp
    │   ├── HStack.hpp
    │   └── Spacer.hpp
    └── Graphics/               // Rendering (2 headers)
        ├── RenderContext.hpp
        └── Renderer.hpp
```

## Components

**Layout Components (3):**
- VStack, HStack, Spacer

**UI Components (2):**
- Text, Button

## Property System

Properties are flexible and support three modes:

**1. Stateful Mode (Default)** - Changes automatically trigger UI updates:
```cpp
Property counter = 0;        // Stateful by default
Property name = "Alice";

counter++;                   // Triggers automatic redraw
name = "Bob";               // Triggers automatic redraw

// Lambda properties capture values
window.setRootView(
    Text {
        .value = [&]() {
            return std::format("Hello, {} (Count: {})", name, counter);
        }
    }
);
```

**2. Direct Value Mode** - Static values for configuration:
```cpp
Property<std::string> text = "Static";
```

**3. Lambda Mode** - Computed values evaluated each frame:
```cpp
Property<std::string> text = [&]() {
    return std::format("Count: {}", counter);
};
```

**How it works:**
- Modify state → triggers `requestRedraw()`
- On next frame → renderer evaluates all lambda properties
- Lambda properties read current state values
- UI renders with fresh data

## Documentation

- **[docs/DESIGN.md](docs/DESIGN.md)** - Design philosophy and core principles
- **[docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)** - System architecture and implementation details
- **[docs/COMPONENT_GUIDE.md](docs/COMPONENT_GUIDE.md)** - Guide to creating components
- **[docs/DEVELOPMENT.md](docs/DEVELOPMENT.md)** - Development and contribution guide

## Examples

See `examples/` directory:
- **01-hello-world** - Basic setup
- **02-stack-alignment-demo** - Text alignment and custom components
- **03-counter** - Property system + reactivity
- **04-todo-app** - Dynamic lists and CRUD operations
- **05-colors-and-theming** - Color system and theming
- **07-custom-drawing** - Custom graphics rendering
- **08-dashboard** - Chart components and business visualization
- **09-flexbox-demo** - Flexible layouts with expansion/compression
- **10-justify-content-demo** - Content justification and alignment

## Building

### Prerequisites (Arch Linux)
```bash
sudo pacman -S wayland wayland-protocols libxkbcommon mesa freetype2 clang
```

### Prerequisites (Ubuntu/Debian)
```bash
sudo apt install libwayland-dev wayland-protocols libxkbcommon-dev \
                 libegl1-mesa-dev libgles2-mesa-dev libfreetype6-dev clang
```

### Build
```bash
mkdir build && cd build
cmake ..
make

# Run examples
./hello_world
./counter
./calculator
```

## Include Patterns

```cpp
// Full framework
#include <Flux.hpp>

// Selective includes
#include <Flux/Views/VStack.hpp>
#include <Flux/Core/Property.hpp>
```

## Project Status

This is a minimal framework implementation with:
- ✅ 17 headers (TitleCase convention)
- ✅ 5 source files
- ✅ 10 view components (Text, Button, VStack, HStack, Spacer, Grid, Image, SVG, Slider)
- ✅ Flexible layout system (expansionBias, compressionBias)
- ✅ ViewHelpers for unified rendering
- ✅ 12+ comprehensive examples
- ✅ Concept-based view system (no inheritance required)
- ✅ Wayland backend with NanoVG hardware-accelerated rendering
- ✅ Basic mouse event handling
- 🚧 Keyboard input and focus management
- 🚧 Text input components (TextInput, TextArea)
- 🚧 Wayland clipboard protocol integration
- 🚧 ScrollView and list virtualization

## Platform Support

Flux is **Linux/Wayland-only** by design. Cross-platform support is not a goal at this time.
This allows for:
- Deep Wayland protocol integration
- Native Linux experience
- Simplified codebase
- Better performance

## License

MIT License
