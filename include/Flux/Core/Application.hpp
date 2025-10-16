#pragma once

#include <atomic>
#include <vector>
#include <memory>
#include <chrono>
#include <thread>

namespace flux {

class Window;

// Forward declaration
void requestApplicationRedraw();

class Application {
private:
    std::atomic<bool> needsRedraw{false};
    bool running{true};
    std::vector<Window*> windows_;

    static Application* instance_;

    friend void requestApplicationRedraw();

public:
    Application(int argc, char* argv[]);
    ~Application();

    // Delete copy and move operations
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    // Called by State<T> on any change - thread-safe
    void requestRedraw() {
        needsRedraw.store(true, std::memory_order_relaxed);
    }

    // Main event loop with automatic frame batching
    int exec();

    void quit() {
        running = false;
    }

    static Application& instance() {
        if (!instance_) {
            throw std::runtime_error("Application not initialized");
        }
        return *instance_;
    }

    // Internal: Called by Window constructor/destructor
    void registerWindow(Window* window);
    void unregisterWindow(Window* window);

    // Internal: Access windows for event handling
    const std::vector<Window*>& windows() const { return windows_; }
    std::vector<Window*>& windows() { return windows_; }

private:
    void processEvents();
    void waitForNextFrame();
};

} // namespace flux
