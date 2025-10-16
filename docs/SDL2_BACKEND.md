# SDL2 Backend for Flux

## Overview

The Flux UI framework now includes SDL2 backend support for cross-platform windowing and hardware-accelerated rendering on Linux, macOS, and Windows.

## Features

- **Hardware-Accelerated Rendering**: Uses SDL2's renderer with VSync support
- **Retina/High-DPI Support**: Automatic DPI detection and scaling for crisp rendering on high-resolution displays
- **Cross-Platform Windowing**: Works on Linux, macOS, and Windows
- **Full Event Handling**: Mouse, keyboard, window resize, and close events
- **Automatic Backend Selection**: Automatically uses SDL2 if available
- **Required Dependency**: SDL2 is now required for building Flux

## Building with SDL2

### Prerequisites

**macOS:**
```bash
brew install sdl2
```

**Linux (Debian/Ubuntu):**
```bash
sudo apt-get install libsdl2-dev
```

**Linux (Fedora):**
```bash
sudo dnf install SDL2-devel
```

### Building

```bash
mkdir build && cd build
cmake ..
make
```

CMake will automatically detect SDL2 and enable the backend if available. You'll see:
```
-- SDL2 found - building with SDL2 backend support
```

## Usage

### Default Behavior (Auto-Select)

By default, Flux automatically selects the best available backend:

```cpp
Window window({
    .size = {800, 600},
    .title = "My Flux App"
    // backend = WindowBackend::Auto (default)
});
```

SDL2 is required for building Flux. The build will fail if SDL2 is not available.

### Explicitly Request SDL2

```cpp
Window window({
    .size = {800, 600},
    .title = "My Flux App",
    .backend = WindowBackend::SDL2
});
```

### Backend Selection

Only SDL2 backend is supported. The `WindowBackend::Simple` option has been removed.

## Retina/High-DPI Support

Flux automatically detects and supports high-DPI displays (like Retina displays on macOS) for crisp, high-resolution rendering.

### How It Works

1. **Automatic DPI Detection**: When creating a window, Flux queries the display's DPI using SDL2's `SDL_GetDisplayDPI()`
2. **Scale Factor Calculation**: DPI scale factors are calculated relative to 96 DPI (standard resolution)
3. **Coordinate Scaling**: All rendering coordinates are automatically scaled by the detected DPI factors
4. **Font Scaling**: Text rendering uses scaled font sizes to maintain readability
5. **High-DPI Window Creation**: Windows are created with `SDL_WINDOW_ALLOW_HIGHDPI` flag

### Example Output

On a Retina display (2x scaling), you'll see:
```
[SDL2Window] DPI detected - X: 192, Y: 192 -> Scale: 2x2
```

### Benefits

- **Crisp Text**: Text remains sharp and readable on high-DPI displays
- **Sharp Graphics**: All UI elements render at native resolution
- **Automatic**: No code changes required - works out of the box
- **Cross-Platform**: Works on macOS Retina, Windows high-DPI, and Linux high-DPI displays

## Architecture

### SDL2RenderContext

Implements the `RenderContext` interface using SDL2's 2D rendering API:

- `drawRect()` - Filled rectangles
- `drawRoundedRect()` - Rounded corner rectangles
- `drawCircle()` - Filled circles
- `drawLine()` - Lines
- `drawShadow()` - Shadow effects (multi-pass blur approximation)
- `drawText()` - Text placeholders (can be extended with SDL_ttf)

### SDL2Window

Manages the SDL2 window and renderer lifecycle:

- Initializes SDL2 on first window creation
- Creates SDL2 window with requested size and flags
- Creates hardware-accelerated SDL2 renderer with VSync
- Cleans up SDL2 on last window destruction

### Event Handling

The `Application` class processes SDL2 events:

- **Window Events**: Resize, expose, close
- **Mouse Events**: Motion, button press/release
- **Keyboard Events**: Key press/release, text input

Events are dispatched to the appropriate `Window` instance.

## Running Examples

All existing examples work with SDL2:

```bash
cd build

# Counter app with SDL2 window
./counter

# Dashboard demo
./dashboard

# Flexbox demo
./flexbox_demo
```

## Text Rendering with SDL_ttf

The SDL2 backend now includes **full text rendering support** using SDL_ttf!

### Features

- **TrueType Font Rendering**: High-quality text rendering using SDL_ttf
- **Font Caching**: Fonts are cached by size and weight for performance
- **Multiple Font Paths**: Automatically searches common system font paths:
  - macOS: `/System/Library/Fonts/Helvetica.ttc`, `/System/Library/Fonts/SFNS.ttf`
  - Linux: `/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf`, `/usr/share/fonts/TTF/DejaVuSans.ttf`
  - Windows: `C:\Windows\Fonts\arial.ttf`
- **Font Styles**: Supports regular and bold weights
- **Text Alignment**: Supports leading, center, and trailing alignment
- **Accurate Measurement**: Precise text width/height measurement
- **Graceful Fallback**: If SDL_ttf is not available, falls back to placeholder rendering

### Installation

**macOS:**
```bash
brew install sdl2_ttf
```

**Linux (Debian/Ubuntu):**
```bash
sudo apt-get install libsdl2-ttf-dev
```

**Linux (Fedora):**
```bash
sudo dnf install SDL2_ttf-devel
```

After installing SDL2_ttf, rebuild the project:
```bash
cd build
cmake ..
make
```

You should see: `-- SDL2_ttf found - enabling text rendering`

## Current Limitations

1. **Image Loading**: Image support is placeholder. Can be extended with SDL_image.
2. **Line Width**: SDL2 doesn't support line width directly; thick lines need manual implementation.
3. **Rotation**: SDL2 renderer doesn't support arbitrary rotation; needs texture-based rendering.
4. **Font Selection**: Currently uses system default fonts; custom font paths not yet configurable.

## Future Enhancements

- [x] ~~Integrate SDL_ttf for proper text rendering with font loading and caching~~ **DONE!**
- [ ] Add configurable custom font paths
- [ ] Add SDL_image support for image loading (PNG, JPEG, etc.)
- [ ] Implement texture caching for better performance
- [ ] Add GPU-accelerated shadow and blur effects
- [ ] Support for custom SDL2 renderer flags
- [ ] Multi-window support improvements
- [ ] Support for more font styles (italic, underline, strikethrough)

## Implementation Files

- `include/Flux/Core/SDL2Window.hpp` - SDL2 window wrapper header
- `src/core/SDL2Window.cpp` - SDL2 window implementation
- `src/graphics/SDL2RenderContext.cpp` - SDL2 render context implementation
- `include/Flux/Core/WindowBackend.hpp` - Backend selection enum
- `src/core/Window.cpp` - Backend selection logic
- `src/core/Application.cpp` - SDL2 event processing

## Text Measurement Consistency

The SDL2 backend uses **unified text measurement** via `SDL2TextUtils` to ensure layout calculations match rendering exactly:

- **Problem**: Previously, layout used FreeType while rendering used SDL_ttf, causing mismatches
- **Solution**: Both layout and rendering now use SDL_ttf measurements
- **Result**: Text is properly sized, aligned, and never cut off

This ensures that:
- `Text::preferredSize()` returns accurate dimensions
- Text containers are sized correctly
- Alignment is pixel-perfect
- Bold/different font weights are measured accurately

## Testing

The SDL2 backend has been tested with:
- ✅ Window creation and destruction
- ✅ Hardware-accelerated rendering
- ✅ **Retina/High-DPI rendering** with automatic DPI detection and scaling
- ✅ **Real text rendering with SDL_ttf**
- ✅ **Consistent text measurements** between layout and rendering
- ✅ **Correct text alignment** (leading, center, trailing)
- ✅ **Accurate vertical alignment** (top, center, bottom) with font metrics
- ✅ Font caching and management
- ✅ Multiple font sizes and weights
- ✅ Window resize without crashes
- ✅ Event handling (mouse, keyboard, resize, close)
- ✅ All existing Flux examples
- ✅ Automatic backend selection
- ✅ SDL2 as required dependency

## Troubleshooting

### "SDL2 not found" during CMake configuration

Install SDL2 development libraries for your platform (see Prerequisites above).

### Window opens but shows placeholder text instead of real text

If you see placeholder rectangles instead of rendered text:

1. Check that SDL2_ttf is installed: `brew list | grep sdl2_ttf` (macOS) or `dpkg -l | grep libsdl2-ttf` (Linux)
2. Check CMake output for: `-- SDL2_ttf found - enabling text rendering`
3. If SDL2_ttf was just installed, rebuild: `cd build && cmake .. && make`
4. Check console output for font loading errors

The system will automatically fall back to placeholder rendering if SDL_ttf is not available or if no system fonts can be found.

### Build errors about SDL.h

Make sure SDL2 include directories are in your system's include path. On macOS with Homebrew, this should be automatic.

