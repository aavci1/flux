#include <Flux/Platform/GLFWWindow.hpp>
#include <Flux/Platform/NanoVGRenderer.hpp>
#include <Flux/Graphics/NanoVGRenderContext.hpp>
#include <Flux/Core/Window.hpp>
#include <iostream>
#include <stdexcept>

namespace flux {

// Static callback function for GLFW resize events
void GLFWWindow::resizeCallback(GLFWwindow* window, int width, int height) {
    GLFWWindow* glfwWin = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
    if (glfwWin) {
        Window* fluxWindow = glfwWin->fluxWindow();
        if (fluxWindow) {
            fluxWindow->handleResize(Size{static_cast<float>(width), static_cast<float>(height)});
        }
    }
}

// Static callback function for GLFW window close events
void GLFWWindow::closeCallback(GLFWwindow* window) {
    GLFWWindow* glfwWin = static_cast<GLFWWindow*>(glfwGetWindowUserPointer(window));
    if (glfwWin) {
        Window* fluxWindow = glfwWin->fluxWindow();
        if (fluxWindow) {
            // Set the window to close
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
}

int GLFWWindow::windowCount_ = 0;

GLFWWindow::GLFWWindow(const std::string& title, const Size& size, bool resizable, bool fullscreen)
    : window_(nullptr), currentSize_(size), isFullscreen_(fullscreen), dpiScaleX_(1.0f), dpiScaleY_(1.0f), fluxWindow_(nullptr) {

    // Initialize GLFW if this is the first window
    if (windowCount_ == 0) {
        if (!glfwInit()) {
            throw std::runtime_error("Failed to initialize GLFW");
        }
        std::cout << "[GLFWWindow] GLFW initialized\n";
    }
    windowCount_++;

    // Configure GLFW for OpenGL 2.1 (better macOS compatibility)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_FALSE);
    glfwWindowHint(GLFW_DOUBLEBUFFER, GL_TRUE);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    // Create window flags
    int windowFlags = GLFW_VISIBLE;
    if (resizable) {
        windowFlags |= GLFW_RESIZABLE;
    }

    // Create GLFW window
    window_ = glfwCreateWindow(
        static_cast<int>(size.width),
        static_cast<int>(size.height),
        title.c_str(),
        fullscreen ? glfwGetPrimaryMonitor() : nullptr,
        nullptr
    );

    if (!window_) {
        throw std::runtime_error("Failed to create GLFW window");
    }

    // Make context current
    glfwMakeContextCurrent(window_);

    // Enable VSync
    glfwSwapInterval(1);

    // Set up resize callback
    glfwSetWindowSizeCallback(window_, resizeCallback);

    // Set up close callback
    glfwSetWindowCloseCallback(window_, closeCallback);

    // Store pointer to this GLFWWindow instance in the GLFW window user data
    glfwSetWindowUserPointer(window_, this);

    // Detect DPI scale
    int framebufferWidth, framebufferHeight;
    int windowWidth, windowHeight;
    glfwGetFramebufferSize(window_, &framebufferWidth, &framebufferHeight);
    glfwGetWindowSize(window_, &windowWidth, &windowHeight);

    dpiScaleX_ = static_cast<float>(framebufferWidth) / static_cast<float>(windowWidth);
    dpiScaleY_ = static_cast<float>(framebufferHeight) / static_cast<float>(windowHeight);

    std::cout << "[GLFWWindow] DPI scale: " << dpiScaleX_ << "x" << dpiScaleY_ << "\n";

    // Create renderer with window dimensions (logical pixels), not framebuffer dimensions
    renderer_ = std::make_unique<NanoVGRenderer>();
    if (!renderer_->initialize(windowWidth, windowHeight, dpiScaleX_, dpiScaleY_)) {
        throw std::runtime_error("Failed to initialize NanoVG renderer");
    }

    std::cout << "[GLFWWindow] Created window \"" << title
              << "\" size: " << size.width << "x" << size.height << "\n";
}

GLFWWindow::~GLFWWindow() {
    std::cout << "[GLFWWindow] Destroying window\n";

    renderer_.reset();

    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }

    // Terminate GLFW if this was the last window
    windowCount_--;
    if (windowCount_ == 0) {
        glfwTerminate();
        std::cout << "[GLFWWindow] GLFW terminated\n";
    }
}

void GLFWWindow::resize(const Size& newSize) {
    currentSize_ = newSize;

    // Update the actual GLFW window size
    glfwSetWindowSize(window_, static_cast<int>(newSize.width), static_cast<int>(newSize.height));

    // Force GLFW to process the resize event
    glfwPollEvents();

    // Get the ACTUAL current framebuffer size after resize
    int framebufferWidth, framebufferHeight;
    int windowWidth, windowHeight;
    glfwGetFramebufferSize(window_, &framebufferWidth, &framebufferHeight);
    glfwGetWindowSize(window_, &windowWidth, &windowHeight);

    // Update DPI scaling with actual values
    dpiScaleX_ = static_cast<float>(framebufferWidth) / static_cast<float>(windowWidth);
    dpiScaleY_ = static_cast<float>(framebufferHeight) / static_cast<float>(windowHeight);

    if (renderer_) {
        // Cast to NanoVGRenderer to access updateDPIScale method
        auto* nanoVGRenderer = static_cast<NanoVGRenderer*>(renderer_.get());
        // Update DPI scaling first
        nanoVGRenderer->updateDPIScale(dpiScaleX_, dpiScaleY_);
        // Pass logical dimensions, not framebuffer dimensions
        renderer_->resize(static_cast<int>(newSize.width), static_cast<int>(newSize.height));

        // Also update the render context DPI scale
        auto* renderContext = static_cast<NanoVGRenderContext*>(renderer_->renderContext());
        renderContext->updateDPIScale(dpiScaleX_, dpiScaleY_);
    }

    std::cout << "[GLFWWindow] Resized to " << newSize.width << "x" << newSize.height
              << " (Framebuffer: " << framebufferWidth << "x" << framebufferHeight
              << ", DPI scale: " << dpiScaleX_ << "x" << dpiScaleY_ << ")\n";
}

void GLFWWindow::setFullscreen(bool fullscreen) {
    isFullscreen_ = fullscreen;

    if (fullscreen) {
        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        glfwSetWindowMonitor(window_, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
        currentSize_ = Size{static_cast<float>(mode->width), static_cast<float>(mode->height)};
    } else {
        glfwSetWindowMonitor(window_, nullptr, 100, 100,
                           static_cast<int>(currentSize_.width),
                           static_cast<int>(currentSize_.height), 0);
    }

    std::cout << "[GLFWWindow] Fullscreen: " << (fullscreen ? "ON" : "OFF") << "\n";
}

void GLFWWindow::setTitle(const std::string& title) {
    glfwSetWindowTitle(window_, title.c_str());
    std::cout << "[GLFWWindow] Title changed to \"" << title << "\"\n";
}

unsigned int GLFWWindow::windowID() const {
    return static_cast<unsigned int>(reinterpret_cast<uintptr_t>(window_));
}

RenderContext* GLFWWindow::renderContext() {
    return renderer_ ? renderer_->renderContext() : nullptr;
}

void GLFWWindow::swapBuffers() {
    if (window_) {
        glfwSwapBuffers(window_);
    }
}

} // namespace flux
