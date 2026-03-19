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
    virtual RenderBackendType renderBackend() const { return kDefaultRenderBackend; }
};

#if defined(__APPLE__)

class MacWindowFactory : public PlatformWindowFactory {
public:
    std::unique_ptr<PlatformWindow> createWindow(
        const std::string& title,
        const Size& size,
        bool resizable,
        bool fullscreen
    ) override;

    std::string getPlatformName() const override { return "Cocoa"; }

    void setRenderBackend(RenderBackendType backend) override;
    RenderBackendType renderBackend() const override { return backend_; }

private:
    RenderBackendType backend_ = kDefaultRenderBackend;
};

#else

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
    RenderBackendType backend_ = kDefaultRenderBackend;
};

#endif

PlatformWindowFactory* getDefaultPlatformFactory();

} // namespace flux
