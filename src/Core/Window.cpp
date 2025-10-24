#include <Flux/Core/Window.hpp>
#include <Flux/Core/Application.hpp>
#include <Flux/Graphics/Renderer.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <Flux/Platform/PlatformWindow.hpp>
#include <Flux/Platform/WaylandWindow.hpp>

#include <iostream>
#include <cstdlib>

namespace flux {

Window::Window(const WindowConfig& config)
    : config_(config), currentSize_(config.size) {

    // Create Wayland window (Linux/Wayland only)
    platformWindow_ = std::make_unique<WaylandWindow>(
        config.title,
        config.size,
        config.resizable,
        config.fullscreen
    );
    
    std::cout << "[WINDOW] Using Wayland + NanoVG backend\n";

    // Set the Flux Window reference for resize callbacks
    platformWindow_->setFluxWindow(this);

    renderContext_.reset(); // PlatformWindow owns the render context
    renderer_ = std::make_unique<ImmediateModeRenderer>(platformWindow_->renderContext());
    
    // Set window reference in renderer for cursor management
    auto* immediateRenderer = static_cast<ImmediateModeRenderer*>(renderer_.get());
    immediateRenderer->setWindow(this);

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
    Event event;
    event.type = Event::MouseMove;
    event.mouseMove.x = x;
    event.mouseMove.y = y;
    dispatchEvent(event);
}

void Window::handleMouseDown(int button, float x, float y) {
    Event event;
    event.type = Event::MouseDown;
    event.mouseButton.x = x;
    event.mouseButton.y = y;
    event.mouseButton.button = button;
    dispatchEvent(event);
}

void Window::handleMouseUp(int button, float x, float y) {
    Event event;
    event.type = Event::MouseUp;
    event.mouseButton.x = x;
    event.mouseButton.y = y;
    event.mouseButton.button = button;
    dispatchEvent(event);
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

void Window::dispatchEvent(const Event& event) {
    // Create window bounds for hit testing
    Rect windowBounds = {0, 0, currentSize_.width, currentSize_.height};

    // Dispatch event to renderer for hit testing and view targeting
    if (renderer_) {
        auto* immediateRenderer = static_cast<ImmediateModeRenderer*>(renderer_.get());
        immediateRenderer->handleEvent(event, windowBounds);
    }
}

void Window::setCursor(CursorType cursor) {
    if (platformWindow_) {
        platformWindow_->setCursor(cursor);
    }
}

CursorType Window::currentCursor() const {
    if (platformWindow_) {
        return platformWindow_->currentCursor();
    }
    return CursorType::Default;
}

} // namespace flux
