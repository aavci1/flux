#include <Flux/Core/Application.hpp>
#include <Flux/Core/Window.hpp>
#include <Flux/Core/OverlayManager.hpp>
#include <Flux/Platform/EventLoopWake.hpp>
#if defined(__APPLE__)
// Public libobjc entry points (used by Swift/clang); not always declared in <objc/runtime.h>.
extern "C" {
void* objc_autoreleasePoolPush(void);
void objc_autoreleasePoolPop(void* pool);
}
#endif
#include <Flux/Platform/PlatformRegistry.hpp>
#include <Flux/Platform/PlatformWindowFactory.hpp>
#include <Flux/Platform/PlatformWindow.hpp>
#include <Flux/Platform/MemoryFootprint.hpp>
#include <Flux/Core/Log.hpp>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <filesystem>

#if defined(__APPLE__)
namespace {
/// One AppKit/Metal frame can create many autoreleased objects; drain each turn of the
/// manual event loop (same role as @autoreleasepool in an NSRunLoop-driven app).
struct MacAutoreleasePoolFrame {
    void* token = objc_autoreleasePoolPush();
    ~MacAutoreleasePoolFrame() { objc_autoreleasePoolPop(token); }
};
} // namespace
#endif

namespace flux {

Application* Application::current_ = nullptr;

static thread_local int suppressRedrawRequests_ = 0;

void suppressRedrawRequests() { ++suppressRedrawRequests_; }
void resumeRedrawRequests()   { --suppressRedrawRequests_; }

void requestApplicationRedraw() {
    if (suppressRedrawRequests_ > 0) return;
    if (Application::current_) {
        Application::current_->bumpBodyGeneration();
        Application::current_->requestRedraw();
    }
}

uint64_t currentBodyGeneration() {
    if (!Application::hasInstance()) return 0;
    return Application::instance().bodyGeneration();
}

OverlayManager* Application::findOverlayManager() {
    if (windows_.empty()) return nullptr;
    return windows_.front()->overlayManager();
}

void showOverlay(const std::string& id, View content, Rect anchor, OverlayConfig config) {
    if (!Application::hasInstance()) return;
    if (auto* mgr = Application::instance().findOverlayManager()) {
        mgr->show(id, std::move(content), anchor, std::move(config));
    }
}

void hideOverlay(const std::string& id) {
    if (!Application::hasInstance()) return;
    if (auto* mgr = Application::instance().findOverlayManager()) {
        mgr->hide(id);
    }
}

void hideAllOverlays() {
    if (!Application::hasInstance()) return;
    if (auto* mgr = Application::instance().findOverlayManager()) {
        mgr->hideAll();
    }
}

void Application::requestRedraw() {
    needsRedraw_.store(true, std::memory_order_relaxed);
    wakePlatformEventLoop();
}

Application::Application(int argc, char** argv) {
    if (current_) {
        throw std::runtime_error("Only one Application instance allowed");
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

Application::~Application() {
    windows_.clear();
    current_ = nullptr;
}

Window& Application::createWindow(const WindowConfig& config) {
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

int Application::exec() {
    if (backendArgInvalid_) {
        return 1;
    }
    bool memoryReportAfterFirstFrame = false;
    while (running_) {
#if defined(__APPLE__)
        MacAutoreleasePoolFrame macAutoreleasePoolFrame;
#endif
        bool redrawPending = needsRedraw_.load(std::memory_order_relaxed);

        if (redrawPending) {
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
    }
    return 0;
}

void Application::onRedrawRequested(Window* window) {
    (void)window;
    requestRedraw();
}

void Application::onWindowClosing(Window* window) {
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

void Application::processEvents() {
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

void Application::waitForEvents() {
    waitForEventsImpl(-1);
}

void Application::waitForEventsTimeout(int timeoutMs) {
    waitForEventsImpl(timeoutMs);
}

void Application::waitForEventsImpl(int timeoutMs) {
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
