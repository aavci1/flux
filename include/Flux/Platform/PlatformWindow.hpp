#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <string>
#include <memory>

namespace flux {

/**
 * Abstract base class for platform-specific window implementations.
 * This allows the Window class to work with any windowing system
 * without being tightly coupled to specific implementations.
 */
class PlatformWindow {
public:
    virtual ~PlatformWindow() = default;

    // Window management
    virtual void resize(const Size& newSize) = 0;
    virtual void setFullscreen(bool fullscreen) = 0;
    virtual void setTitle(const std::string& title) = 0;
    virtual unsigned int windowID() const = 0;

    // Render context access
    virtual RenderContext* renderContext() = 0;

    // Platform-specific methods
    virtual void swapBuffers() = 0;

    // DPI scaling
    virtual float dpiScaleX() const = 0;
    virtual float dpiScaleY() const = 0;

    // Current window state
    virtual Size currentSize() const = 0;
    virtual bool isFullscreen() const = 0;
};

} // namespace flux
