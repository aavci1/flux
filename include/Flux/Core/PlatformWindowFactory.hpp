#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Platform/SDLWindow.hpp>
#include <memory>
#include <string>

namespace flux {

class PlatformWindow;

class PlatformWindowFactory {
public:
    virtual ~PlatformWindowFactory() = default;

    virtual std::unique_ptr<PlatformWindow> createWindow(
        const std::string& title,
        const Size& size,
        bool resizable,
        bool fullscreen
    ) = 0;

    virtual std::string getPlatformName() const = 0;
};

class SDLWindowFactory : public PlatformWindowFactory {
public:
    SDLWindowFactory();
    ~SDLWindowFactory() override;

    std::unique_ptr<PlatformWindow> createWindow(
        const std::string& title,
        const Size& size,
        bool resizable,
        bool fullscreen
    ) override;

    std::string getPlatformName() const override { return "SDL3"; }

    void setRenderBackend(RenderBackendType backend) { backend_ = backend; }
    RenderBackendType renderBackend() const { return backend_; }

private:
    bool sdlInitialized_ = false;
    RenderBackendType backend_ = RenderBackendType::NanoVG;
};

PlatformWindowFactory* getDefaultPlatformFactory();

} // namespace flux
