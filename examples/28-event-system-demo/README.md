# Event System Demo

A comprehensive demonstration of Flux's event handling system, showcasing all available event callbacks.

## What This Demo Shows

### Mouse Events
- **onClick**: Simple click handling
- **onMouseEnter/onMouseLeave**: Hover effects
- **onMouseMove**: Real-time cursor position tracking
- **onMouseDown**: Button-specific click detection (left, right, middle)
- **onDoubleClick**: Double-click handling

### Keyboard Events
- **onKeyDown**: Key press handling with special key detection
- **onKeyUp**: Key release handling
- **onTextInput**: Character input for text entry

### Focus Events
- **onFocus**: Visual feedback when gaining focus
- **onBlur**: Visual feedback when losing focus

### Interactive Features

1. **Hover Box**: Demonstrates `onMouseEnter`, `onMouseLeave`, and `onMouseMove`
   - Changes color when hovered
   - Tracks and displays mouse position
   - Responds to clicks

2. **Right-Click Detector**: Shows `onMouseDown` with button detection
   - Detects left, middle, and right mouse buttons
   - Displays which button was clicked and where

3. **Text Input Box**: Full keyboard event handling
   - Type characters with `onTextInput`
   - Backspace support with `onKeyDown`
   - Enter and Escape key handling
   - Arrow key detection
   - I-beam cursor when focused

4. **Reset Buttons**: Demonstrates `onClick` on multiple buttons

## Running the Demo

```bash
cd build
./event_system_demo
```

## Features Demonstrated

### Event Callbacks in FLUX_VIEW_PROPERTIES

All these events are now available on every view component:

```cpp
VStack {
    // Mouse Events
    .onClick = []() { /* ... */ },
    .onMouseDown = [](float x, float y, int button) { /* ... */ },
    .onMouseUp = [](float x, float y, int button) { /* ... */ },
    .onMouseMove = [](float x, float y) { /* ... */ },
    .onMouseEnter = []() { /* ... */ },
    .onMouseLeave = []() { /* ... */ },
    .onDoubleClick = []() { /* ... */ },
    
    // Keyboard Events
    .onKeyDown = [](const KeyEvent& e) { return false; },
    .onKeyUp = [](const KeyEvent& e) { return false; },
    .onTextInput = [](const std::string& text) { /* ... */ },
    
    // Focus Events
    .onFocus = []() { /* ... */ },
    .onBlur = []() { /* ... */ }
}
```

## Code Highlights

### Dynamic Background on Hover
```cpp
VStack {
    .backgroundColor = [&isHovered]() {
        return isHovered ? Color::hex(0xe3f2fd) : Color::hex(0xffffff);
    },
    .onMouseEnter = [&isHovered]() { isHovered = true; },
    .onMouseLeave = [&isHovered]() { isHovered = false; }
}
```

### Text Input with Keyboard Handling
```cpp
VStack {
    .focusable = true,
    .cursor = CursorType::Text,
    .onTextInput = [&text](const std::string& input) {
        text = static_cast<std::string>(text) + input;
    },
    .onKeyDown = [&text](const KeyEvent& e) {
        if (e.key == Key::Backspace) {
            std::string current = text;
            if (!current.empty()) {
                text = current.substr(0, current.length() - 1);
            }
            return true;  // Event handled
        }
        return false;
    }
}
```

### Mouse Button Detection
```cpp
.onMouseDown = [](float x, float y, int button) {
    std::string buttonName;
    if (button == 0) buttonName = "Left";
    else if (button == 1) buttonName = "Middle";
    else if (button == 2) buttonName = "Right";
    
    std::cout << buttonName << " click at (" << x << ", " << y << ")" << std::endl;
}
```

## Event Propagation

Keyboard events return a boolean to control propagation:
- Return `true` to mark the event as handled (stops propagation)
- Return `false` to allow the event to bubble up

```cpp
.onKeyDown = [](const KeyEvent& e) {
    if (e.key == Key::Escape) {
        // Handle escape key
        return true;  // Stop propagation
    }
    return false;  // Let other handlers process
}
```

## Tips for Using Events

1. **Use onClick for simple interactions** - Most straightforward for buttons and clickable items

2. **Set focusable = true for keyboard events** - Required to receive keyboard input

3. **Use onMouseEnter/Leave for hover effects** - Better than tracking mouse move for simple hover states

4. **Capture state by reference in lambdas** - Allows modifying reactive state
   ```cpp
   Property<int> counter = 0;
   .onClick = [&counter]() { counter++; }
   ```

5. **Return true from keyboard handlers** - When you want to prevent further processing

6. **Use cursor property** - Set appropriate cursor type for interactive elements

## Related Documentation

- See `docs/EVENT_SYSTEM.md` for complete API reference
- See `EVENT_SYSTEM_IMPLEMENTATION.md` for technical details
- See other examples for specific use cases

