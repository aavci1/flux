#include <Flux/Core/Window.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Core/LayoutTree.hpp>
#include <Flux/Core/KeyboardInputHandler.hpp>
#include <Flux/Core/MouseInputHandler.hpp>
#include <Flux/Core/FocusState.hpp>
#include <Flux/Core/ShortcutManager.hpp>
#include <Flux/Core/WindowEventObserver.hpp>
#include <Flux/Platform/PlatformRegistry.hpp>
#include <Flux/Platform/PlatformWindowFactory.hpp>
#include <Flux/Graphics/Renderer.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <Flux/Platform/PlatformWindow.hpp>
#include "../Testing/TestServer.hpp"
#include "../Testing/ScreenCapture.hpp"

#include <Flux/Core/Log.hpp>
#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>

namespace flux {

// WindowImpl - pImpl idiom to hide implementation details
struct Window::WindowImpl {
    // Configuration and state
    WindowConfig config;
    Size currentSize;
    
    // Platform and rendering
    std::unique_ptr<PlatformWindow> platformWindow;
    std::unique_ptr<ImmediateModeRenderer> renderer;
    View rootView;
    
    // Input subsystems
    KeyboardInputHandler keyboardHandler;
    MouseInputHandler mouseHandler;
    FocusState focusState;
    std::unique_ptr<ShortcutManager> shortcutManager;
    
    // Observer pattern
    std::vector<WindowEventObserver*> observers;
    
    // Post-render hook
    std::function<void()> postRenderCallback;

    // Test mode
    std::unique_ptr<TestServer> testServer;
    std::unique_ptr<ScreenCapture> testCapture;
    
    // Constructor
    WindowImpl(const WindowConfig& cfg, PlatformWindowFactory* factory)
        : config(cfg)
        , currentSize(cfg.size) {
        
        // Create platform window using factory
        platformWindow = factory->createWindow(
            cfg.title,
            cfg.size,
            cfg.resizable,
            cfg.fullscreen
        );
        
        FLUX_LOG_INFO("Using %s backend", factory->getPlatformName().c_str());
        
        // Create renderer
        renderer = std::make_unique<ImmediateModeRenderer>(platformWindow->renderContext());
        
        // Initialize shortcut manager with default shortcuts
        shortcutManager = std::make_unique<ShortcutManager>();
        registerDefaultShortcuts();
        
        FLUX_LOG_INFO("Created window \"%s\" size: %gx%g", cfg.title.c_str(), cfg.size.width, cfg.size.height);
    }
    
    void registerDefaultShortcuts() {
        PlatformRegistry::instance().registerWindowShortcuts(*shortcutManager);
    }
    
    void notifyObservers(std::function<void(WindowEventObserver*, Window*)> callback, Window* window) {
        for (auto* observer : observers) {
            callback(observer, window);
        }
    }
};

// Window public interface implementation

Window::Window(const WindowConfig& config)
    : Window(config, PlatformRegistry::instance().windowFactory()) {
}

Window::Window(const WindowConfig& config, PlatformWindowFactory* factory)
    : impl_(std::make_unique<WindowImpl>(config, factory)) {
    impl_->platformWindow->setFluxWindow(this);
    impl_->renderer->setWindow(this);
}

Window::~Window() {
    FLUX_LOG_INFO("Destroyed window \"%s\"", impl_->config.title.c_str());
}

// Window management

void Window::setRootView(View component) {
    impl_->rootView = std::move(component);
    impl_->renderer->setRootView(impl_->rootView);
    requestRedraw();
}

void Window::render() {
    if (!impl_->renderer) return;
    
    Rect windowBounds = {0, 0, impl_->currentSize.width, impl_->currentSize.height};
    impl_->renderer->renderFrame(windowBounds);
    if (impl_->postRenderCallback) {
        impl_->postRenderCallback();
    }

    if (impl_->testServer) {
        if (impl_->testCapture && impl_->testServer->needsScreenshotCapture()) {
            impl_->testCapture->capture(*this);
            impl_->testServer->updateScreenshot(impl_->testCapture->getPng());
            impl_->testServer->notifyScreenshotCaptured();
        }
        if (impl_->renderer->hasValidLayout()) {
            if (std::getenv("FLUX_TEST_PROFILE")) {
                auto t0 = std::chrono::steady_clock::now();
                std::string json = TestServer::serializeUITree(impl_->renderer->getCachedLayoutTree());
                auto ms = std::chrono::duration<double, std::milli>(
                    std::chrono::steady_clock::now() - t0).count();
                std::cerr << "[FLUX_TEST_PROFILE] serializeUITree " << ms << " ms\n";
                impl_->testServer->updateUITreeJSON(std::move(json));
            } else {
                std::string json = TestServer::serializeUITree(impl_->renderer->getCachedLayoutTree());
                impl_->testServer->updateUITreeJSON(std::move(json));
            }
        }
        impl_->testServer->signalFrameComplete();
    }

    impl_->platformWindow->swapBuffers();
}

void Window::resize(const Size& newSize) {
    impl_->currentSize = newSize;
    
    if (impl_->platformWindow) {
        impl_->platformWindow->resize(newSize);
    }
    
    FLUX_LOG_INFO("Resized to %gx%g", newSize.width, newSize.height);
    requestRedraw();
}

void Window::setFullscreen(bool fullscreen) {
    impl_->config.fullscreen = fullscreen;
    
    if (impl_->platformWindow) {
        impl_->platformWindow->setFullscreen(fullscreen);
    }
    
    FLUX_LOG_INFO("Fullscreen: %s", fullscreen ? "ON" : "OFF");
}

void Window::setTitle(const std::string& title) {
    impl_->config.title = title;
    
    if (impl_->platformWindow) {
        impl_->platformWindow->setTitle(title);
    }
    
    FLUX_LOG_INFO("Title changed to \"%s\"", title.c_str());
}

unsigned int Window::windowID() const {
    if (impl_->platformWindow) {
        return impl_->platformWindow->windowID();
    }
    return 0;
}

Size Window::getSize() const {
    return impl_->currentSize;
}

// Event handling

void Window::handleMouseMove(float x, float y) {
    Rect windowBounds = {0, 0, impl_->currentSize.width, impl_->currentSize.height};
    impl_->mouseHandler.handleMouseMove(x, y, windowBounds, impl_->renderer.get());
}

void Window::handleMouseDown(int button, float x, float y) {
    Rect windowBounds = {0, 0, impl_->currentSize.width, impl_->currentSize.height};
    impl_->mouseHandler.handleMouseDown(button, x, y, windowBounds, impl_->renderer.get());
}

void Window::handleMouseUp(int button, float x, float y) {
    Rect windowBounds = {0, 0, impl_->currentSize.width, impl_->currentSize.height};
    impl_->mouseHandler.handleMouseUp(button, x, y, windowBounds, impl_->renderer.get());
}

void Window::handleMouseScroll(float x, float y, float deltaX, float deltaY) {
    Rect windowBounds = {0, 0, impl_->currentSize.width, impl_->currentSize.height};
    impl_->mouseHandler.handleMouseScroll(x, y, deltaX, deltaY, windowBounds, impl_->renderer.get());
}

void Window::handleKeyDown(int key) {
    impl_->keyboardHandler.handleKeyDown(key);
    processKeyDownPipeline();
}

void Window::handleKeyDown(int key, KeyModifier platformModifiers) {
    impl_->keyboardHandler.handleKeyDown(key, platformModifiers);
    processKeyDownPipeline();
}

void Window::processKeyDownPipeline() {
    const auto& pendingEvents = impl_->keyboardHandler.getPendingKeyDown();
    if (!pendingEvents.empty()) {
        const KeyEvent& event = pendingEvents.back();

        if (impl_->shortcutManager->handleShortcut(event, *this)) {
            KeyEvent saved = event;
            // Clear only key events; PasteCommand may have queued text via handleTextInput.
            impl_->keyboardHandler.clearPendingKeyEvents();
            // Re-queue the same event so the next frame dispatches to focused views without
            // running shortcut matching again (legacy behavior for copy/cut/paste, etc.).
            impl_->keyboardHandler.enqueueSyntheticKeyDown(std::move(saved));
            requestRedraw();
            return;
        }

        if (event.key == Key::Tab && !event.hasCtrl() && !event.hasAlt()
            && impl_->focusState.getFocusableViewCount() > 1) {
            if (event.hasShift()) {
                impl_->focusState.focusPrevious();
            } else {
                impl_->focusState.focusNext();
            }
            impl_->keyboardHandler.clearPendingEvents();
            requestRedraw();
            return;
        }
    }

    requestRedraw();
}

void Window::handleKeyUp(int key) {
    impl_->keyboardHandler.handleKeyUp(key);
    requestRedraw();
}

void Window::handleKeyUp(int key, KeyModifier platformModifiers) {
    impl_->keyboardHandler.handleKeyUp(key, platformModifiers);
    requestRedraw();
}

void Window::handleTextInput(const std::string& text) {
    impl_->keyboardHandler.handleTextInput(text);
    requestRedraw();
}

void Window::handleResize(const Size& newSize) {
    FLUX_LOG_DEBUG("Internal handleResize called with %gx%g", newSize.width, newSize.height);
    
    impl_->currentSize = newSize;
    
    if (impl_->platformWindow) {
        impl_->platformWindow->resize(newSize);
    }
    
    if (impl_->renderer) {
        impl_->renderer->invalidateLayoutCache();
    }
    
    impl_->notifyObservers(
        [&](WindowEventObserver* obs, Window* win) {
            obs->onWindowResized(win, newSize.width, newSize.height);
        },
        this
    );
    
    render();
}

// Cursor management

void Window::setCursor(CursorType cursor) {
    if (impl_->platformWindow) {
        impl_->platformWindow->setCursor(cursor);
    }
}

CursorType Window::currentCursor() const {
    if (impl_->platformWindow) {
        return impl_->platformWindow->currentCursor();
    }
    return CursorType::Default;
}

// Observer pattern

void Window::addObserver(WindowEventObserver* observer) {
    impl_->observers.push_back(observer);
}

void Window::removeObserver(WindowEventObserver* observer) {
    impl_->observers.erase(
        std::remove(impl_->observers.begin(), impl_->observers.end(), observer),
        impl_->observers.end()
    );
}

void Window::requestRedraw() {
    // Invalidate layout cache when redraw is requested due to state changes
    if (impl_->renderer) {
        impl_->renderer->invalidateLayoutCache();
    }
    
    impl_->notifyObservers(
        [](WindowEventObserver* obs, Window* win) {
            obs->onRedrawRequested(win);
        },
        this
    );
}

// Subsystem access

KeyboardInputHandler& Window::keyboard() {
    return impl_->keyboardHandler;
}

MouseInputHandler& Window::mouse() {
    return impl_->mouseHandler;
}

FocusState& Window::focus() {
    return impl_->focusState;
}

ShortcutManager& Window::shortcuts() {
    return *impl_->shortcutManager;
}

bool Window::isCursorBlinkActive() const {
    return impl_->renderer && impl_->renderer->isCursorBlinkActive();
}

// Hooks

void Window::setPostRenderCallback(std::function<void()> callback) {
    impl_->postRenderCallback = std::move(callback);
}

// Internal

void* Window::platformWindow() {
    return impl_->platformWindow.get();
}

void Window::processPendingEvents(LayoutNode& layoutTree) {
    impl_->keyboardHandler.processPendingEvents(layoutTree, impl_->focusState);
}

// Testing support

void Window::enableTestMode(int tcpPort, const std::string& unixSocketPath) {
    impl_->testCapture = std::make_unique<ScreenCapture>();
    impl_->testServer = std::make_unique<TestServer>(*this, tcpPort, unixSocketPath);
    if (impl_->platformWindow) {
        impl_->platformWindow->setGpuReadbackEnabled(true);
    }
    impl_->testServer->start();
}

void Window::processSyntheticEvents() {
    if (!impl_->testServer) return;

    auto events = impl_->testServer->drainEvents();
    for (auto& e : events) {
        switch (e.type) {
            case TestServer::SyntheticEvent::Click:
                handleMouseDown(0, e.x, e.y);
                handleMouseUp(0, e.x, e.y);
                break;
            case TestServer::SyntheticEvent::TextInput:
                handleTextInput(e.text);
                break;
            case TestServer::SyntheticEvent::KeyPress:
                if (e.explicitKeyModifiers) {
                    KeyEvent ev;
                    ev.key = keyFromRawCode(static_cast<uint32_t>(e.keyCode));
                    ev.modifiers = e.keyModifiers;
                    ev.rawKeyCode = static_cast<uint32_t>(e.keyCode);
                    ev.isRepeat = false;
                    if (impl_->shortcutManager->handleShortcut(ev, *this)) {
                        // Shortcut consumed (e.g. paste)
                    } else if (ev.key == Key::Tab && !ev.hasCtrl() && !ev.hasAlt()
                               && impl_->focusState.getFocusableViewCount() > 1) {
                        if (ev.hasShift()) {
                            impl_->focusState.focusPrevious();
                        } else {
                            impl_->focusState.focusNext();
                        }
                    } else {
                        impl_->keyboardHandler.enqueueSyntheticKeyDown(ev);
                    }
                } else {
                    handleKeyDown(e.keyCode);
                    handleKeyUp(e.keyCode);
                }
                break;
            case TestServer::SyntheticEvent::Scroll:
                handleMouseScroll(e.x, e.y, e.deltaX, e.deltaY);
                break;
            case TestServer::SyntheticEvent::Hover:
                handleMouseMove(e.x, e.y);
                break;
            case TestServer::SyntheticEvent::MouseDown:
                handleMouseDown(0, e.x, e.y);
                break;
            case TestServer::SyntheticEvent::MouseUp:
                handleMouseUp(0, e.x, e.y);
                break;
        }
    }

    if (!events.empty()) {
        impl_->testServer->markEventsProcessed(events);
        requestRedraw();
    }
}

} // namespace flux
