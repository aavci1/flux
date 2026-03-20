#pragma once

#include <Flux/Platform/PlatformWindowFactory.hpp>

namespace flux {

class MacWindowFactory : public PlatformWindowFactory {
public:
    std::unique_ptr<PlatformWindow> createWindow(
        const std::string& title,
        const Size& size,
        bool resizable,
        bool fullscreen
    ) override;

    std::string getPlatformName() const override { return "Apple"; }

    void setRenderBackend(RenderBackendType backend) override;
    RenderBackendType renderBackend() const override { return backend_; }

private:
    RenderBackendType backend_ = RenderBackendType::GPU_Metal;
};

} // namespace flux
