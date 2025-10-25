# ScrollArea Component Demo

This example demonstrates the ScrollArea component in Flux, which provides scrollable content with mouse wheel support.

## Features Demonstrated

### ScrollArea Component
- **Mouse Wheel Scrolling**: Use mouse wheel or touchpad to scroll content
- **Content Clipping**: Content is automatically clipped to the visible area
- **Smooth Scrolling**: Configurable scroll sensitivity for smooth interactions
- **Automatic Sizing**: ScrollArea expands to fill available space

## Key Concepts

### ScrollArea Properties

```cpp
ScrollArea {
    .children = {/* content views */},           // Content to scroll
    .scrollX = 0.0f,                             // Horizontal scroll position
    .scrollY = 0.0f,                             // Vertical scroll position
    .scrollSensitivity = 1.0f,                   // Mouse wheel sensitivity
    .expansionBias = 1.0f                        // Allow component to expand
}
```

### Layout Behavior

The ScrollArea uses `expansionBias` to fill available vertical space:
- Returns minimal preferred size (just padding)
- Relies on `expansionBias` to grow into available space
- Content larger than viewport becomes scrollable

## Usage

### Basic Usage

```cpp
ScrollArea {
    .children = {
        Text { .value = "Item 1" },
        Text { .value = "Item 2" },
        // ... more content
    },
    .expansionBias = 1.0f  // Expand to fill parent
}
```

### With Custom Styling

```cpp
ScrollArea {
    .children = myContent,
    .backgroundColor = Colors::white,
    .borderColor = Colors::lightGray,
    .borderWidth = 1,
    .cornerRadius = 8,
    .padding = 16,
    .expansionBias = 1.0f
}
```

### With Custom Scroll Sensitivity

```cpp
ScrollArea {
    .children = myContent,
    .scrollSensitivity = 2.0f  // Faster scrolling
}
```

## Interactive Features

1. **Mouse Wheel**: Hover over the scroll area and scroll with mouse wheel
2. **Content Clipping**: Content outside viewport is clipped and hidden
3. **Smooth Scrolling**: Adjust sensitivity for comfortable scrolling

## Building and Running

From the build directory:
```bash
cmake ..
make scroll_area_demo
./scroll_area_demo
```

## Implementation Notes

- The ScrollArea uses clipping to hide content outside the viewport
- Content is positioned with negative offset based on scroll position
- The component calculates content size from children if not explicitly set
- Mouse wheel events update the scroll position
- The scroll position is automatically clamped to valid ranges
- Border width is accounted for in clipping calculations
