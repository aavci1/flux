# Flux Event System

## Overview

All views in Flux have access to a comprehensive set of event callbacks through the `FLUX_VIEW_PROPERTIES` macro. This provides a consistent, declarative way to handle user interactions across all components.

## Available Events

### Mouse Events

#### onClick
```cpp
std::function<void()> onClick = nullptr;
```
Triggered when the primary mouse button (left click) is pressed and released on the view. This is the most commonly used mouse event.

**Example:**
```cpp
Button {
    .text = "Click me!",
    .onClick = []() {
        std::cout << "Button clicked!" << std::endl;
    }
}
```

#### onMouseDown
```cpp
std::function<void(float x, float y, int button)> onMouseDown = nullptr;
```
Triggered when any mouse button is pressed down on the view. Provides the mouse coordinates relative to the view and the button index.

**Parameters:**
- `x`, `y`: Mouse position relative to the view
- `button`: Mouse button (0 = left, 1 = middle, 2 = right)

**Example:**
```cpp
Text {
    .value = "Right-click me",
    .onMouseDown = [](float x, float y, int button) {
        if (button == 2) {
            std::cout << "Right-clicked at (" << x << ", " << y << ")" << std::endl;
        }
    }
}
```

#### onMouseUp
```cpp
std::function<void(float x, float y, int button)> onMouseUp = nullptr;
```
Triggered when a mouse button is released over the view.

#### onMouseMove
```cpp
std::function<void(float x, float y)> onMouseMove = nullptr;
```
Triggered when the mouse moves within the view's bounds. Useful for tracking cursor position or implementing custom hover effects.

**Example:**
```cpp
VStack {
    .onMouseMove = [](float x, float y) {
        std::cout << "Mouse at: " << x << ", " << y << std::endl;
    }
}
```

#### onMouseEnter
```cpp
std::function<void()> onMouseEnter = nullptr;
```
Triggered when the mouse cursor enters the view's bounds.

**Example:**
```cpp
Text {
    .value = "Hover over me",
    .onMouseEnter = []() {
        std::cout << "Mouse entered!" << std::endl;
    }
}
```

#### onMouseLeave
```cpp
std::function<void()> onMouseLeave = nullptr;
```
Triggered when the mouse cursor leaves the view's bounds.

#### onDoubleClick
```cpp
std::function<void()> onDoubleClick = nullptr;
```
Triggered when the view is double-clicked with the primary mouse button.

#### onScroll
```cpp
std::function<void(float x, float y, float deltaX, float deltaY)> onScroll = nullptr;
```
Triggered when the mouse wheel or touchpad scroll is used over the view. Provides the mouse position and scroll deltas.

**Parameters:**
- `x`, `y`: Mouse position relative to the view when scrolling
- `deltaX`: Horizontal scroll delta (positive = right, negative = left)
- `deltaY`: Vertical scroll delta (positive = down, negative = up)

**Example:**
```cpp
VStack {
    .onScroll = [](float x, float y, float deltaX, float deltaY) {
        std::cout << "Scrolled at (" << x << ", " << y << ") "
                  << "deltaX=" << deltaX << " deltaY=" << deltaY << std::endl;
    }
}
```

### Keyboard Events

#### onKeyDown
```cpp
std::function<bool(const KeyEvent&)> onKeyDown = nullptr;
```
Triggered when a key is pressed while the view has focus. Return `true` to mark the event as handled and prevent further propagation.

**Example:**
```cpp
VStack {
    .focusable = true,
    .onKeyDown = [](const KeyEvent& event) {
        if (event.key == Key::Escape) {
            std::cout << "Escape pressed!" << std::endl;
            return true;  // Event handled
        }
        return false;  // Event not handled
    }
}
```

#### onKeyUp
```cpp
std::function<bool(const KeyEvent&)> onKeyUp = nullptr;
```
Triggered when a key is released while the view has focus.

#### onTextInput
```cpp
std::function<void(const std::string&)> onTextInput = nullptr;
```
Triggered when text input is received (typically from keyboard input). This is useful for implementing text fields and text-based input.

**Example:**
```cpp
Text {
    .focusable = true,
    .onTextInput = [&text](const std::string& input) {
        text = text + input;
    }
}
```

### Focus Events

#### onFocus
```cpp
std::function<void()> onFocus = nullptr;
```
Triggered when the view gains focus. Set `focusable = true` to enable focus.

**Example:**
```cpp
Button {
    .text = "Focus me",
    .focusable = true,
    .onFocus = []() {
        std::cout << "Button gained focus" << std::endl;
    }
}
```

#### onBlur
```cpp
std::function<void()> onBlur = nullptr;
```
Triggered when the view loses focus.

### Value Change Events

#### onChange
```cpp
std::function<void()> onChange = nullptr;
```
Generic callback for when a view's value changes. Useful for custom components that manage state.

### Drag and Drop Events

The following drag and drop events are available for future implementation:

#### onDragStart
```cpp
std::function<void(float x, float y)> onDragStart = nullptr;
```
Triggered when a drag operation begins.

#### onDrag
```cpp
std::function<void(float x, float y)> onDrag = nullptr;
```
Triggered continuously during a drag operation.

#### onDragEnd
```cpp
std::function<void(float x, float y)> onDragEnd = nullptr;
```
Triggered when a drag operation ends.

#### onDrop
```cpp
std::function<void(float x, float y)> onDrop = nullptr;
```
Triggered when something is dropped on the view.

## Event Handling Priority

For components that define both custom event handlers (like `Button::handleKeyDown`) and event callbacks (like `onKeyDown`), the custom handler is checked first. If it returns `false` or doesn't handle the event, the callback is invoked.

This allows components to provide default behavior while still allowing users to override or extend it with callbacks.

## Interactive Views

A view is considered interactive (and will receive mouse events) if any of these callbacks are set:
- `onClick`
- `onMouseDown`
- `onMouseUp`
- `onMouseMove`
- `onMouseEnter`
- `onMouseLeave`
- `onDoubleClick`
- `onScroll`

## Best Practices

### 1. Use onClick for Simple Button-Like Interactions
```cpp
Text {
    .value = "Click me",
    .onClick = []() { /* handle click */ }
}
```

### 2. Set focusable = true for Keyboard Events
```cpp
VStack {
    .focusable = true,
    .onKeyDown = [](const KeyEvent& e) { /* handle key */ return false; }
}
```

### 3. Return true from Keyboard Events to Stop Propagation
```cpp
.onKeyDown = [](const KeyEvent& e) {
    if (e.key == Key::Enter) {
        // Handle enter key
        return true;  // Stop propagation
    }
    return false;  // Let other handlers process
}
```

### 4. Capture State by Reference in Lambdas
```cpp
Property<int> counter = 0;

Button {
    .text = "Increment",
    .onClick = [&counter]() {
        counter++;  // Modifies the state
    }
}
```

### 5. Use onMouseEnter/onMouseLeave for Hover Effects
```cpp
VStack {
    .onMouseEnter = []() { /* show hover state */ },
    .onMouseLeave = []() { /* hide hover state */ }
}
```

## Examples

### Counter with Keyboard Support
```cpp
Property<int> counter = 0;

VStack {
    .focusable = true,
    .onKeyDown = [&counter](const KeyEvent& e) {
        if (e.key == Key::Up) {
            counter++;
            return true;
        } else if (e.key == Key::Down) {
            counter--;
            return true;
        }
        return false;
    },
    .children = {
        Text {
            .value = [&counter]() {
                return std::format("Count: {}", static_cast<int>(counter));
            }
        },
        Button {
            .text = "Increment",
            .onClick = [&counter]() { counter++; }
        }
    }
}
```

### Interactive Text with Multi-Event Support
```cpp
Property<std::string> status = "Ready";

Text {
    .value = [&status]() { return static_cast<std::string>(status); },
    .onMouseEnter = [&status]() { status = "Hovering..."; },
    .onMouseLeave = [&status]() { status = "Ready"; },
    .onClick = [&status]() { status = "Clicked!"; },
    .onDoubleClick = [&status]() { status = "Double-clicked!"; }
}
```

### Form Input with Text Input
```cpp
Property<std::string> text = "";

VStack {
    .children = {
        Text {
            .value = [&text]() { return "Input: " + static_cast<std::string>(text); }
        },
        VStack {
            .focusable = true,
            .onTextInput = [&text](const std::string& input) {
                text = static_cast<std::string>(text) + input;
            },
            .onKeyDown = [&text](const KeyEvent& e) {
                if (e.key == Key::Backspace && !text.get().empty()) {
                    std::string current = text;
                    text = current.substr(0, current.length() - 1);
                    return true;
                }
                return false;
            }
        }
    }
}
```

## Migration from Component-Specific Events

If you previously had components with their own `onClick` members:

**Before:**
```cpp
struct Button {
    FLUX_VIEW_PROPERTIES;
    Property<std::string> text;
    std::function<void()> onClick;  // Component-specific
};
```

**After:**
```cpp
struct Button {
    FLUX_VIEW_PROPERTIES;  // onClick is now included here
    Property<std::string> text;
};
```

All existing code using `.onClick` will continue to work without changes since `onClick` is now part of `FLUX_VIEW_PROPERTIES`.

