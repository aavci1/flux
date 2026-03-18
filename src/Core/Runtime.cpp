#include <Flux/Core/Runtime.hpp>
#include <Flux/Core/Window.hpp>
#include <Flux/Platform/PlatformWindow.hpp>
#include <Flux/Core/Log.hpp>
#include <algorithm>
#include <cstring>

namespace flux {

Runtime* Runtime::current_ = nullptr;

static std::atomic<uint64_t> bodyGeneration_{0};

void requestApplicationRedraw() {
    bodyGeneration_.fetch_add(1, std::memory_order_relaxed);
    if (Runtime::current_) {
        Runtime::current_->requestRedraw();
    }
}

uint64_t currentBodyGeneration() {
    return bodyGeneration_.load(std::memory_order_relaxed);
}

Runtime::Runtime(int argc, char** argv) {
    if (current_) {
        throw std::runtime_error("Only one Runtime instance allowed");
    }
    current_ = this;

    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--test-mode") == 0) {
            testMode_ = true;
        } else if (std::strcmp(argv[i], "--test-port") == 0 && i + 1 < argc) {
            testPort_ = std::atoi(argv[++i]);
        }
    }
}

Runtime::~Runtime() {
    windows_.clear();
    current_ = nullptr;
}

Window& Runtime::createWindow(const WindowConfig& config) {
    auto window = std::make_unique<Window>(config);
    window->addObserver(this);
    Window& ref = *window;
    windows_.push_back(std::move(window));
    needsRedraw_.store(true, std::memory_order_relaxed);

    if (testMode_) {
        ref.enableTestMode(testPort_);
    }

    return ref;
}

int Runtime::run() {
    while (running_) {
        bool redrawPending = needsRedraw_.load(std::memory_order_relaxed);

        if (redrawPending) {
            processEvents();
        } else {
            waitForEvents();
        }

        for (auto& window : windows_) {
            window->processSyntheticEvents();
        }

        if (needsRedraw_.exchange(false, std::memory_order_relaxed)) {
            for (auto& window : windows_) {
                window->render();
            }
        }
    }
    return 0;
}

void Runtime::onRedrawRequested(Window* window) {
    (void)window;
    needsRedraw_.store(true, std::memory_order_relaxed);
}

void Runtime::onWindowClosing(Window* window) {
    auto it = std::find_if(windows_.begin(), windows_.end(),
        [window](const auto& w) { return w.get() == window; });
    if (it != windows_.end()) {
        (*it)->removeObserver(this);
        windows_.erase(it);
    }
    if (windows_.empty()) {
        quit();
    }
}

void Runtime::processEvents() {
    for (auto& window : windows_) {
        if (auto* platformWindow = static_cast<PlatformWindow*>(window->platformWindow())) {
            platformWindow->processEvents();
            if (platformWindow->shouldClose()) {
                quit();
                break;
            }
        }
    }
}

void Runtime::waitForEvents() {
    if (windows_.empty()) return;
    if (auto* platformWindow = static_cast<PlatformWindow*>(windows_.front()->platformWindow())) {
        platformWindow->waitForEvents(-1);
        if (platformWindow->shouldClose()) {
            quit();
            return;
        }
        for (size_t i = 1; i < windows_.size(); ++i) {
            if (auto* pw = static_cast<PlatformWindow*>(windows_[i]->platformWindow())) {
                pw->processEvents();
                if (pw->shouldClose()) { quit(); return; }
            }
        }
    }
}

} // namespace flux
