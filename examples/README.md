# Flux Examples

This directory contains examples demonstrating core Flux concepts.

## Examples Overview

### 01. Hello World (~20 lines)
**Concepts:** Basic application setup, window creation, static UI

The simplest possible Flux application.

### 02. Stack Alignment Demo (~250 lines)
**Concepts:** VStack, HStack, text alignment combinations, custom components

Demonstrates all 9 combinations of text alignment (top/middle/bottom + left/center/right) using a VStack containing 3 HStacks. Shows how to create custom components with fixed sizes.

### 03. Counter (~50 lines)
**Concepts:** State management, lambda properties, layouts, buttons, reactivity

Demonstrates automatic reactivity through lambda properties. Shows the key pattern:

```cpp
Text {
    .value = [&]() {
        return std::format("Count: {}", counter);  // Lambda evaluates fresh!
    }
}
```

When `counter` changes, the lambda is re-evaluated on the next frame, showing the updated count.

### 04. Todo App (~150 lines)
**Concepts:** Dynamic lists, complex state, lambda closures, CRUD operations, nested layouts

A fully functional todo list app showcasing:
- `State<std::vector<T>>` for dynamic collections
- Adding/removing items from lists
- Lambda capture with mixed reference/value semantics
- Programmatic child generation in loops
- Styled cards with shadows and backgrounds

### 05. Colors and Theming (~150 lines)
**Concepts:** Color system, theming, color shortcuts, gradients

Demonstrates the Flux color system:
- Built-in color shortcuts (primary, secondary, success, warning, error)
- Custom colors with RGBA values
- Color transformations (lighten, darken, adjustSaturation)
- Applying colors to various view properties

### 07. Custom Drawing (~75 lines)
**Concepts:** Custom rendering, RenderContext API, drawing primitives

Shows how to create custom visual components:
- Drawing shapes (rectangles, circles, lines)
- Custom layout and rendering logic
- Using RenderContext drawing methods

### 08. Dashboard (~347 lines)
**Concepts:** Custom chart components, dashboard layout, business visualization, three-method pattern

A comprehensive dashboard example featuring:
- Line chart component for sales trends
- Bar chart component for revenue data
- Doughnut chart for market share
- Business dashboard layout with multiple charts
- Custom components using the standardized three-method pattern

### 09. Flexbox Demo (~200 lines)
**Concepts:** Flexible layouts, expansionBias, compressionBias, space distribution, CSS flexbox behavior

Demonstrates the flexible layout system with:
- Equal expansion (expansionBias = 1.0) for uniform sizing
- Different expansion ratios (1x, 2x, 1x) for proportional sizing
- Mixed expansion (fixed + flexible components)
- Compression behavior when space is limited
- Vertical stack expansion for equal-height columns

### 10. Justify Content Demo (~150 lines)
**Concepts:** Content justification, alignment modes, space distribution patterns

Explores HStack and VStack content justification:
- Start, center, end alignment
- Space between, space around, space evenly
- Visual comparison of all justification modes
- Practical layout patterns

### 17. Login Manager (~320 lines)
**Concepts:** Custom components, macOS Big Sur theme, real-time updates, system UI design

A macOS Big Sur-inspired login manager featuring:
- WhiteSur theme with gradient background
- Real-time clock and date display
- Custom circular user avatar component
- Translucent password input field
- System action buttons (emergency, restart, shutdown, switch user)
- Custom rendering with NanoVG graphics
- Modern UI design patterns

### 24. Calculator (~240 lines)
**Concepts:** Complex state management, arithmetic operations, Grid layout with colspan/rowspan, reactive UI updates

A fully functional calculator application featuring:
- Complete arithmetic operations (+, -, ×, ÷)
- Special functions (clear, plus/minus toggle, percentage)
- Decimal number support
- Error handling (division by zero)
- Grid layout with 4×5 button arrangement
- Colspan support for the "0" button spanning 2 columns
- Real-time display updates using lambda properties
- Professional calculator UI design with color-coded buttons

## Building Examples

Each example is a standalone application. To build an example:

```bash
cd <example-directory>
g++ -std=c++23 -o app main.cpp -lflux -lvulkan -lskia -ldrm -lgbm -linput
./app
```

Or using CMake:

```bash
cd <example-directory>
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/app
```

## Learning Path

Follow the examples in order:

1. **01-hello-world**: Basic setup
2. **02-stack-alignment-demo**: Stack components and text alignment
3. **03-counter**: State management and reactivity
4. **04-todo-app**: Dynamic lists and complex state
5. **05-colors-and-theming**: Color system and theming
6. **07-custom-drawing**: Custom rendering with RenderContext
7. **08-dashboard**: Custom chart components and business visualization
8. **09-flexbox-demo**: Flexible layouts and space distribution
9. **10-justify-content-demo**: Content justification and alignment modes
10. **17-login-manager**: Custom components and macOS Big Sur theme
11. **24-calculator**: Complex state management and calculator UI

## Example Structure

Each example contains:
- `main.cpp` - The complete source code
- Comments explaining key concepts
- Documentation of what's being demonstrated

## Requirements

All examples assume:
- Flux framework is installed and available
- C++23 compatible compiler
- Required system libraries (Vulkan, Skia, DRM, etc.)

## Notes

- These examples use only the public Flux API
- No framework internals are implemented here
- Source code is heavily commented for learning purposes
- Each example is self-contained and can run independently

## Contributing

Found an issue or want to add an example? Contributions are welcome!

1. Follow the existing example structure
2. Add comprehensive comments
3. Update this README with your example
4. Ensure the example demonstrates new concepts

## License

All examples are provided under the same license as the Flux framework (MIT).

