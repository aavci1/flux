# Flux Examples

This directory contains examples demonstrating core Flux concepts.

## Examples Overview

### 19. Clock (~50 lines)
**Concepts:** Custom rendering, real-time updates, trigonometry, animation

An analog clock rendered with custom drawing, demonstrating:
- Real-time updates with time-based rendering
- Custom drawing with lines and circles
- Trigonometric calculations for clock hands

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

### LLM Studio (~500 lines)
**Concepts:** Multi-view application, sidebar navigation, complex state, theming, custom components

A multi-view LLM chat studio application featuring:
- Sidebar navigation with icon rail
- Chat view with message bubbles
- Settings view with configuration options
- Hub view for model browsing
- Custom theme system
- Modular component architecture

## Building Examples

All examples are built automatically with the main project:

```bash
# From project root
mkdir build && cd build
cmake ..
make

# Run any example
./clock
./calculator
./llm_studio
```

## Learning Path

1. **19-clock**: Custom rendering and real-time updates
2. **24-calculator**: Complex state management and grid layout
3. **llm-studio**: Multi-view architecture and sidebars

## Example Structure

Each example contains:
- `main.cpp` - The complete source code
- Additional headers for larger examples (e.g., `AppState.hpp`, view headers)

## Requirements

All examples require:
- C++23 compatible compiler (Clang or GCC recommended)
- CMake 3.25+
- OpenGL
- Freetype2
- SDL3 (fetched automatically if not found)

Platform support:
- **macOS** (tested)
- **Linux** (X11 and Wayland)
- **Windows** (MSVC support in progress)

## Notes

- These examples use only the public Flux API
- No framework internals are implemented here
- Each example is self-contained and can run independently

## Contributing

Found an issue or want to add an example? Contributions are welcome!

1. Follow the existing example structure
2. Add comprehensive comments
3. Update this README with your example
4. Ensure the example demonstrates new concepts

## License

All examples are provided under the same license as the Flux framework (MIT).
