#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Core/View.hpp>
#include <string>
#include <memory>

namespace flux {

// Forward declarations
class WindowEventObserver;
class KeyboardInputHandler;
class MouseInputHandler;
class FocusState;
class ShortcutManager;
class PlatformWindowFactory;
struct LayoutNode;

/**
 * @brief Window configuration
 */
struct WindowConfig {
    Size size = {1280, 720};
    std::string title = "Flux Application";
    bool fullscreen = false;
    bool resizable = true;
};

/**
 * @brief Main window class using pImpl idiom
 * 
 * Provides a clean public interface while hiding all implementation details.
 * Manages window lifecycle, rendering, and coordinates input subsystems.
 */
class Window {
public:
    /**
     * @brief Create a window with given configuration
     */
    explicit Window(const WindowConfig& config);
    
    /**
     * @brief Create a window with custom platform factory
     */
    Window(const WindowConfig& config, PlatformWindowFactory* factory);
    
    /**
     * @brief Destructor
     */
    ~Window();

    // Window management
    
    /**
     * @brief Set the root view to display
     */
    void setRootView(View component);
    
    /**
     * @brief Render the current frame
     */
    void render();
    
    /**
     * @brief Resize the window
     */
    void resize(const Size& newSize);
    
    /**
     * @brief Set fullscreen mode
     */
    void setFullscreen(bool fullscreen);
    
    /**
     * @brief Set window title
     */
    void setTitle(const std::string& title);
    
    /**
     * @brief Get window ID
     */
    unsigned int windowID() const;
    
    /**
     * @brief Get current window size
     */
    Size getSize() const;

    // Event handling (called by platform layer)
    
    /**
     * @brief Handle mouse movement
     */
    void handleMouseMove(float x, float y);
    
    /**
     * @brief Handle mouse button down
     */
    void handleMouseDown(int button, float x, float y);
    
    /**
     * @brief Handle mouse button up
     */
    void handleMouseUp(int button, float x, float y);
    
    /**
     * @brief Handle mouse scroll/wheel events
     */
    void handleMouseScroll(float x, float y, float deltaX, float deltaY);
    
    /**
     * @brief Handle key down
     */
    void handleKeyDown(int key);
    
    /**
     * @brief Handle key up
     */
    void handleKeyUp(int key);
    
    /**
     * @brief Handle text input
     */
    void handleTextInput(const std::string& text);
    
    /**
     * @brief Handle window resize from platform
     */
    void handleResize(const Size& newSize);

    // Cursor management
    
    /**
     * @brief Set cursor type
     */
    void setCursor(CursorType cursor);
    
    /**
     * @brief Get current cursor type
     */
    CursorType currentCursor() const;

    // Observer pattern
    
    /**
     * @brief Add an observer for window events
     */
    void addObserver(WindowEventObserver* observer);
    
    /**
     * @brief Remove an observer
     */
    void removeObserver(WindowEventObserver* observer);
    
    /**
     * @brief Notify observers that redraw is requested
     */
    void requestRedraw();

    // Subsystem access
    
    /**
     * @brief Access keyboard input subsystem
     */
    KeyboardInputHandler& keyboard();
    
    /**
     * @brief Access mouse input subsystem
     */
    MouseInputHandler& mouse();
    
    /**
     * @brief Access focus management subsystem
     */
    FocusState& focus();
    
    /**
     * @brief Access shortcut manager subsystem
     */
    ShortcutManager& shortcuts();

    // Internal (used by subsystems)
    
    /**
     * @brief Get platform window (for platform-specific operations)
     * @internal
     */
    void* platformWindow();
    
    /**
     * @brief Process pending keyboard events (called during render)
     * @internal
     */
    void processPendingEvents(LayoutNode& layoutTree);

private:
    // pImpl idiom - hide all implementation details
    struct WindowImpl;
    std::unique_ptr<WindowImpl> impl_;
    
    // Non-copyable
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
};

} // namespace flux
