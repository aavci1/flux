# Login Manager Example

A macOS Big Sur-inspired login manager built with Flux UI framework, featuring the WhiteSur theme.

## Features

- **WhiteSur Background**: Uses the beautiful gradient background from the WhiteSur KDE theme
- **Real-time Clock**: Displays current time and date at the top
- **User Avatar**: Circular avatar with user initial
- **Password Field**: Translucent password input with submit button
- **System Actions**: Four action buttons for emergency, restart, shutdown, and user switching

## Design Elements

### Background
- Full-screen gradient background image (`white-sur-background.jpg`)
- Smooth color transitions from purple/magenta to blue/white

### UI Components
- **Time Display**: Large, bold white text showing current time
- **User Avatar**: Circular avatar with blue gradient background and white border
- **Password Field**: Translucent white background with rounded corners
- **Submit Button**: Circular white button with arrow icon
- **Action Buttons**: Four circular buttons with white borders and system icons

### Layout
- **Top Section**: Time and date display
- **Center Section**: User avatar and login form
- **Bottom Section**: System action buttons

## Custom Components

This example demonstrates several custom Flux components:

- `BackgroundImage`: Renders background images using NanoVG
- `UserAvatar`: Circular avatar with gradient background
- `PasswordField`: Custom password input with transparency
- `SubmitButton`: Circular button with arrow icon
- `ActionButton`: System action buttons with custom icons

## Building and Running

```bash
mkdir build && cd build
cmake ..
make login_manager
./login_manager
```

## Assets Required

- `white-sur-background.jpg`: Background image (should be in the project root)
- SVG icons for action buttons (optional, currently using simple drawn icons)

## Technical Details

- Uses Flux's custom rendering capabilities
- Demonstrates image loading with NanoVG
- Shows real-time state updates with threading
- Implements custom view components with `FLUX_VIEW_PROPERTIES`
- Uses transparency and gradients for modern UI effects
