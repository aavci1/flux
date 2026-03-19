#include <Flux/Core/PlatformWindowFactory.hpp>
#include <Flux/Platform/SDLWindow.hpp>
#include <SDL3/SDL.h>
#include <stdexcept>
#include <memory>

namespace flux {

SDLWindowFactory::SDLWindowFactory() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        throw std::runtime_error(std::string("Failed to initialize SDL3: ") + SDL_GetError());
    }
    sdlInitialized_ = true;
}

SDLWindowFactory::~SDLWindowFactory() {
    if (sdlInitialized_) {
        SDL_Quit();
    }
}

std::unique_ptr<PlatformWindow> SDLWindowFactory::createWindow(
    const std::string& title,
    const Size& size,
    bool resizable,
    bool fullscreen
) {
    return std::make_unique<SDLWindow>(title, size, resizable, fullscreen, backend_);
}

PlatformWindowFactory* getDefaultPlatformFactory() {
    static SDLWindowFactory factory;
    return &factory;
}

} // namespace flux
