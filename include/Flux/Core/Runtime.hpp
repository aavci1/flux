#pragma once

#include <Flux/Core/WindowEventObserver.hpp>
#include <atomic>
#include <string>
#include <vector>
#include <memory>

namespace flux {

class Window;
struct WindowConfig;

class Runtime : public WindowEventObserver {
public:
    explicit Runtime(int argc = 0, char** argv = nullptr);
    ~Runtime();

    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    Runtime(Runtime&&) = delete;
    Runtime& operator=(Runtime&&) = delete;

    Window& createWindow(const WindowConfig& config);

    int run();
    int exec() { return run(); }
    void quit() { running_ = false; }

    static Runtime& instance() {
        if (!current_) throw std::runtime_error("Runtime not initialized");
        return *current_;
    }

    void requestRedraw();

    bool isTestMode() const { return testMode_; }
    int testPort() const { return testPort_; }
    const std::string& testSocketPath() const { return testSocketPath_; }

    // WindowEventObserver
    void onRedrawRequested(Window* window) override;
    void onWindowClosing(Window* window) override;

private:
    void processEvents();
    void waitForEvents();
    void waitForEventsTimeout(int timeoutMs);
    void waitForEventsImpl(int timeoutMs);

    std::atomic<bool> needsRedraw_{false};
    bool running_{true};
    std::vector<std::unique_ptr<Window>> windows_;

    bool testMode_ = false;
    int testPort_ = 8435;
    std::string testSocketPath_;
    bool backendArgInvalid_ = false;

    /// Basename of argv[0] for logging (e.g. `llm_studio`, `terminal`).
    std::string programName_;

    static Runtime* current_;
    friend void requestApplicationRedraw();
};

using Application = Runtime;

} // namespace flux
