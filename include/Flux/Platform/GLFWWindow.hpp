#include <Flux/Platform/PlatformWindow.hpp>
#include <Flux/Platform/PlatformRenderer.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>

namespace flux {

// Forward declaration
class Window;

class GLFWWindow : public PlatformWindow {
private:
    GLFWwindow* window_;
    std::unique_ptr<PlatformRenderer> renderer_;
    Size currentSize_;
    bool isFullscreen_;
    bool shouldClose_;
    float dpiScaleX_;
    float dpiScaleY_;
    Window* fluxWindow_; // Reference to the Flux Window instance

    // Static initialization counter
    static int windowCount_;

    // Static callback function for GLFW events
    static void resizeCallback(GLFWwindow* window, int width, int height);
    static void closeCallback(GLFWwindow* window);

public:
    GLFWWindow(const std::string& title, const Size& size, bool resizable, bool fullscreen);
    ~GLFWWindow() override;

    // PlatformWindow interface
    void resize(const Size& newSize) override;
    void setFullscreen(bool fullscreen) override;
    void setTitle(const std::string& title) override;
    unsigned int windowID() const override;

    RenderContext* renderContext() override;

    void swapBuffers() override;

    float dpiScaleX() const override { return dpiScaleX_; }
    float dpiScaleY() const override { return dpiScaleY_; }

    Size currentSize() const override { return currentSize_; }
    bool isFullscreen() const override { return isFullscreen_; }

    void processEvents() override;
    bool shouldClose() const override;
    void setFluxWindow(Window* window) override { fluxWindow_ = window; }

    // Method to get the Flux Window instance
    Window* fluxWindow() const { return fluxWindow_; }

    // GLFW-specific accessor
    GLFWwindow* glfwWindow() const { return window_; }
};

} // namespace flux
