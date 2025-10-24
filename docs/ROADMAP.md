# Flux Roadmap

This document tracks features that are not yet implemented but are planned for future development.

## Priority 1: Essential Input Components

### TextInput Component
**Status**: Not implemented  
**Description**: Single-line text input field with cursor and selection support

**Required Features:**
- Cursor rendering and positioning
- Text selection (mouse drag, shift+arrows)
- Copy/paste support (requires clipboard protocol)
- Input validation hooks
- Placeholder text
- Character limits
- Password mode (obscured text)

**Dependencies:**
- Wayland text-input protocol
- Clipboard protocol
- Focus system (✅ implemented)
- Keyboard events (✅ implemented)

### TextArea Component
**Status**: Not implemented  
**Description**: Multi-line text input with scrolling

**Required Features:**
- All TextInput features
- Multi-line editing
- Line wrapping (word wrap, character wrap)
- Vertical scrolling
- Line numbers (optional)
- Tab handling
- Auto-indent (optional)

**Dependencies:**
- ScrollView component
- TextInput foundation

## Priority 2: Advanced Interaction

### Mouse Enter/Leave Events
**Status**: Partially implemented (callbacks defined, detection not implemented)  
**Description**: onMouseEnter and onMouseLeave event detection

**Required Features:**
- Track which view mouse is currently over
- Detect when mouse enters/exits view bounds
- Trigger onMouseEnter callback on entry
- Trigger onMouseLeave callback on exit
- Handle nested views correctly (event bubbling)

**Current State:**
- Callbacks defined in FLUX_VIEW_PROPERTIES
- Not yet wired up in renderer/event system

### Double-Click Detection
**Status**: Partially implemented (callback defined, timing not implemented)  
**Description**: onDoubleClick event with timing detection

**Required Features:**
- Track time between clicks
- Detect double-click within threshold (typically 500ms)
- Distinguish from two separate clicks
- Trigger onDoubleClick callback

**Current State:**
- Callback defined in FLUX_VIEW_PROPERTIES
- Timing logic not implemented

### Drag and Drop System
**Status**: Not implemented (callbacks defined)  
**Description**: Full drag and drop support with visual feedback

**Required Features:**
- Drag gesture detection (mouse down + move threshold)
- onDragStart callback when drag begins
- onDrag callback during drag motion
- onDragEnd callback when drag completes
- onDrop callback on drop target
- Visual drag preview/ghost
- Drop target highlighting
- Data transfer between drag source and drop target

**Current State:**
- Callbacks defined in FLUX_VIEW_PROPERTIES
- No implementation yet

## Priority 3: Scrolling & Virtualization

### ScrollView Component
**Status**: Not implemented  
**Description**: Scrollable container with scrollbars

**Required Features:**
- Vertical and horizontal scrolling
- Scrollbar rendering (optional, can be hidden)
- Mouse wheel support
- Touch/trackpad gestures
- Scroll position API
- Programmatic scrolling
- Scroll animation/easing
- Content clipping

**Implementation Notes:**
- Requires clip property support (✅ implemented)
- Needs scroll event handling
- Viewport calculations for visible area

### ListView Component
**Status**: Not implemented  
**Description**: Efficient list with virtualization for large datasets

**Required Features:**
- Virtual scrolling (only render visible items)
- Dynamic item heights
- Item selection (single/multi)
- Keyboard navigation (arrow keys, home/end, page up/down)
- Separator lines (optional)
- Section headers (optional)
- Pull-to-refresh (optional)
- Infinite scroll (optional)

**Dependencies:**
- ScrollView component
- Focus system (✅ implemented)

## Priority 4: Advanced Layout

### FlexBox Layout
**Status**: Partially implemented (basic flex via expansionBias)  
**Description**: Full CSS Flexbox-style layout

**Current State:**
- expansionBias/compressionBias provide basic flex growth/shrink
- AlignItems provides cross-axis alignment
- JustifyContent provides main-axis alignment

**Missing Features:**
- flex-wrap (multi-line flex containers)
- align-content (multi-line alignment)
- order property (reorder children)
- flex-basis (initial size before flex)
- gap property (spacing between items)

### Absolute Positioning
**Status**: Not implemented  
**Description**: Position views at absolute coordinates

**Required Features:**
- Position property (absolute, relative)
- Top, left, right, bottom offsets
- Z-index for layering
- Position relative to parent or viewport

### Overlay/Modal System
**Status**: Not implemented  
**Description**: Floating overlays, modals, tooltips, menus

**Required Features:**
- Z-ordering/layering
- Focus trapping (modals capture focus)
- Click-outside-to-close
- Escape key to close
- Backdrop/dimming
- Position anchoring (tooltip relative to target)
- Animation (fade in/out)

## Priority 5: Wayland Protocol Extensions

### Clipboard Support
**Status**: Not implemented  
**Description**: Copy/paste via Wayland clipboard protocol

**Required Features:**
- wl_data_device_manager protocol
- Text clipboard (MIME type text/plain)
- Copy API
- Paste API
- Cut API
- Clipboard change notifications

**Implementation:**
- New protocol implementation in Platform/
- Window-level clipboard API
- TextInput/TextArea integration

### IME (Input Method Editor) Support
**Status**: Not implemented  
**Description**: International text input via Wayland text-input protocol

**Required Features:**
- zwp_text_input_v3 protocol
- Composition events (preedit text)
- Candidate window positioning
- Commit events
- Language-specific input methods (CJK, etc.)

**Implementation:**
- New protocol implementation in Platform/
- TextInput component integration
- Composition UI rendering

### Touch Input
**Status**: Not implemented  
**Description**: Touch screen support via Wayland touch protocol

**Required Features:**
- wl_touch interface
- Multi-touch support
- Touch events (down, up, move)
- Gesture recognition (tap, swipe, pinch, rotate)
- Touch event callbacks

## Priority 6: Advanced Components

### Checkbox Component
**Status**: Not implemented  
**Required Features:**
- Check/uncheck state
- Visual indicator (checkmark)
- Label support
- Keyboard toggle (Space)
- onChange callback

### Radio Button Component
**Status**: Not implemented  
**Required Features:**
- Radio group (mutual exclusion)
- Selection state
- Visual indicator (circle/dot)
- Keyboard navigation (arrow keys)
- onChange callback

### Toggle/Switch Component
**Status**: Not implemented  
**Required Features:**
- On/off state
- Animated transition
- Visual switch design
- Keyboard toggle (Space)
- onChange callback

### Dropdown/Select Component
**Status**: Not implemented  
**Required Features:**
- Dropdown menu
- Option list
- Selection state
- Keyboard navigation
- Search/filter options
- Multi-select mode (optional)
- onChange callback

**Dependencies:**
- Overlay system
- ScrollView (for long lists)

### ProgressBar Component
**Status**: Not implemented  
**Required Features:**
- Determinate mode (0-100%)
- Indeterminate mode (loading animation)
- Custom styling
- Label/text display
- Circular variant (optional)

### TabView Component
**Status**: Not implemented  
**Required Features:**
- Tab headers
- Tab content panels
- Tab selection state
- Keyboard navigation (left/right arrows)
- Dynamic tab add/remove
- Close button (optional)

### DatePicker Component
**Status**: Not implemented  
**Required Features:**
- Calendar view
- Date selection
- Month/year navigation
- Date range selection (optional)
- Custom date formatting
- Min/max date constraints

**Dependencies:**
- Overlay system
- Grid layout (✅ implemented)

## Priority 7: Navigation & Routing

### Navigation System
**Status**: Not implemented  
**Description**: Multi-view navigation with history

**Required Features:**
- Push/pop view stack
- Navigation history
- Back button support
- Route parameters
- Transition animations
- Deep linking

### Tab Navigation
**Status**: Not implemented  
**Description**: Tab-based navigation container

**Required Features:**
- Tab bar
- View switching
- Persistent state per tab
- Badge counts
- Icons + labels

## Priority 8: Form Handling

### Form Validation Framework
**Status**: Not implemented  
**Description**: Declarative form validation

**Required Features:**
- Validation rules (required, min/max, pattern, custom)
- Error messages
- Field-level validation
- Form-level validation
- Async validation
- Error display UI

### Form State Management
**Status**: Not implemented  
**Description**: Unified form state handling

**Required Features:**
- Form submission
- Dirty state tracking
- Pristine/touched state
- Reset form
- Initial values
- Form-level onChange

## Priority 9: Accessibility

### AT-SPI Integration
**Status**: Not implemented  
**Description**: Linux accessibility support via AT-SPI

**Required Features:**
- Expose UI tree to assistive technologies
- Screen reader support
- Keyboard navigation descriptions
- ARIA-like roles and properties
- Focus announcements

### Accessibility Properties
**Status**: Not implemented  
**Required Features:**
- Role property (button, heading, etc.)
- Label property (accessible name)
- Description property
- Hidden from accessibility tree
- Live regions (announcements)

## Priority 10: Animation

### Animation System
**Status**: Not implemented  
**Description**: Declarative animations

**Required Features:**
- Property animations (opacity, position, size, etc.)
- Easing functions
- Duration control
- Animation composition (sequence, parallel)
- Spring physics (optional)
- Gesture-driven animations

### Transition System
**Status**: Not implemented  
**Description**: Automatic transitions on property changes

**Required Features:**
- Implicit transitions
- Transition curves
- Staggered transitions
- Conditional transitions

## Priority 11: Advanced Graphics

### Custom Shaders
**Status**: Not implemented  
**Description**: OpenGL shader support for custom effects

**Required Features:**
- Shader compilation
- Uniform passing
- Texture support
- Effect composition

### Blur Effects
**Status**: Not implemented  
**Required Features:**
- Gaussian blur
- Background blur (frosted glass)
- Blur radius property
- Performance optimization

### Shadows
**Status**: Partially implemented (basic shadow support exists)  
**Missing Features:**
- Multiple shadows
- Inset shadows
- Shadow spread
- Shadow quality settings

## Priority 12: Developer Tools

### UI Inspector
**Status**: Not implemented  
**Description**: Runtime UI debugging tool

**Required Features:**
- View hierarchy visualization
- Property inspection
- Layout bounds visualization
- Event logging
- Performance metrics

### Hot Reload
**Status**: Not implemented  
**Description**: Live code reloading during development

**Required Features:**
- Watch source files
- Recompile on change
- Reload UI without restart
- State preservation

### Performance Profiler
**Status**: Not implemented  
**Description**: Frame timing and performance analysis

**Required Features:**
- Frame time tracking
- Layout performance
- Render performance
- Property evaluation time
- Memory usage tracking

## Priority 13: Testing

### Unit Testing Framework
**Status**: Not implemented  
**Description**: Testing utilities for Flux components

**Required Features:**
- Component testing
- Event simulation
- Property verification
- Layout testing
- Snapshot testing

### Integration Testing
**Status**: Not implemented  
**Description**: Full application testing

**Required Features:**
- Window automation
- Input simulation
- Screenshot comparison
- Accessibility testing

## Priority 14: Multi-Platform Support

**Note**: Flux is currently Linux/Wayland-only by design. Multi-platform support is not a goal at this time, but this section documents what would be required if that changes.

### Windows Support
**Status**: Not planned  
**Required**: Win32 window backend or SDL2 abstraction

### macOS Support
**Status**: Not planned  
**Required**: Cocoa window backend or SDL2 abstraction

### Web (WASM) Support
**Status**: Not planned  
**Required**: Canvas/WebGL backend, Emscripten build

## Feature Request Process

To request a new feature:
1. Open an issue on the project repository
2. Describe the feature and use cases
3. Discuss implementation approach
4. Feature may be added to this roadmap based on alignment with project goals

## Contributing

Interested in implementing one of these features? Great! Please:
1. Check the Dependencies section to ensure prerequisites are met
2. Open an issue to discuss your implementation approach
3. Follow the existing code patterns and architecture
4. Write examples demonstrating the feature
5. Update documentation

See `docs/DEVELOPMENT.md` for development guidelines.

