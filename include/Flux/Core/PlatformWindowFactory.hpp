#pragma once

#include <Flux/Core/Types.hpp>
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

private:
    bool sdlInitialized_ = false;
};

PlatformWindowFactory* getDefaultPlatformFactory();

} // namespace flux
