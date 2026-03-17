#include <Flux/Core/Window.hpp>
#include <Flux/Core/LayoutTree.hpp>
#include <Flux/Core/KeyboardInputHandler.hpp>
#include <Flux/Core/MouseInputHandler.hpp>
#include <Flux/Core/FocusState.hpp>
#include <Flux/Core/ShortcutManager.hpp>
#include <Flux/Core/WindowEventObserver.hpp>
#include <Flux/Core/PlatformWindowFactory.hpp>
#include <Flux/Graphics/Renderer.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <Flux/Graphics/NanoVGRenderContext.hpp>
#include <Flux/Platform/PlatformWindow.hpp>

#include <Flux/Core/Log.hpp>
#include <algorithm>

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
        
        FLUX_LOG_INFO("Using %s + NanoVG backend", factory->getPlatformName().c_str());
        
        // Create renderer
        renderer = std::make_unique<ImmediateModeRenderer>(platformWindow->renderContext());
        
        // Initialize shortcut manager with default shortcuts
        shortcutManager = std::make_unique<ShortcutManager>();
        registerDefaultShortcuts();
        
        FLUX_LOG_INFO("Created window \"%s\" size: %gx%g", cfg.title.c_str(), cfg.size.width, cfg.size.height);
    }
    
    void registerDefaultShortcuts() {
        // Ctrl+Q - Quit
        shortcutManager->registerShortcut(
            {Key::Q, KeyModifier::Ctrl},
            std::make_unique<QuitCommand>()
        );
        
        // Ctrl+C - Copy
        shortcutManager->registerShortcut(
            {Key::C, KeyModifier::Ctrl},
            std::make_unique<CopyCommand>()
        );
        
        // Ctrl+V - Paste
        shortcutManager->registerShortcut(
            {Key::V, KeyModifier::Ctrl},
            std::make_unique<PasteCommand>()
        );
        
        // Ctrl+X - Cut
        shortcutManager->registerShortcut(
            {Key::X, KeyModifier::Ctrl},
            std::make_unique<CutCommand>()
        );
        
        // Ctrl+A - Select All
        shortcutManager->registerShortcut(
            {Key::A, KeyModifier::Ctrl},
            std::make_unique<SelectAllCommand>()
        );
    }
    
    void notifyObservers(std::function<void(WindowEventObserver*, Window*)> callback, Window* window) {
        for (auto* observer : observers) {
            callback(observer, window);
        }
    }
};

// Window public interface implementation

Window::Window(const WindowConfig& config)
    : Window(config, getDefaultPlatformFactory()) {
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
    
    // Check if we should handle Tab navigation or shortcuts
    const auto& pendingEvents = impl_->keyboardHandler.getPendingKeyDown();
    if (!pendingEvents.empty()) {
        const KeyEvent& event = pendingEvents.back();
        
        // Handle shortcuts first
        if (impl_->shortcutManager->handleShortcut(event, *this)) {
            // Shortcut consumed the event, remove it from pending
            impl_->keyboardHandler.clearPendingEvents();
            impl_->keyboardHandler.handleKeyDown(key); // Re-add for tracking
            return;
        }
        
        // Handle Tab navigation
        if (event.key == Key::Tab && !event.hasCtrl() && !event.hasAlt()) {
            if (event.hasShift()) {
                impl_->focusState.focusPrevious();
            } else {
                impl_->focusState.focusNext();
            }
            // Clear the pending event since we handled it
            impl_->keyboardHandler.clearPendingEvents();
            return;
        }
    }
    
    FLUX_LOG_DEBUG("Key down queued for next frame");
}

void Window::handleKeyUp(int key) {
    impl_->keyboardHandler.handleKeyUp(key);
}

void Window::handleTextInput(const std::string& text) {
    impl_->keyboardHandler.handleTextInput(text);
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

} // namespace flux
