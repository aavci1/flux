# Colors and Theming Demo

A comprehensive showcase of Flux's color system and theming capabilities.

## What This Demo Shows

### 1. **Base Colors**
- White, Black, Gray, Dark Gray, Light Gray
- Essential neutral colors for any UI

### 2. **Semantic Colors (Brand & UI)**
- **Primary**: Main brand color
- **Secondary**: Supporting brand color
- **Accent**: Highlight and attention color
- **Text**: Default text color
- **Surface**: Card and surface backgrounds
- **Background**: Page background
- **Card BG**: Special card background
- **Transparent**: Fully transparent color

### 3. **Color Palette**
- Red, Blue, Green, Yellow
- Vibrant colors for various UI elements

### 4. **Status Colors**
- **Success**: Positive actions and confirmations (green)
- **Warning**: Cautionary states and alerts (orange)
- **Error**: Error states and destructive actions (red)

### 5. **Custom Colors**
Demonstrates two ways to create custom colors:
- `Color::hex(0xFF6B6B)` - Using hexadecimal color codes
- `Color::rgb(138, 43, 226)` - Using RGB values (0-255)

### 6. **Opacity Variations**
Shows the `.opacity()` method:
```cpp
Color::hex(0x2196F3).opacity(0.5)  // 50% opacity
```
Perfect for overlays, glassmorphism, and fade effects.

### 7. **Darken Method**
Demonstrates the `.darken()` method:
```cpp
Color::hex(0x4CAF50).darken(0.3)  // 30% darker
```
Useful for hover states, pressed buttons, and creating color variations.

### 8. **Practical Examples**
Real-world usage examples:
- Success alert (green with light background)
- Warning alert (orange with light background)
- Error alert (red with light background)

## Key Concepts

### Color Shortcuts with `_`
Flux provides convenient color shortcuts via the `_` proxy:
```cpp
.backgroundColor = _.primary
.color = _.white
.borderColor = _.error
```

### Custom Color Creation
Create custom colors for your brand:
```cpp
Color::hex(0x1A73E8)           // Google Blue
Color::rgb(29, 161, 242)       // Twitter Blue
```

### Color Manipulation
Transform colors dynamically:
```cpp
baseColor.opacity(0.8)         // 80% transparent
baseColor.darken(0.2)          // 20% darker
baseColor.opacity(0.5).darken(0.1)  // Chain methods!
```

### Semantic Color Usage
Use semantic colors for better maintainability:
- `_.success` for positive actions
- `_.error` for destructive actions
- `_.warning` for cautions
- `_.primary` for brand elements
- `_.text` for body text

## Code Highlights

### Creating a Color Swatch
```cpp
VStack {
    .expansionBias = 1.0f,
    .spacing = 12,
    .children = {
        // Color square
        VStack {
            .backgroundColor = _.primary,
            .cornerRadius = 12,
            .padding = 50,
            .shadow = {0, 4, 8}
        },
        // Label
        Text {
            .value = "Primary",
            .fontSize = 16,
            .fontWeight = _.medium
        }
    }
}
```

### Alert Box with Semantic Colors
```cpp
VStack {
    .backgroundColor = Color::hex(0x4CAF50).opacity(0.1),
    .borderColor = _.success,
    .borderWidth = 2,
    .cornerRadius = 12,
    .padding = 20,
    .children = { /* ... */ }
}
```

## Building and Running

```bash
cd examples/05-colors-and-theming
# Using CMake from the project root
cd ../..
mkdir -p build && cd build
cmake ..
make colors_and_theming
./colors_and_theming
```

Or view the HTML output:
```bash
open build/flux_output.html
```

## What You'll Learn

1. **All available color shortcuts** in Flux
2. **How to create custom colors** with hex and RGB
3. **Color manipulation methods** (opacity, darken)
4. **Semantic color usage** for maintainable UIs
5. **Practical patterns** for alerts and status indicators
6. **Color system best practices**

## Design Tips

1. **Use semantic colors** for consistency
2. **Leverage opacity** for overlays and subtle backgrounds
3. **Use darken()** for hover/pressed states
4. **Create color palettes** with custom hex colors
5. **Test contrast** for accessibility
6. **Keep color usage consistent** across your app

## Next Steps

After understanding colors, explore:
- **06-typography-showcase**: Text styling and fonts
- **07-button-styles-gallery**: Button variations with colors
- **08-shadows-and-elevation**: Depth and visual hierarchy

