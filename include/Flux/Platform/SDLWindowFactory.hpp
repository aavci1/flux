#pragma once

#include <Flux/Platform/PlatformWindowFactory.hpp>

namespace flux {

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

    void setRenderBackend(RenderBackendType backend) override { backend_ = backend; }
    RenderBackendType renderBackend() const override { return backend_; }

private:
    bool sdlInitialized_ = false;
    RenderBackendType backend_ = RenderBackendType::GPU_Auto;
};

} // namespace flux
