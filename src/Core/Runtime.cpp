#include <Flux/Core/Runtime.hpp>
#include <Flux/Core/Window.hpp>
#include <Flux/Core/OverlayManager.hpp>
#include <Flux/Platform/EventLoopWake.hpp>
#include <Flux/Platform/PlatformRegistry.hpp>
#include <Flux/Platform/PlatformWindowFactory.hpp>
#include <Flux/Platform/PlatformWindow.hpp>
#include <Flux/Platform/MemoryFootprint.hpp>
#include <Flux/Animation/AnimationEngine.hpp>
#include <Flux/Core/Log.hpp>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>

namespace flux {

Runtime* Runtime::current_ = nullptr;

static std::atomic<uint64_t> bodyGeneration_{0};

static thread_local int suppressRedrawRequests_ = 0;

void suppressRedrawRequests() { ++suppressRedrawRequests_; }
void resumeRedrawRequests()   { --suppressRedrawRequests_; }

void requestApplicationRedraw() {
    if (suppressRedrawRequests_ > 0) return;
    bodyGeneration_.fetch_add(1, std::memory_order_relaxed);
    if (Runtime::current_) {
        Runtime::current_->requestRedraw();
    }
}

void requestRedrawOnly() {
    if (suppressRedrawRequests_ > 0) return;
    if (Runtime::current_) {
        Runtime::current_->requestRedraw();
    }
}

uint64_t currentBodyGeneration() {
    return bodyGeneration_.load(std::memory_order_relaxed);
}

OverlayManager* Runtime::findOverlayManager() {
    if (windows_.empty()) return nullptr;
    return windows_.front()->overlayManager();
}

void showOverlay(const std::string& id, View content, Rect anchor, OverlayConfig config) {
    if (!Runtime::hasInstance()) return;
    if (auto* mgr = Runtime::instance().findOverlayManager()) {
        mgr->show(id, std::move(content), anchor, std::move(config));
    }
}

void hideOverlay(const std::string& id) {
    if (!Runtime::hasInstance()) return;
    if (auto* mgr = Runtime::instance().findOverlayManager()) {
        mgr->hide(id);
    }
}

void hideAllOverlays() {
    if (!Runtime::hasInstance()) return;
    if (auto* mgr = Runtime::instance().findOverlayManager()) {
        mgr->hideAll();
    }
}

void Runtime::requestRedraw() {
    needsRedraw_.store(true, std::memory_order_relaxed);
    wakePlatformEventLoop();
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
        } else if (std::strcmp(argv[i], "--test-socket") == 0 && i + 1 < argc) {
            testSocketPath_ = argv[++i];
        } else if (std::strcmp(argv[i], "--backend") == 0) {
            if (i + 1 >= argc) {
                backendArgInvalid_ = true;
                FLUX_LOG_ERROR("Missing value for --backend");
                continue;
            }
            const char* b = argv[++i];
            PlatformWindowFactory* factory = PlatformRegistry::instance().windowFactory();
            bool ok = false;
            if (std::strcmp(b, "metal") == 0) {
#ifdef __APPLE__
                try {
                    factory->setRenderBackend(RenderBackendType::GPU_Metal);
                    ok = true;
                } catch (const std::exception& ex) {
                    (void)ex;
                    ok = false;
                }
#else
                (void)factory;
#endif
            } else if (std::strcmp(b, "vulkan") == 0) {
#ifndef __APPLE__
#if defined(FLUX_HAS_VULKAN)
                factory->setRenderBackend(RenderBackendType::GPU_Vulkan);
                ok = true;
#endif
#endif
            } else if (std::strcmp(b, "gpu") == 0) {
                try {
                    factory->setRenderBackend(RenderBackendType::GPU_Auto);
                    ok = true;
                } catch (const std::exception&) {
                    ok = false;
                }
            }
            if (!ok) {
                backendArgInvalid_ = true;
                FLUX_LOG_ERROR("Unknown or unavailable backend \"%s\"", b);
            }
        }
    }

    if (argv && argv[0] && argv[0][0] != '\0') {
        try {
            programName_ = std::filesystem::path(argv[0]).filename().string();
        } catch (...) {
            programName_ = "flux";
        }
    }
}

Runtime::~Runtime() {
    windows_.clear();
    current_ = nullptr;
}

Window& Runtime::createWindow(const WindowConfig& config) {
    if (backendArgInvalid_) {
        std::exit(1);
    }
    auto window = std::make_unique<Window>(config);
    window->addObserver(this);
    Window& ref = *window;
    windows_.push_back(std::move(window));
    requestRedraw();

    if (testMode_) {
        ref.enableTestMode(testPort_, testSocketPath_);
    }

    return ref;
}

int Runtime::run() {
    if (backendArgInvalid_) {
        return 1;
    }
    bool memoryReportAfterFirstFrame = false;
    auto lastFrameTime = std::chrono::steady_clock::now();

    while (running_) {
        bool animating = AnimationEngine::instance().hasActiveAnimations();
        bool redrawPending = needsRedraw_.load(std::memory_order_relaxed);

        if (animating) {
            auto now = std::chrono::steady_clock::now();
            float dt = std::chrono::duration<float>(now - lastFrameTime).count();
            lastFrameTime = now;
            AnimationEngine::instance().tick(dt);
            needsRedraw_.store(true, std::memory_order_relaxed);
            redrawPending = true;

            processEvents();
        } else if (redrawPending) {
            lastFrameTime = std::chrono::steady_clock::now();
            processEvents();
        } else {
            bool blinkActive = false;
            for (auto& window : windows_) {
                if (window->isCursorBlinkActive()) { blinkActive = true; break; }
            }
            if (blinkActive) {
                waitForEventsTimeout(500);
                needsRedraw_.store(true, std::memory_order_relaxed);
            } else {
                waitForEvents();
            }
            lastFrameTime = std::chrono::steady_clock::now();
        }

        for (auto& window : windows_) {
            window->processSyntheticEvents();
        }

        if (needsRedraw_.exchange(false, std::memory_order_relaxed)) {
            for (auto& window : windows_) {
                window->render();
            }
            if (!memoryReportAfterFirstFrame) {
                memoryReportAfterFirstFrame = true;
                const std::string tag =
                    programName_.empty() ? std::string("flux") : programName_;
                logMemoryFootprintIfRequested((tag + ": first frame").c_str());
            }
        }

        // If animations are still running, keep the event loop awake
        if (AnimationEngine::instance().hasActiveAnimations()) {
            needsRedraw_.store(true, std::memory_order_relaxed);
            wakePlatformEventLoop();
        }
    }
    return 0;
}

void Runtime::onRedrawRequested(Window* window) {
    (void)window;
    requestRedraw();
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
    waitForEventsImpl(-1);
}

void Runtime::waitForEventsTimeout(int timeoutMs) {
    waitForEventsImpl(timeoutMs);
}

void Runtime::waitForEventsImpl(int timeoutMs) {
    if (windows_.empty()) return;
    if (auto* platformWindow = static_cast<PlatformWindow*>(windows_.front()->platformWindow())) {
        platformWindow->waitForEvents(timeoutMs);
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
