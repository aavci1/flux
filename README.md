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
    ├── Core/                   // Core system (15 headers)
    │   ├── Types.hpp          // Point, Rect, Color, etc.
    │   ├── Property.hpp       // Reactive properties & state
    │   ├── View.hpp           // Base view class
    │   ├── ViewHelpers.hpp    // View rendering helpers
    │   ├── Application.hpp    // App lifecycle
    │   ├── Window.hpp         // Window management
    │   ├── LayoutTree.hpp     // Layout management
    │   ├── Utilities.hpp      // Common utilities
    │   ├── KeyEvent.hpp       // Keyboard event types
    │   ├── FocusState.hpp     // Focus management
    │   ├── KeyboardInputHandler.hpp
    │   ├── MouseInputHandler.hpp
    │   ├── ShortcutManager.hpp
    │   ├── PlatformWindowFactory.hpp
    │   └── WindowEventObserver.hpp
    ├── Views/                  // UI components and layouts (16 headers)
    │   ├── Text.hpp           // Text display
    │   ├── Button.hpp         // Interactive button
    │   ├── VStack.hpp         // Vertical stack layout
    │   ├── HStack.hpp         // Horizontal stack layout
    │   ├── Spacer.hpp         // Flexible space
    │   ├── Grid.hpp           // Grid layout
    │   ├── StackLayout.hpp    // Stack layout utilities
    │   ├── Image.hpp          // Image display
    │   ├── SVG.hpp            // SVG rendering
    │   ├── Slider.hpp         // Value slider
    │   ├── Checkbox.hpp       // Checkbox input
    │   ├── RadioButton.hpp    // Radio button input
    │   ├── Toggle.hpp         // Toggle switch
    │   ├── Badge.hpp          // Badge indicator
    │   ├── Divider.hpp        // Visual divider
    │   └── ProgressBar.hpp    // Progress indicator
    ├── Graphics/               // Rendering (4 headers)
    │   ├── RenderContext.hpp
    │   ├── Renderer.hpp
    │   ├── NanoVGRenderer.hpp
    │   └── Path.hpp
    └── Platform/               // Platform support (4 headers)
        ├── PlatformWindow.hpp
        ├── WaylandWindow.hpp
        ├── WaylandProtocols.hpp
        └── WaylandCursor.hpp
```

## Components

**Layout Components (4):**
- VStack, HStack, Grid, Spacer

**Basic UI Components (2):**
- Text, Button

**Form Input Components (4):**
- Checkbox, RadioButton, Toggle, Slider

**Visual Components (5):**
- Image, SVG, Badge, Divider, ProgressBar

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

See `examples/` directory (32 examples):
- **01-hello-world** - Basic setup
- **03-counter** - Property system + reactivity
- **04-todo-app** - Dynamic lists and CRUD operations
- **05-colors-and-theming** - Color system and theming
- **09-flexbox-demo** - Flexible layouts with expansion/compression
- **10-justify-content-demo** - Content justification and alignment
- **11-automotive-dashboard** - Dashboard with gauges and indicators
- **14-svg-demo** - SVG rendering
- **15-render-context-demo** - Custom drawing with RenderContext
- **17-login-manager** - Form layout example
- **19-clock** - Animated clock with custom rendering
- **22-grid-demo** - Grid layout demonstration
- **23-spacer-grid-demo** - Grid with spacers
- **24-calculator** - Full calculator implementation
- **25-cursor-demo** - Custom cursor types
- **26-declarative-cursor** - Declarative cursor API
- **27-focus-keyboard-demo** - Focus management and keyboard input
- **28-event-system-demo** - Comprehensive event handling
- **29-color-picker** - Interactive color picker
- **31-component-showcase** - All components in one demo
- **33-scroll-area-demo** - ScrollArea component demonstration
- **32-label-position-demo** - Label positioning

And more!

## Building

### Prerequisites (Arch Linux)
```bash
sudo pacman -S wayland wayland-protocols libxkbcommon mesa freetype2 clang cmake stb glfw
```

### Prerequisites (Ubuntu/Debian)
```bash
sudo apt install libwayland-dev wayland-protocols libxkbcommon-dev \
                 libegl1-mesa-dev libgles2-mesa-dev libfreetype6-dev clang cmake libstb-dev libglfw3-dev
```

### Build

On first configure, CMake downloads **SDL3** (if not installed), **NanoVG**, **NanoSVG**, and **stb** into your build directory (`_deps/`), similar to SDL3’s fetch fallback. You need **Git** and network access for the first run.

To use fixed revisions, the defaults are `FLUX_NANOVG_GIT_TAG`, `FLUX_NANOSVG_GIT_TAG`, and `FLUX_STB_GIT_TAG` in `CMakeLists.txt` (overridable with `-D` on the `cmake` command line).

Optional offline override: check out those repos under `third_party/nanovg`, `third_party/nanosvg`, and `third_party/stb` (same layout as before); CMake will use them instead of fetching.

```bash
mkdir build && cd build
cmake ..
cmake --build .

# Run examples (when built)
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

This is a feature-rich framework implementation with:
- ✅ 39 headers (TitleCase convention)
  - 15 Core headers
  - 16 Views headers
  - 4 Graphics headers
  - 4 Platform headers
- ✅ 12 source files
- ✅ 16 view components
  - Layout: VStack, HStack, Grid, Spacer
  - Basic UI: Text, Button
  - Form Inputs: Checkbox, RadioButton, Toggle, Slider
  - Visual: Image, SVG, Badge, Divider, ProgressBar
- ✅ Flexible layout system (expansionBias, compressionBias)
- ✅ ViewHelpers for unified rendering
- ✅ 32+ comprehensive examples
- ✅ Concept-based view system (no inheritance required)
- ✅ Wayland backend with NanoVG hardware-accelerated rendering
- ✅ Comprehensive mouse event handling (7 events)
- ✅ Keyboard input and focus management
- ✅ Tab navigation and focus system
- ✅ 17 total event callbacks on all views
- ✅ Custom cursor support (9 cursor types)
- ✅ Shortcut manager for global keyboard shortcuts
- 🚧 Text input components (TextInput, TextArea)
- 🚧 Wayland clipboard protocol integration
- ✅ ScrollArea component with mouse wheel scrolling
- 🚧 ScrollView with virtualization

## Platform Support

Flux is **Linux/Wayland-only** by design. Cross-platform support is not a goal at this time.
This allows for:
- Deep Wayland protocol integration
- Native Linux experience
- Simplified codebase
- Better performance

## License

MIT License
