# Scroll/Wheel Events Demo

This example demonstrates the mouse scroll/wheel event system in Flux.

## Features Demonstrated

### Basic Scroll Events
- **onScroll Callback**: Captures mouse wheel and touchpad scroll events
- **Event Parameters**: Access to mouse position (x, y) and scroll deltas (deltaX, deltaY)
- **Scroll Statistics**: Track total scroll events and accumulated scroll values

### Scroll to Zoom
- Uses scroll events to implement zoom functionality
- Demonstrates scaling views based on scroll input
- Limits zoom range between 0.5x and 3.0x

### Scroll to Pan
- Uses scroll events to implement panning/movement
- Demonstrates offset manipulation based on scroll input
- Shows how to implement drag-like behavior with scroll

## Key Concepts

### onScroll Event
```cpp
.onScroll = [](float x, float y, float deltaX, float deltaY) {
    // x, y: Mouse position relative to the view
    // deltaX: Horizontal scroll (positive = right, negative = left)
    // deltaY: Vertical scroll (positive = down, negative = up)
}
```

### Scroll Deltas
- **Vertical Scrolling**: `deltaY` changes (mouse wheel up/down)
- **Horizontal Scrolling**: `deltaX` changes (touchpad left/right)
- Values are in scroll units (typically pixels or lines)

## Usage

Hover your mouse over any of the interactive areas and:
- **Mouse Wheel**: Scroll up/down for vertical scrolling
- **Touchpad**: Use two-finger scroll for both vertical and horizontal
- Watch the real-time feedback as scroll events are captured

## Interactive Features

1. **Basic Scroll Detector**: Shows scroll direction, deltas, and statistics
2. **Zoom Control**: Scroll to zoom a box in/out
3. **Pan Control**: Scroll to move a box around
4. **Reset Buttons**: Reset individual sections or all states

## Building and Running

From the build directory:
```bash
make scroll_demo
./scroll_demo
```

