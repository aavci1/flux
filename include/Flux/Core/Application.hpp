#pragma once

#include <Flux/Core/WindowEventObserver.hpp>
#include <Flux/Core/ResourceManager.hpp>
#include <atomic>
#include <string>
#include <vector>
#include <memory>

namespace flux {

class Window;
class OverlayManager;
struct WindowConfig;

class Application : public WindowEventObserver {
public:
    explicit Application(int argc = 0, char** argv = nullptr);
    ~Application();

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    Window& createWindow(const WindowConfig& config);

    int exec();
    void quit() { running_ = false; }

    static Application& instance() {
        if (!current_) throw std::runtime_error("Application not initialized");
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
    std::atomic<uint64_t> bodyGeneration_{0};
    bool running_{true};
    std::vector<std::unique_ptr<Window>> windows_;

    bool testMode_ = false;
    int testPort_ = 8435;
    std::string testSocketPath_;
    bool backendArgInvalid_ = false;

    /// Basename of argv[0] for logging (e.g. `llm_studio`, `terminal`).
    std::string programName_;

    ResourceManager resourceManager_;

    static Application* current_;
    friend void requestApplicationRedraw();
    friend void requestRedrawOnly();

public:
    uint64_t bodyGeneration() const { return bodyGeneration_.load(std::memory_order_relaxed); }
    void bumpBodyGeneration() { bodyGeneration_.fetch_add(1, std::memory_order_relaxed); }

    ResourceManager& resourceManager() { return resourceManager_; }
    const ResourceManager& resourceManager() const { return resourceManager_; }

    OverlayManager* findOverlayManager();

    static bool hasInstance() { return current_ != nullptr; }
};

} // namespace flux
