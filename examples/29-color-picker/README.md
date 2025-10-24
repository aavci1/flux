# Color Picker

A beautiful color palette application that displays 90 carefully curated colors in a grid layout.

## Features

- **90 Beautiful Colors**: A curated collection of colors organized by category:
  - Reds & Pinks (10 colors)
  - Oranges & Yellows (10 colors)
  - Greens (10 colors)
  - Blues (10 colors)
  - Purples (10 colors)
  - Browns & Neutrals (10 colors)
  - Grays & Blacks (10 colors)
  - Unique & Exotic colors (10 colors)

- **One-Click Copy**: Click any color cell to copy its hex code to the clipboard
- **Smart Text Contrast**: Automatically chooses black or white text based on background color luminance
- **Responsive Grid**: 10×9 grid layout that fills the entire window
- **Visual Feedback**: Status bar shows the last copied color hex code
- **Hover Effects**: Pointer cursor on hover to indicate clickable elements

## Usage

Simply click on any color cell to copy its hex code to your clipboard. The status bar at the top will show which color was copied.

## Technical Details

**Concepts Demonstrated:**
- Custom component creation with `ColorCell`
- Grid layout with fixed columns and rows
- Click event handling with `onClick`
- Clipboard integration using `wl-copy` (Wayland)
- Dynamic text rendering with contrast calculation
- Reactive state updates with `Property<T>`
- Lambda-based dynamic UI updates

**Color Cell Component:**
The app features a custom `ColorCell` component that:
- Displays the color name and hex code
- Automatically adjusts text color for contrast
- Handles click events for clipboard copying
- Has rounded corners for modern appearance

**Clipboard Support:**
Uses `wl-copy` command for Wayland clipboard integration. Make sure you have `wl-clipboard` package installed:
```bash
sudo apt install wl-clipboard  # Debian/Ubuntu
```

## Running

```bash
cd build
./color_picker
```

## Code Size

Approximately 280 lines including the complete color palette data.

