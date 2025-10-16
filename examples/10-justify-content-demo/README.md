# JustifyContent Demo

This example demonstrates the `justifyContent` property for `HStack` and `VStack` components.

## JustifyContent Options

The `justifyContent` property controls how children are distributed along the main axis of a stack:

### For HStack (Horizontal Distribution):
- **`start`** - Children are packed at the start (left side)
- **`center`** - Children are centered as a group
- **`end`** - Children are packed at the end (right side)
- **`spaceBetween`** - First item at start, last at end, equal space between items
- **`spaceAround`** - Equal space around each item (half space at edges)
- **`spaceEvenly`** - Equal space everywhere including edges

### For VStack (Vertical Distribution):
- **`start`** - Children are packed at the start (top)
- **`center`** - Children are centered as a group
- **`end`** - Children are packed at the end (bottom)
- **`spaceBetween`** - First item at top, last at bottom, equal space between items
- **`spaceAround`** - Equal space around each item (half space at edges)
- **`spaceEvenly`** - Equal space everywhere including edges

## Usage

```cpp
HStack {
    .justifyContent = JustifyContent::center,
    .spacing = 10,
    .children = { /* ... */ }
}

VStack {
    .justifyContent = JustifyContent::spaceBetween,
    .spacing = 10,
    .children = { /* ... */ }
}
```

## Building and Running

```bash
cd build
cmake ..
make justify_content_demo
./justify_content_demo
```

Or use CMake directly:
```bash
cd build
cmake ..
make justify_content_demo
./justify_content_demo
```

## Note

The `justifyContent` property is inspired by CSS Flexbox and provides the same behavior for laying out children along the main axis of a stack container.

