#pragma once

#include <Flux/Platform/PlatformWindow.hpp>
#include <Flux/Platform/PlatformRenderer.hpp>
#include <Flux/Platform/RenderBackend.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <SDL3/SDL.h>
#include <string>
#include <memory>
#include <unordered_map>

namespace flux {

class Window;

class SDLWindow : public PlatformWindow {
public:
    SDLWindow(const std::string& title, const Size& size, bool resizable, bool fullscreen,
              RenderBackendType backend = RenderBackendType::GPU_Auto);
    ~SDLWindow() override;

    void resize(const Size& newSize) override;
    void setFullscreen(bool fullscreen) override;
    void setTitle(const std::string& title) override;
    unsigned int windowID() const override;

    RenderContext* renderContext() override;
    PlatformRenderer* platformRenderer() override;
    void swapBuffers() override;

    float dpiScaleX() const override;
    float dpiScaleY() const override;

    Size currentSize() const override;
    bool isFullscreen() const override;

    void processEvents() override;
    void waitForEvents(int timeoutMs = -1) override;
    bool shouldClose() const override;

    void setCursor(CursorType cursor) override;
    CursorType currentCursor() const override;

    void setFluxWindow(Window* window) override;

    void setWindowTranslucency(bool enabled);

private:
    SDL_Window* window_ = nullptr;
    std::unique_ptr<PlatformRenderer> renderer_;
    Window* fluxWindow_ = nullptr;
    RenderBackendType backendType_ = RenderBackendType::GPU_Auto;

    Size size_;
    bool fullscreen_ = false;
    bool shouldClose_ = false;
    CursorType currentCursor_ = CursorType::Default;
    SDL_Cursor* sdlCursor_ = nullptr;

    static std::unordered_map<SDL_WindowID, SDLWindow*> windowMap_;

    void handleSDLEvent(const SDL_Event& event);
    void dispatchSDLEvent(const SDL_Event& event);
    static SDL_WindowID getWindowIDFromEvent(const SDL_Event& event);
    static Key sdlScancodeToFluxKey(SDL_Scancode scancode);
    static KeyModifier sdlModToFluxMod(SDL_Keymod mod);
    static int sdlButtonToFluxButton(Uint8 button);
    static SDL_SystemCursor fluxCursorToSDL(CursorType cursor);

    static bool SDLCALL liveResizeWatcher(void* userdata, SDL_Event* event);
};

} // namespace flux
