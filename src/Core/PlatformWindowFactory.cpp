#include <Flux/Core/PlatformWindowFactory.hpp>
#include <Flux/Platform/PlatformWindow.hpp>
#include <stdexcept>
#include <memory>

#if defined(__APPLE__)
#include <Flux/Platform/MacWindow.hpp>
#else
#include <Flux/Platform/SDLWindow.hpp>
#include <SDL3/SDL.h>
#endif

namespace flux {

#if defined(__APPLE__)

void MacWindowFactory::setRenderBackend(RenderBackendType backend) {
    if (backend == RenderBackendType::NanoVG || backend == RenderBackendType::GPU_Vulkan) {
        throw std::runtime_error("Render backend not available on macOS (use Metal)");
    }
    if (backend == RenderBackendType::GPU_Auto || backend == RenderBackendType::GPU_Metal) {
        backend_ = RenderBackendType::GPU_Metal;
    } else {
        backend_ = backend;
    }
}

std::unique_ptr<PlatformWindow> MacWindowFactory::createWindow(
    const std::string& title,
    const Size& size,
    bool resizable,
    bool fullscreen
) {
    return std::make_unique<MacWindow>(title, size, resizable, fullscreen, backend_);
}

PlatformWindowFactory* getDefaultPlatformFactory() {
    static MacWindowFactory factory;
    return &factory;
}

#else

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

#endif

} // namespace flux
