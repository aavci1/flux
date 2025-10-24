#include <Flux/Core/Application.hpp>
#include <Flux/Core/Window.hpp>
#include <Flux/Platform/PlatformWindow.hpp>
#include <chrono>
#include <thread>
#include <algorithm>
#include <iostream>

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
    window->addObserver(this);
}

void Application::unregisterWindow(Window* window) {
    window->removeObserver(this);
    windows_.erase(std::remove(windows_.begin(), windows_.end(), window), windows_.end());
}

void Application::onRedrawRequested(Window* window) {
    (void)window;
    needsRedraw.store(true, std::memory_order_relaxed);
}

void Application::onWindowClosing(Window* window) {
    (void)window;
    // Could handle cleanup here if needed
}

void Application::processEvents() {
    // Process platform-specific events for all windows
    for (auto* window : windows_) {
        if (auto* platformWindow = static_cast<PlatformWindow*>(window->platformWindow())) {
            platformWindow->processEvents();
            
            // Check if window should close
            if (platformWindow->shouldClose()) {
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