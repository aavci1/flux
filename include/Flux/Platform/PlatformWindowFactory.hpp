#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Platform/RenderBackend.hpp>
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

    /// SDL / non-macOS only (ignored on macOS native factory).
    virtual void setRenderBackend(RenderBackendType backend) { (void)backend; }
    virtual RenderBackendType renderBackend() const { return RenderBackendType::GPU_Auto; }
};

} // namespace flux
