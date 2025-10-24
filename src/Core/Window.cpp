#include <Flux/Core/Window.hpp>
#include <Flux/Core/Application.hpp>
#include <Flux/Core/LayoutTree.hpp>
#include <Flux/Graphics/Renderer.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <Flux/Platform/PlatformWindow.hpp>
#include <Flux/Platform/WaylandWindow.hpp>

#include <iostream>
#include <cstdlib>

namespace flux {

Window::Window(const WindowConfig& config)
    : config_(config)
    , currentSize_(config.size)
    , currentModifiers_(KeyModifier::None) {

    // Create Wayland window (Linux/Wayland only)
    platformWindow_ = std::make_unique<WaylandWindow>(
        config.title,
        config.size,
        config.resizable,
        config.fullscreen
    );
    
    std::cout << "[WINDOW] Using Wayland + NanoVG backend\n";

    // Set the Flux Window reference for resize callbacks
    platformWindow_->setFluxWindow(this);

    renderContext_.reset(); // PlatformWindow owns the render context
    renderer_ = std::make_unique<ImmediateModeRenderer>(platformWindow_->renderContext());
    
    // Set window reference in renderer for cursor management
    auto* immediateRenderer = static_cast<ImmediateModeRenderer*>(renderer_.get());
    immediateRenderer->setWindow(this);

    // Initialize focus manager
    focusManager_ = std::make_unique<FocusManager>(this);

    // Register with the application
    Application::instance().registerWindow(this);

    std::cout << "[WINDOW] Created window \"" << config.title
              << "\" size: " << config.size.width << "x" << config.size.height << "\n";
}

Window::~Window() {
    // Unregister from the application
    Application::instance().unregisterWindow(this);

    std::cout << "[WINDOW] Destroyed window \"" << config_.title << "\"\n";
}

void Window::setRootView(View component) {
    rootView_ = std::move(component);
    renderer_->setRootView(rootView_);
    // Request initial render
    Application::instance().requestRedraw();
}

void Window::render() {
    if (!renderer_) return;

    // Render the root view
    // The renderer will handle scaling based on DPI
    Rect windowBounds = {0, 0, currentSize_.width, currentSize_.height};
    renderer_->renderFrame(windowBounds);

    platformWindow_->swapBuffers();
}

void Window::resize(const Size& newSize) {
    currentSize_ = newSize;

    if (platformWindow_) {
        platformWindow_->resize(newSize);
    }

    std::cout << "[WINDOW] Resized to " << newSize.width << "x" << newSize.height << "\n";

    // Request a redraw with the new size
    Application::instance().requestRedraw();
}

void Window::setFullscreen(bool fullscreen) {
    config_.fullscreen = fullscreen;

    if (platformWindow_) {
        platformWindow_->setFullscreen(fullscreen);
    }

    std::cout << "[WINDOW] Fullscreen: " << (fullscreen ? "ON" : "OFF") << "\n";
}

void Window::setTitle(const std::string& title) {
    config_.title = title;

    if (platformWindow_) {
        platformWindow_->setTitle(title);
    }

    std::cout << "[WINDOW] Title changed to \"" << title << "\"\n";
}

unsigned int Window::windowID() const {
    if (platformWindow_) {
        return platformWindow_->windowID();
    }
    return 0; // Invalid ID
}

void Window::handleMouseMove(float x, float y) {
    Event event;
    event.type = Event::MouseMove;
    event.mouseMove.x = x;
    event.mouseMove.y = y;
    dispatchEvent(event);
}

void Window::handleMouseDown(int button, float x, float y) {
    Event event;
    event.type = Event::MouseDown;
    event.mouseButton.x = x;
    event.mouseButton.y = y;
    event.mouseButton.button = button;
    dispatchEvent(event);
}

void Window::handleMouseUp(int button, float x, float y) {
    Event event;
    event.type = Event::MouseUp;
    event.mouseButton.x = x;
    event.mouseButton.y = y;
    event.mouseButton.button = button;
    dispatchEvent(event);
}

void Window::handleKeyDown(int key) {
    // Update modifier state
    updateModifiers(key, true);
    
    // Create KeyEvent
    KeyEvent event;
    event.key = keyFromRawCode(key);
    event.modifiers = currentModifiers_;
    event.rawKeyCode = key;
    event.isRepeat = false;
    
    std::cout << "[INPUT] Key down: " << keyName(event.key)
              << " (raw: " << key << ", mods: "
              << (event.hasCtrl() ? "Ctrl " : "")
              << (event.hasShift() ? "Shift " : "")
              << (event.hasAlt() ? "Alt " : "")
              << ")\n";
    
    dispatchKeyDown(event);
}

void Window::handleKeyUp(int key) {
    // Update modifier state
    updateModifiers(key, false);
    
    // Create KeyEvent
    KeyEvent event;
    event.key = keyFromRawCode(key);
    event.modifiers = currentModifiers_;
    event.rawKeyCode = key;
    event.isRepeat = false;
    
    std::cout << "[INPUT] Key up: " << keyName(event.key) << " (raw: " << key << ")\n";
    
    dispatchKeyUp(event);
}

void Window::handleTextInput(const std::string& text) {
    std::cout << "[INPUT] Text input: \"" << text << "\"\n";
    
    TextInputEvent event(text);
    dispatchTextInput(event);
}

void Window::handleResize(const Size& newSize) {
    std::cout << "[WINDOW] Internal handleResize called with " << newSize.width << "x" << newSize.height << "\n";

    // Update size from the resize event
    resize(newSize);

    // Resize the platform renderer to match new window size
    if (platformWindow_) {
        platformWindow_->renderContext()->resize(
            static_cast<int>(newSize.width),
            static_cast<int>(newSize.height)
        );
    }

    // Render immediately with this exact size
    render();
}

void Window::dispatchEvent(const Event& event) {
    // Create window bounds for hit testing
    Rect windowBounds = {0, 0, currentSize_.width, currentSize_.height};

    // Dispatch event to renderer for hit testing and view targeting
    if (renderer_) {
        auto* immediateRenderer = static_cast<ImmediateModeRenderer*>(renderer_.get());
        immediateRenderer->handleEvent(event, windowBounds);
    }
}

void Window::setCursor(CursorType cursor) {
    if (platformWindow_) {
        platformWindow_->setCursor(cursor);
    }
}

CursorType Window::currentCursor() const {
    if (platformWindow_) {
        return platformWindow_->currentCursor();
    }
    return CursorType::Default;
}

void Window::dispatchKeyDown(const KeyEvent& event) {
    // Handle global shortcuts first
    if (handleGlobalShortcut(event)) {
        return; // Shortcut consumed the event
    }
    
    // Handle focus navigation
    if (event.key == Key::Tab && !event.hasCtrl() && !event.hasAlt()) {
        if (event.hasShift()) {
            focusManager_->focusPrevious();
        } else {
            focusManager_->focusNext();
        }
        return;
    }
    
    // Queue the event to be processed during the next render frame
    // We can't dispatch directly because View objects are temporary
    pendingKeyDownEvents_.push_back(event);
    std::cout << "[INPUT] Key down queued for next frame\n";
}

void Window::dispatchKeyUp(const KeyEvent& event) {
    // Queue the event to be processed during the next render frame
    pendingKeyUpEvents_.push_back(event);
}

void Window::dispatchTextInput(const TextInputEvent& event) {
    // Queue the event to be processed during the next render frame
    pendingTextInputEvents_.push_back(event);
}

bool Window::handleGlobalShortcut(const KeyEvent& event) {
    // Ctrl+Q - Quit application
    if (event.key == Key::Q && event.hasCtrl()) {
        std::cout << "[SHORTCUT] Ctrl+Q - Quit application\n";
        Application::instance().quit();
        return true;
    }
    
    // Ctrl+C - Copy (placeholder for clipboard integration)
    if (event.key == Key::C && event.hasCtrl()) {
        std::cout << "[SHORTCUT] Ctrl+C - Copy (not yet implemented)\n";
        // TODO: Implement clipboard copy
        return true;
    }
    
    // Ctrl+V - Paste (placeholder for clipboard integration)
    if (event.key == Key::V && event.hasCtrl()) {
        std::cout << "[SHORTCUT] Ctrl+V - Paste (not yet implemented)\n";
        // TODO: Implement clipboard paste
        return true;
    }
    
    // Ctrl+X - Cut (placeholder for clipboard integration)
    if (event.key == Key::X && event.hasCtrl()) {
        std::cout << "[SHORTCUT] Ctrl+X - Cut (not yet implemented)\n";
        // TODO: Implement clipboard cut
        return true;
    }
    
    // Ctrl+A - Select All (placeholder)
    if (event.key == Key::A && event.hasCtrl()) {
        std::cout << "[SHORTCUT] Ctrl+A - Select All (not yet implemented)\n";
        // TODO: Implement select all
        return true;
    }
    
    return false; // Shortcut not handled
}

void Window::updateModifiers(int key, bool pressed) {
    uint32_t mod = static_cast<uint32_t>(currentModifiers_);
    
    switch (key) {
        case static_cast<int>(Key::LeftShift):
        case static_cast<int>(Key::RightShift):
            if (pressed) {
                mod |= static_cast<uint32_t>(KeyModifier::Shift);
            } else {
                mod &= ~static_cast<uint32_t>(KeyModifier::Shift);
            }
            break;
            
        case static_cast<int>(Key::LeftCtrl):
        case static_cast<int>(Key::RightCtrl):
            if (pressed) {
                mod |= static_cast<uint32_t>(KeyModifier::Ctrl);
            } else {
                mod &= ~static_cast<uint32_t>(KeyModifier::Ctrl);
            }
            break;
            
        case static_cast<int>(Key::LeftAlt):
        case static_cast<int>(Key::RightAlt):
            if (pressed) {
                mod |= static_cast<uint32_t>(KeyModifier::Alt);
            } else {
                mod &= ~static_cast<uint32_t>(KeyModifier::Alt);
            }
            break;
            
        case static_cast<int>(Key::LeftSuper):
        case static_cast<int>(Key::RightSuper):
            if (pressed) {
                mod |= static_cast<uint32_t>(KeyModifier::Super);
            } else {
                mod &= ~static_cast<uint32_t>(KeyModifier::Super);
            }
            break;
    }
    
    currentModifiers_ = static_cast<KeyModifier>(mod);
}

void Window::processPendingEvents(LayoutNode& layoutTree) {
    if (!focusManager_) return;
    
    // Process pending key down events
    for (const auto& event : pendingKeyDownEvents_) {
        focusManager_->dispatchKeyDownToFocused(layoutTree, event);
    }
    pendingKeyDownEvents_.clear();
    
    // Process pending key up events
    for (const auto& event : pendingKeyUpEvents_) {
        focusManager_->dispatchKeyUpToFocused(layoutTree, event);
    }
    pendingKeyUpEvents_.clear();
    
    // Process pending text input events
    for (const auto& event : pendingTextInputEvents_) {
        focusManager_->dispatchTextInputToFocused(layoutTree, event);
    }
    pendingTextInputEvents_.clear();
}

} // namespace flux
