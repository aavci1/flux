# SVG Demo

This demo showcases the SVG view component in the Flux UI framework. The SVG view allows you to render SVG strings directly in your UI using NanoVG for high-quality graphics rendering.

## Features

- **SVG String Parsing**: Parses SVG strings and extracts basic elements
- **Supported Elements**:
  - `circle` - Circles with fill and stroke support
  - `rect` - Rectangles with fill and stroke support
  - `line` - Lines with stroke support
  - `path` - SVG paths with fill and stroke support (basic commands: M, L, H, V, C, Q, Z)
- **Color Support**: Named colors (red, blue, green, etc.) and hex colors (#RRGGBB, #RGB)
- **Scaling**: Automatically scales SVG content to fit within the view bounds
- **Integration**: Full integration with Flux's layout system and styling

## Usage

```cpp
#include <Flux.hpp>

using namespace flux;

// Define an SVG string
std::string circleSVG = R"(
    <svg width="100" height="100" xmlns="http://www.w3.org/2000/svg">
        <circle cx="50" cy="50" r="40" fill="red" stroke="black" stroke-width="3"/>
    </svg>
)";

// Use in your UI
SVG {
    .content = circleSVG,
    .backgroundColor = Colors::white,
    .padding = EdgeInsets{10},
    .cornerRadius = 10,
    .shadow = Shadow{2, 2, 4}
}
```

## SVG Properties

- `content`: The SVG string to render
- `tintColor`: Optional tint color to apply to the SVG
- `preserveAspectRatio`: Whether to preserve aspect ratio when scaling

## Supported SVG Attributes

### Circle
- `cx`, `cy`: Center coordinates
- `r`: Radius
- `fill`: Fill color
- `stroke`: Stroke color
- `stroke-width`: Stroke width

### Rectangle
- `x`, `y`: Position
- `width`, `height`: Dimensions
- `fill`: Fill color
- `stroke`: Stroke color
- `stroke-width`: Stroke width

### Line
- `x1`, `y1`: Start coordinates
- `x2`, `y2`: End coordinates
- `stroke`: Stroke color
- `stroke-width`: Stroke width

### Path
- `d`: Path data string containing SVG path commands
- `fill`: Fill color
- `stroke`: Stroke color
- `stroke-width`: Stroke width

## Color Formats

- Named colors: `red`, `blue`, `green`, `yellow`, `black`, `white`, `cyan`, `magenta`
- Hex colors: `#FF0000`, `#F00`
- Transparent: `none` or empty string

## Path Commands

The SVG path renderer supports the following basic SVG path commands:

- `M` / `m` - Move to (absolute/relative)
- `L` / `l` - Line to (absolute/relative)
- `H` / `h` - Horizontal line to (absolute/relative)
- `V` / `v` - Vertical line to (absolute/relative)
- `C` / `c` - Cubic Bezier curve (absolute/relative)
- `Q` / `q` - Quadratic Bezier curve (absolute/relative)
- `Z` / `z` - Close path

## Limitations

- Currently supports basic shapes and simple paths
- Complex path features like arc commands (A) are not yet implemented
- SVG groups and nested elements are not supported
- Transformations are not yet implemented
- Path parsing is basic and may not handle all edge cases

## Building and Running

```bash
cd build
make svg_demo
./svg_demo
```

The demo will show various SVG examples including circles, rectangles, lines, and complex path-based graphics like the Nemo fish illustration.
