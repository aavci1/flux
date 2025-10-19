#include <Flux/Core/Window.hpp>
#include <Flux/Core/Application.hpp>
#include <Flux/Graphics/Renderer.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <Flux/Platform/PlatformWindow.hpp> // Include PlatformWindow header

#if defined(__linux__) && !defined(__ANDROID__)
    #include <Flux/Platform/WaylandWindow.hpp>
#else
    #include <Flux/Platform/GLFWWindow.hpp>
#endif

#include <iostream>
#include <cstdlib>

namespace flux {

// Forward declaration of factory functions
// Platform window is created by GLFWWindow

Window::Window(const WindowConfig& config)
    : config_(config), currentSize_(config.size) {

    // Determine which backend to use
    WindowBackend backendToUse = config.backend;

    if (backendToUse == WindowBackend::Auto) {
        // Auto-select: use GLFW + NanoVG
        backendToUse = WindowBackend::Default;
    }

    activeBackend_ = backendToUse;

    // Initialize the appropriate backend
    if (backendToUse == WindowBackend::Default) {
#if defined(__linux__) && !defined(__ANDROID__)
        // Create Wayland window on Linux
        platformWindow_ = std::make_unique<WaylandWindow>(
            config.title,
            config.size,
            config.resizable,
            config.fullscreen
        );

        // Set the Flux Window reference in the WaylandWindow for resize callbacks
        static_cast<WaylandWindow*>(platformWindow_.get())->setFluxWindow(this);

        renderContext_.reset(); // PlatformWindow owns the render context
        renderer_ = std::make_unique<ImmediateModeRenderer>(platformWindow_->renderContext());
        std::cout << "[WINDOW] Using Wayland + NanoVG backend\n";
#else
        // Create GLFW window on macOS and Windows
        platformWindow_ = std::make_unique<GLFWWindow>(
            config.title,
            config.size,
            config.resizable,
            config.fullscreen
        );

        // Set the Flux Window reference in the GLFWWindow for resize callbacks
        static_cast<GLFWWindow*>(platformWindow_.get())->setFluxWindow(this);

        renderContext_.reset(); // PlatformWindow owns the render context
        renderer_ = std::make_unique<ImmediateModeRenderer>(platformWindow_->renderContext());
        std::cout << "[WINDOW] Using GLFW + NanoVG backend\n";
#endif
    } else {
        throw std::runtime_error("Unsupported backend. Please use WindowBackend::Default or WindowBackend::Auto.");
    }

    // Register with the application
    Application::instance().registerWindow(this);

    std::cout << "[WINDOW] Created window \"" << config.title
              << "\" size: " << config.size.width << "x" << config.size.height << "\n";
}

Window::~Window() {
    // Unregister from the application
    Application::instance().unregisterWindow(this);

    std::cout << "[WINDOW] Destroyed window \"" << config_.title << "\"\n";
}

void Window::setRootView(View component) {
    rootView_ = std::move(component);
    renderer_->setRootView(rootView_);
    // Request initial render
    Application::instance().requestRedraw();
}

void Window::render() {
    if (!renderer_) return;

    // Render the root view
    // The renderer will handle scaling based on DPI
    Rect windowBounds = {0, 0, currentSize_.width, currentSize_.height};
    renderer_->renderFrame(windowBounds);

    platformWindow_->swapBuffers();
}

void Window::resize(const Size& newSize) {
    currentSize_ = newSize;

    if (platformWindow_) {
        platformWindow_->resize(newSize);
    }

    std::cout << "[WINDOW] Resized to " << newSize.width << "x" << newSize.height << "\n";

    // Request a redraw with the new size
    Application::instance().requestRedraw();
}

void Window::setFullscreen(bool fullscreen) {
    config_.fullscreen = fullscreen;

    if (platformWindow_) {
        platformWindow_->setFullscreen(fullscreen);
    }

    std::cout << "[WINDOW] Fullscreen: " << (fullscreen ? "ON" : "OFF") << "\n";
}

void Window::setTitle(const std::string& title) {
    config_.title = title;

    if (platformWindow_) {
        platformWindow_->setTitle(title);
    }

    std::cout << "[WINDOW] Title changed to \"" << title << "\"\n";
}

unsigned int Window::windowID() const {
    if (platformWindow_) {
        return platformWindow_->windowID();
    }
    return 0; // Invalid ID
}

void Window::handleMouseMove(float x, float y) {
    std::cout << "[INPUT] Mouse moved to (" << x << ", " << y << ")\n";
    // In real implementation, would propagate to views
}

void Window::handleMouseDown(int button, float x, float y) {
    std::cout << "[INPUT] Mouse button " << button << " down at (" << x << ", " << y << ")\n";
}

void Window::handleMouseUp(int button, float x, float y) {
    std::cout << "[INPUT] Mouse button " << button << " up at (" << x << ", " << y << ")\n";
}

void Window::handleKeyDown(int key) {
    std::cout << "[INPUT] Key down: " << key << "\n";
}

void Window::handleKeyUp(int key) {
    std::cout << "[INPUT] Key up: " << key << "\n";
}

void Window::handleTextInput(const std::string& text) {
    std::cout << "[INPUT] Text input: " << text << "\n";
}

void Window::handleResize(const Size& newSize) {
    std::cout << "[WINDOW] Internal handleResize called with " << newSize.width << "x" << newSize.height << "\n";

    // Update size from the resize event
    resize(newSize);

    // Resize the platform renderer to match new window size
    if (platformWindow_) {
        platformWindow_->renderContext()->resize(
            static_cast<int>(newSize.width),
            static_cast<int>(newSize.height)
        );
    }

    // Render immediately with this exact size
    render();
}

} // namespace flux
