#include <Flux/Core/Window.hpp>
#include <Flux/Core/Application.hpp>
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

#include <iostream>
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
        
        std::cout << "[WINDOW] Using " << factory->getPlatformName() << " + NanoVG backend\n";
        
        // Create renderer
        renderer = std::make_unique<ImmediateModeRenderer>(platformWindow->renderContext());
        
        // Initialize shortcut manager with default shortcuts
        shortcutManager = std::make_unique<ShortcutManager>();
        registerDefaultShortcuts();
        
        std::cout << "[WINDOW] Created window \"" << cfg.title
                  << "\" size: " << cfg.size.width << "x" << cfg.size.height << "\n";
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
    
    // Set window reference for platform callbacks
    impl_->platformWindow->setFluxWindow(this);
    
    // Set window reference in renderer for cursor management
    impl_->renderer->setWindow(this);
    
    // Register with application
    Application::instance().registerWindow(this);
}

Window::~Window() {
    // Unregister from application
    Application::instance().unregisterWindow(this);
    
    std::cout << "[WINDOW] Destroyed window \"" << impl_->config.title << "\"\n";
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
    impl_->platformWindow->swapBuffers();
}

void Window::resize(const Size& newSize) {
    impl_->currentSize = newSize;
    
    if (impl_->platformWindow) {
        impl_->platformWindow->resize(newSize);
    }
    
    std::cout << "[WINDOW] Resized to " << newSize.width << "x" << newSize.height << "\n";
    requestRedraw();
}

void Window::setFullscreen(bool fullscreen) {
    impl_->config.fullscreen = fullscreen;
    
    if (impl_->platformWindow) {
        impl_->platformWindow->setFullscreen(fullscreen);
    }
    
    std::cout << "[WINDOW] Fullscreen: " << (fullscreen ? "ON" : "OFF") << "\n";
}

void Window::setTitle(const std::string& title) {
    impl_->config.title = title;
    
    if (impl_->platformWindow) {
        impl_->platformWindow->setTitle(title);
    }
    
    std::cout << "[WINDOW] Title changed to \"" << title << "\"\n";
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
    
    std::cout << "[INPUT] Key down queued for next frame\n";
}

void Window::handleKeyUp(int key) {
    impl_->keyboardHandler.handleKeyUp(key);
}

void Window::handleTextInput(const std::string& text) {
    impl_->keyboardHandler.handleTextInput(text);
}

void Window::handleResize(const Size& newSize) {
    std::cout << "[WINDOW] Internal handleResize called with " << newSize.width << "x" << newSize.height << "\n";
    
    resize(newSize);
    
    if (impl_->platformWindow) {
        impl_->platformWindow->renderContext()->resize(
            static_cast<int>(newSize.width),
            static_cast<int>(newSize.height)
        );
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

// Internal

void* Window::platformWindow() {
    return impl_->platformWindow.get();
}

void Window::processPendingEvents(LayoutNode& layoutTree) {
    impl_->keyboardHandler.processPendingEvents(layoutTree, impl_->focusState);
}

} // namespace flux
