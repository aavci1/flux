#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Core/View.hpp>
#include <Flux/Graphics/Renderer.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <string>
#include <memory>

namespace flux {

class RenderContext;
class PlatformWindow; // Forward declaration

struct WindowConfig {
    Size size = {1280, 720};
    std::string title = "Flux Application";
    bool fullscreen = false;
    bool resizable = true;
};

class Window {
private:
    std::unique_ptr<ImmediateModeRenderer> renderer_;
    std::unique_ptr<RenderContext> renderContext_; // Owned by PlatformWindow now
    View rootView_;
    WindowConfig config_;
    Size currentSize_;

    std::unique_ptr<PlatformWindow> platformWindow_; // Wayland window implementation

public:
    explicit Window(const WindowConfig& config);
    ~Window();

    void setRootView(View component);
    void render();
    void resize(const Size& newSize);
    void setFullscreen(bool fullscreen);
    void setTitle(const std::string& title);
    unsigned int windowID() const; // Returns the platform-specific window ID

    // Event handlers
    void handleMouseMove(float x, float y);
    void handleMouseDown(int button, float x, float y);
    void handleMouseUp(int button, float x, float y);
    void handleKeyDown(int key);
    void handleKeyUp(int key);
    void handleTextInput(const std::string& text);
    void handleResize(const Size& newSize);

    // Event dispatch to renderer
    void dispatchEvent(const Event& event);

    // Cursor management
    void setCursor(CursorType cursor);
    CursorType currentCursor() const;

    // Platform-specific accessors (for Application event handling)
    PlatformWindow* platformWindow() { return platformWindow_.get(); }
};

} // namespace flux
