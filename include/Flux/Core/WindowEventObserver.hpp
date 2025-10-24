#pragma once

namespace flux {

// Forward declaration
class Window;

/**
 * @brief Observer interface for window events
 * 
 * Implements Observer pattern to decouple Window from Application
 * and allow multiple observers for window lifecycle events.
 */
class WindowEventObserver {
public:
    virtual ~WindowEventObserver() = default;
    
    /**
     * @brief Called when window requests a redraw
     */
    virtual void onRedrawRequested(Window* window) = 0;
    
    /**
     * @brief Called when window is about to close
     */
    virtual void onWindowClosing(Window* window) = 0;
    
    /**
     * @brief Called when window is resized
     */
    virtual void onWindowResized(Window* window, float width, float height) {}
};

} // namespace flux

