# Component Showcase Demo

A comprehensive demonstration of Flux UI Framework's complete component library, showcasing both existing and newly implemented interactive components.

## Featured Components

### Form Controls
- **Checkbox** - Checkable items with labels and keyboard support (Space/Enter)
- **Toggle/Switch** - Modern on/off switch with smooth animations
- **Radio Buttons** - Mutually exclusive selection groups
- **Slider** - Interactive range slider with mouse drag and keyboard navigation

### Visual Feedback
- **Progress Bar** - Determinate (percentage) and indeterminate (loading) modes
- **Badge** - Compact labels for counts, status, and tags
- **Divider** - Visual separators (horizontal and vertical)

### Existing Components
- **Button** - Interactive buttons with focus states
- **Text** - Styled text rendering
- **VStack/HStack** - Flexible layout containers
- **Grid** - Grid-based layouts

## Key Features Demonstrated

### Full Interactivity
- ✅ Mouse click handling
- ✅ Keyboard navigation (Tab, Arrow keys, Space, Enter)
- ✅ Focus management with visual indicators
- ✅ State management with Property<T>
- ✅ Real-time updates and reactivity

### Component Capabilities
- **Checkboxes**: Click or keyboard toggle, customizable colors
- **Toggles**: Animated state transitions, keyboard support  
- **Radio Buttons**: Group management, keyboard selection, custom colors
- **Sliders**: Mouse drag, keyboard arrow keys (↑↓←→), Home/End keys
- **Progress Bars**: Determinate progress with percentage, animated loading state

## Building

```bash
cd build
cmake ..
make component_showcase
./component_showcase
```

## Implementation Highlights

All new components follow Flux's design principles:
- **Declarative API** - Component-based UI with designated initializers
- **Reactive State** - Automatic UI updates via Property<T>
- **Keyboard First** - Full keyboard navigation and control
- **Focus Management** - Integrated with Flux's focus system
- **Customizable** - Extensive styling options

## Code Structure

The demo showcases:
- Two-column layout with organized sections
- Proper state management with Property<T>
- Event callbacks (onClick, onChange)
- Lambda-based dynamic values
- Clean component composition

## Component Details

### Checkbox
```cpp
Checkbox {
    .checked = myProperty,
    .label = "Option",
    .checkColor = Colors::blue,
    .onChange = []() { /* callback */ }
}
```

### Toggle
```cpp
Toggle {
    .isOn = enabled,
    .onColor = Colors::green,
    .onChange = []() { /* callback */ }
}
```

### RadioButton
```cpp
RadioButton {
    .selected = isSelected,
    .value = "option1",
    .group = "myGroup",  // Mutual exclusion
    .label = "Option 1",
    .onChange = []() { /* callback */ }
}
```

### Slider
```cpp
Slider {
    .value = currentValue,
    .minValue = 0.0f,
    .maxValue = 100.0f,
    .step = 1.0f,
    .onChange = []() { /* callback */ }
}
```

### ProgressBar
```cpp
// Determinate
ProgressBar {
    .value = 0.65f,  // 0.0 to 1.0
    .mode = ProgressBarMode::Determinate,
    .showLabel = true
}

// Indeterminate
ProgressBar {
    .mode = ProgressBarMode::Indeterminate,
    .fillColor = Colors::green
}
```

### Badge
```cpp
Badge {
    .text = "New",
    .badgeColor = Colors::red,
    .textColor = Colors::white
}
```

### Divider
```cpp
Divider {
    .orientation = DividerOrientation::Horizontal,
    .thickness = 2.0f,
    .color = Colors::gray
}
```

## Next Steps

With these components, Flux now has a solid foundation for building:
- Forms and data entry interfaces
- Settings panels
- Dashboards with real-time data
- Progress tracking interfaces
- Status displays and notifications

The framework is ready for building production-quality desktop applications!

