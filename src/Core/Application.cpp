#include <Flux/Core/Application.hpp>
#include <Flux/Core/Window.hpp>
#include <Flux/Core/WindowBackend.hpp>
#include <chrono>
#include <thread>
#include <algorithm>

#include <Flux/Platform/PlatformWindow.hpp>
#include <Flux/Platform/GLFWWindow.hpp>
#include <GLFW/glfw3.h> // Include GLFW header for event polling

namespace flux {

Application* Application::instance_ = nullptr;

// Global redraw request function (called by State changes)
void requestApplicationRedraw() {
    if (Application::instance_) {
        Application::instance_->requestRedraw();
    }
}

Application::Application(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    if (instance_) {
        throw std::runtime_error("Application already initialized");
    }
    instance_ = this;
}

Application::~Application() {
    instance_ = nullptr;
}

int Application::exec() {
    while (running) {
        // Process system events (mouse, keyboard, etc.)
        processEvents();

        // Render all windows if a redraw is requested
        if (needsRedraw.load(std::memory_order_relaxed)) {
            for (auto* window : windows_) {
                window->render();
            }
            needsRedraw.store(false, std::memory_order_relaxed);
        }

        waitForNextFrame();
    }
    return 0;
}

void Application::registerWindow(Window* window) {
    windows_.push_back(window);
}

void Application::unregisterWindow(Window* window) {
    windows_.erase(std::remove(windows_.begin(), windows_.end(), window), windows_.end());
}

void Application::processEvents() {
    // Process GLFW events
    glfwPollEvents();

    // Check if any window should close
    for (auto* window : windows_) {
        if (window->backend() == WindowBackend::Default) {
            auto* glfwWin = dynamic_cast<GLFWWindow*>(window->platformWindow());
            if (glfwWin && glfwWindowShouldClose(glfwWin->glfwWindow())) {
                quit();
                break;
            }
        }
    }
}

void Application::waitForNextFrame() {
    // Target 60 FPS = 16.67ms per frame
    static auto lastFrame = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> diff = now - lastFrame;

    if (diff.count() < 16.67) {
        std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(16.67 - diff.count()));
    }
    lastFrame = std::chrono::high_resolution_clock::now();
}

} // namespace flux