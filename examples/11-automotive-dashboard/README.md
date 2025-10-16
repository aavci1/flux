# Automotive Dashboard Demo

A modern automotive dashboard interface built with the Flux UI framework, featuring a three-column layout with media controls, navigation, and vehicle status information.

## Features

### Top Bar
- Current time and date display
- User profile with name
- Battery level indicator with visual representation

### Left Column - Media & Apps
- **Music Player**: Album art, song title, artist, progress bar, and playback controls
- **App Grid**: 2x3 grid of application shortcuts (Phone, Music, Bluetooth, Savings, Wind, Maps)

### Middle Column - Navigation
- **Turn-by-turn Instructions**: Next maneuver with distance and direction
- **Interactive Map**: Street grid with route visualization and current position marker
- **Destination Info**: Destination name, estimated time, and distance
- **Navigation Controls**: Audio, settings, and compass controls

### Right Column - Vehicle & Climate
- **Vehicle Status**: Model name, status badge, gear selector (P/R/N/D), and driving modes
- **Climate Control**: Outdoor temperature, location, temperature presets, and climate icons

## Design Elements

- **Color Scheme**: Clean white backgrounds with blue and green accents
- **Typography**: Bold headings with secondary text in muted colors
- **Icons**: Unicode symbols for cross-platform compatibility
- **Layout**: Responsive three-column design with proper spacing
- **Visual Hierarchy**: Clear information organization with appropriate sizing

## Building and Running

```bash
# Build the project
mkdir build && cd build
cmake ..
make automotive_dashboard

# Run the demo
./automotive_dashboard
```

## Customization

The dashboard components are designed to be easily customizable:

- **Colors**: Modify the color scheme by changing the hex color values
- **Content**: Update text, icons, and data through component properties
- **Layout**: Adjust spacing, sizing, and positioning through layout properties
- **Functionality**: Add event handlers and state management for interactive features

## Component Architecture

Each dashboard section is implemented as a custom Flux view component:

- `TopBar`: Header with time, user info, and battery
- `MusicPlayer`: Media playback interface
- `AppGrid`: Application launcher grid
- `NavigationWidget`: Maps and navigation interface
- `VehicleStatus`: Car information and controls
- `ClimateControl`: Temperature and climate settings

This modular approach makes it easy to extend the dashboard with additional features or modify existing components.
