#include <Flux/Platform/SDLWindow.hpp>
#include <Flux/Platform/GPUPlatformRenderer.hpp>
#include <Flux/Platform/NativeGraphicsSurface.hpp>
#include <Flux/Core/Window.hpp>
#include <Flux/Core/Log.hpp>
#include <stdexcept>

namespace flux {

std::unordered_map<SDL_WindowID, SDLWindow*> SDLWindow::windowMap_;

SDLWindow::SDLWindow(const std::string& title, const Size& size, bool resizable, bool fullscreen,
                     RenderBackendType backend)
    : size_(size)
    , fullscreen_(fullscreen)
    , backendType_(backend)
{
    if (backend == RenderBackendType::GPU_Auto) {
        backendType_ = RenderBackendType::GPU_Vulkan;
    }

    Uint64 flags = SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_VULKAN;
    if (resizable) flags |= SDL_WINDOW_RESIZABLE;
    if (fullscreen) flags |= SDL_WINDOW_FULLSCREEN;

    window_ = SDL_CreateWindow(
        title.c_str(),
        static_cast<int>(size.width),
        static_cast<int>(size.height),
        flags
    );

    if (!window_) {
        throw std::runtime_error(std::string("Failed to create SDL window: ") + SDL_GetError());
    }

    float dpi = SDL_GetWindowDisplayScale(window_);

    auto gpuRenderer = std::make_unique<GPUPlatformRenderer>(gpu::Backend::Vulkan);
    gpuRenderer->setGraphicsSurface(gpu::NativeGraphicsSurface::fromSdlWindow(window_));
    if (!gpuRenderer->initialize(
            static_cast<int>(size.width),
            static_cast<int>(size.height),
            dpi, dpi)) {
        throw std::runtime_error("Failed to initialize GPU renderer");
    }
    renderer_ = std::move(gpuRenderer);

    windowMap_[SDL_GetWindowID(window_)] = this;

    SDL_StartTextInput(window_);
    SDL_AddEventWatch(liveResizeWatcher, this);
}

SDLWindow::~SDLWindow() {
    SDL_RemoveEventWatch(liveResizeWatcher, this);
    if (window_) {
        SDL_StopTextInput(window_);
        windowMap_.erase(SDL_GetWindowID(window_));
    }
    if (sdlCursor_) {
        SDL_DestroyCursor(sdlCursor_);
    }
    renderer_.reset();
    if (window_) {
        SDL_DestroyWindow(window_);
    }
}

void SDLWindow::resize(const Size& newSize) {
    size_ = newSize;
    renderer_->resize(
        static_cast<int>(newSize.width),
        static_cast<int>(newSize.height)
    );
}

void SDLWindow::setFullscreen(bool fullscreen) {
    fullscreen_ = fullscreen;
    SDL_SetWindowFullscreen(window_, fullscreen);
}

void SDLWindow::setTitle(const std::string& title) {
    SDL_SetWindowTitle(window_, title.c_str());
}

unsigned int SDLWindow::windowID() const {
    return window_ ? SDL_GetWindowID(window_) : 0;
}

RenderContext* SDLWindow::renderContext() {
    return renderer_ ? renderer_->renderContext() : nullptr;
}

PlatformRenderer* SDLWindow::platformRenderer() {
    return renderer_.get();
}

void SDLWindow::swapBuffers() {
}

float SDLWindow::dpiScaleX() const {
    return window_ ? SDL_GetWindowDisplayScale(window_) : 1.0f;
}

float SDLWindow::dpiScaleY() const {
    return dpiScaleX();
}

Size SDLWindow::currentSize() const {
    return size_;
}

bool SDLWindow::isFullscreen() const {
    return fullscreen_;
}

bool SDLWindow::shouldClose() const {
    return shouldClose_;
}

void SDLWindow::setFluxWindow(Window* window) {
    fluxWindow_ = window;
}

void SDLWindow::setWindowTranslucency(bool /*enabled*/) {
    // No-op for now. Future: macOS NSVisualEffectView / Windows DwmSetWindowAttribute
}


void SDLWindow::setCursor(CursorType cursor) {
    if (cursor == currentCursor_) return;
    currentCursor_ = cursor;

    if (sdlCursor_) {
        SDL_DestroyCursor(sdlCursor_);
        sdlCursor_ = nullptr;
    }

    sdlCursor_ = SDL_CreateSystemCursor(fluxCursorToSDL(cursor));
    if (sdlCursor_) {
        SDL_SetCursor(sdlCursor_);
    }
}

CursorType SDLWindow::currentCursor() const {
    return currentCursor_;
}

// --- Event Processing ---

void SDLWindow::dispatchSDLEvent(const SDL_Event& event) {
    if (event.type == SDL_EVENT_QUIT) {
        for (auto& [id, win] : windowMap_) {
            win->shouldClose_ = true;
        }
        return;
    }

    SDL_WindowID wid = getWindowIDFromEvent(event);
    if (wid != 0) {
        auto it = windowMap_.find(wid);
        if (it != windowMap_.end()) {
            it->second->handleSDLEvent(event);
        }
    } else {
        handleSDLEvent(event);
    }
}

void SDLWindow::processEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        dispatchSDLEvent(event);
    }
}

void SDLWindow::waitForEvents(int timeoutMs) {
    SDL_Event event;
    bool got = (timeoutMs < 0)
        ? SDL_WaitEvent(&event)
        : SDL_WaitEventTimeout(&event, timeoutMs);
    if (got) {
        dispatchSDLEvent(event);
        while (SDL_PollEvent(&event)) {
            dispatchSDLEvent(event);
        }
    }
}

SDL_WindowID SDLWindow::getWindowIDFromEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_WINDOW_RESIZED:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
        case SDL_EVENT_WINDOW_EXPOSED:
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        case SDL_EVENT_WINDOW_FOCUS_LOST:
        case SDL_EVENT_WINDOW_MOVED:
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            return event.window.windowID;
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            return event.key.windowID;
        case SDL_EVENT_TEXT_INPUT:
            return event.text.windowID;
        case SDL_EVENT_MOUSE_MOTION:
            return event.motion.windowID;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            return event.button.windowID;
        case SDL_EVENT_MOUSE_WHEEL:
            return event.wheel.windowID;
        default:
            return 0;
    }
}

void SDLWindow::handleSDLEvent(const SDL_Event& event) {
    if (!fluxWindow_) return;

    switch (event.type) {
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            shouldClose_ = true;
            break;

        case SDL_EVENT_WINDOW_RESIZED:
            // Handled by liveResizeWatcher for immediate response during
            // modal resize loops. Processing stale queued events here would
            // cause renders at old sizes, producing visible stuttering.
            break;

        case SDL_EVENT_WINDOW_EXPOSED:
            fluxWindow_->requestRedraw();
            break;

        case SDL_EVENT_KEY_DOWN: {
            Key k = sdlScancodeToFluxKey(event.key.scancode);
            KeyModifier mods = sdlModToFluxMod(SDL_GetModState());
            fluxWindow_->handleKeyDown(static_cast<int>(k), mods);
            break;
        }

        case SDL_EVENT_KEY_UP: {
            Key k = sdlScancodeToFluxKey(event.key.scancode);
            KeyModifier mods = sdlModToFluxMod(SDL_GetModState());
            fluxWindow_->handleKeyUp(static_cast<int>(k), mods);
            break;
        }

        case SDL_EVENT_TEXT_INPUT:
            fluxWindow_->handleTextInput(event.text.text);
            break;

        case SDL_EVENT_MOUSE_MOTION:
            fluxWindow_->handleMouseMove(event.motion.x, event.motion.y);
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            fluxWindow_->handleMouseDown(
                sdlButtonToFluxButton(event.button.button),
                event.button.x, event.button.y);
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            fluxWindow_->handleMouseUp(
                sdlButtonToFluxButton(event.button.button),
                event.button.x, event.button.y);
            break;

        case SDL_EVENT_MOUSE_WHEEL:
            fluxWindow_->handleMouseScroll(
                event.wheel.mouse_x, event.wheel.mouse_y,
                event.wheel.x, event.wheel.y);
            break;

        default:
            break;
    }
}

// --- Key Mapping ---

Key SDLWindow::sdlScancodeToFluxKey(SDL_Scancode scancode) {
    switch (scancode) {
        case SDL_SCANCODE_A: return Key::A;
        case SDL_SCANCODE_B: return Key::B;
        case SDL_SCANCODE_C: return Key::C;
        case SDL_SCANCODE_D: return Key::D;
        case SDL_SCANCODE_E: return Key::E;
        case SDL_SCANCODE_F: return Key::F;
        case SDL_SCANCODE_G: return Key::G;
        case SDL_SCANCODE_H: return Key::H;
        case SDL_SCANCODE_I: return Key::I;
        case SDL_SCANCODE_J: return Key::J;
        case SDL_SCANCODE_K: return Key::K;
        case SDL_SCANCODE_L: return Key::L;
        case SDL_SCANCODE_M: return Key::M;
        case SDL_SCANCODE_N: return Key::N;
        case SDL_SCANCODE_O: return Key::O;
        case SDL_SCANCODE_P: return Key::P;
        case SDL_SCANCODE_Q: return Key::Q;
        case SDL_SCANCODE_R: return Key::R;
        case SDL_SCANCODE_S: return Key::S;
        case SDL_SCANCODE_T: return Key::T;
        case SDL_SCANCODE_U: return Key::U;
        case SDL_SCANCODE_V: return Key::V;
        case SDL_SCANCODE_W: return Key::W;
        case SDL_SCANCODE_X: return Key::X;
        case SDL_SCANCODE_Y: return Key::Y;
        case SDL_SCANCODE_Z: return Key::Z;

        case SDL_SCANCODE_0: return Key::Num0;
        case SDL_SCANCODE_1: return Key::Num1;
        case SDL_SCANCODE_2: return Key::Num2;
        case SDL_SCANCODE_3: return Key::Num3;
        case SDL_SCANCODE_4: return Key::Num4;
        case SDL_SCANCODE_5: return Key::Num5;
        case SDL_SCANCODE_6: return Key::Num6;
        case SDL_SCANCODE_7: return Key::Num7;
        case SDL_SCANCODE_8: return Key::Num8;
        case SDL_SCANCODE_9: return Key::Num9;

        case SDL_SCANCODE_F1:  return Key::F1;
        case SDL_SCANCODE_F2:  return Key::F2;
        case SDL_SCANCODE_F3:  return Key::F3;
        case SDL_SCANCODE_F4:  return Key::F4;
        case SDL_SCANCODE_F5:  return Key::F5;
        case SDL_SCANCODE_F6:  return Key::F6;
        case SDL_SCANCODE_F7:  return Key::F7;
        case SDL_SCANCODE_F8:  return Key::F8;
        case SDL_SCANCODE_F9:  return Key::F9;
        case SDL_SCANCODE_F10: return Key::F10;
        case SDL_SCANCODE_F11: return Key::F11;
        case SDL_SCANCODE_F12: return Key::F12;

        case SDL_SCANCODE_ESCAPE:    return Key::Escape;
        case SDL_SCANCODE_TAB:       return Key::Tab;
        case SDL_SCANCODE_BACKSPACE: return Key::Backspace;
        case SDL_SCANCODE_RETURN:    return Key::Enter;
        case SDL_SCANCODE_SPACE:     return Key::Space;

        case SDL_SCANCODE_INSERT:   return Key::Insert;
        case SDL_SCANCODE_DELETE:   return Key::Delete;
        case SDL_SCANCODE_HOME:     return Key::Home;
        case SDL_SCANCODE_END:      return Key::End;
        case SDL_SCANCODE_PAGEUP:   return Key::PageUp;
        case SDL_SCANCODE_PAGEDOWN: return Key::PageDown;

        case SDL_SCANCODE_LEFT:  return Key::Left;
        case SDL_SCANCODE_RIGHT: return Key::Right;
        case SDL_SCANCODE_UP:    return Key::Up;
        case SDL_SCANCODE_DOWN:  return Key::Down;

        case SDL_SCANCODE_LSHIFT: return Key::LeftShift;
        case SDL_SCANCODE_RSHIFT: return Key::RightShift;
        case SDL_SCANCODE_LCTRL:  return Key::LeftCtrl;
        case SDL_SCANCODE_RCTRL:  return Key::RightCtrl;
        case SDL_SCANCODE_LALT:   return Key::LeftAlt;
        case SDL_SCANCODE_RALT:   return Key::RightAlt;
        case SDL_SCANCODE_LGUI:   return Key::LeftSuper;
        case SDL_SCANCODE_RGUI:   return Key::RightSuper;

        case SDL_SCANCODE_CAPSLOCK:  return Key::CapsLock;
        case SDL_SCANCODE_NUMLOCKCLEAR: return Key::NumLock;
        case SDL_SCANCODE_SCROLLLOCK: return Key::ScrollLock;

        case SDL_SCANCODE_MINUS:        return Key::Minus;
        case SDL_SCANCODE_EQUALS:       return Key::Equal;
        case SDL_SCANCODE_LEFTBRACKET:  return Key::LeftBracket;
        case SDL_SCANCODE_RIGHTBRACKET: return Key::RightBracket;
        case SDL_SCANCODE_SEMICOLON:    return Key::Semicolon;
        case SDL_SCANCODE_APOSTROPHE:   return Key::Apostrophe;
        case SDL_SCANCODE_GRAVE:        return Key::Grave;
        case SDL_SCANCODE_BACKSLASH:    return Key::Backslash;
        case SDL_SCANCODE_COMMA:        return Key::Comma;
        case SDL_SCANCODE_PERIOD:       return Key::Period;
        case SDL_SCANCODE_SLASH:        return Key::Slash;

        default: return Key::Unknown;
    }
}

KeyModifier SDLWindow::sdlModToFluxMod(SDL_Keymod mod) {
    uint32_t result = 0;
    if (mod & SDL_KMOD_SHIFT) result |= static_cast<uint32_t>(KeyModifier::Shift);
    if (mod & SDL_KMOD_CTRL)  result |= static_cast<uint32_t>(KeyModifier::Ctrl);
    if (mod & SDL_KMOD_ALT)   result |= static_cast<uint32_t>(KeyModifier::Alt);
    if (mod & SDL_KMOD_GUI)   result |= static_cast<uint32_t>(KeyModifier::Super);
    return static_cast<KeyModifier>(result);
}

int SDLWindow::sdlButtonToFluxButton(Uint8 button) {
    switch (button) {
        case SDL_BUTTON_LEFT:   return 0;
        case SDL_BUTTON_RIGHT:  return 1;
        case SDL_BUTTON_MIDDLE: return 2;
        default: return button;
    }
}

bool SDLCALL SDLWindow::liveResizeWatcher(void* userdata, SDL_Event* event) {
    if (event->type == SDL_EVENT_WINDOW_RESIZED) {
        auto* self = static_cast<SDLWindow*>(userdata);
        if (event->window.windowID == SDL_GetWindowID(self->window_) && self->fluxWindow_) {
            self->size_ = {static_cast<float>(event->window.data1),
                           static_cast<float>(event->window.data2)};
            self->fluxWindow_->handleResize(self->size_);
        }
    }
    return true;
}

SDL_SystemCursor SDLWindow::fluxCursorToSDL(CursorType cursor) {
    switch (cursor) {
        case CursorType::Default:      return SDL_SYSTEM_CURSOR_DEFAULT;
        case CursorType::Pointer:      return SDL_SYSTEM_CURSOR_POINTER;
        case CursorType::Text:         return SDL_SYSTEM_CURSOR_TEXT;
        case CursorType::Crosshair:    return SDL_SYSTEM_CURSOR_CROSSHAIR;
        case CursorType::Move:         return SDL_SYSTEM_CURSOR_MOVE;
        case CursorType::ResizeNS:     return SDL_SYSTEM_CURSOR_NS_RESIZE;
        case CursorType::ResizeEW:     return SDL_SYSTEM_CURSOR_EW_RESIZE;
        case CursorType::ResizeNESW:   return SDL_SYSTEM_CURSOR_NESW_RESIZE;
        case CursorType::ResizeNWSE:   return SDL_SYSTEM_CURSOR_NWSE_RESIZE;
        case CursorType::NotAllowed:   return SDL_SYSTEM_CURSOR_NOT_ALLOWED;
        case CursorType::Wait:         return SDL_SYSTEM_CURSOR_WAIT;
        case CursorType::Progress:     return SDL_SYSTEM_CURSOR_PROGRESS;
        case CursorType::Help:         return SDL_SYSTEM_CURSOR_DEFAULT;
        case CursorType::ContextMenu:  return SDL_SYSTEM_CURSOR_DEFAULT;
        case CursorType::Cell:         return SDL_SYSTEM_CURSOR_CROSSHAIR;
        case CursorType::VerticalText: return SDL_SYSTEM_CURSOR_TEXT;
        case CursorType::Alias:        return SDL_SYSTEM_CURSOR_POINTER;
        case CursorType::Copy:         return SDL_SYSTEM_CURSOR_POINTER;
        case CursorType::NoDrop:       return SDL_SYSTEM_CURSOR_NOT_ALLOWED;
        case CursorType::Grab:         return SDL_SYSTEM_CURSOR_POINTER;
        case CursorType::Grabbing:     return SDL_SYSTEM_CURSOR_POINTER;
        case CursorType::AllScroll:    return SDL_SYSTEM_CURSOR_MOVE;
        case CursorType::ZoomIn:       return SDL_SYSTEM_CURSOR_POINTER;
        case CursorType::ZoomOut:      return SDL_SYSTEM_CURSOR_POINTER;
        default:                       return SDL_SYSTEM_CURSOR_DEFAULT;
    }
}

} // namespace flux
