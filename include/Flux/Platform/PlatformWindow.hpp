#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <string>
#include <memory>

namespace flux {

class Window;
class PlatformRenderer;

class PlatformWindow {
public:
    virtual ~PlatformWindow() = default;

    virtual void resize(const Size& newSize) = 0;
    virtual void setFullscreen(bool fullscreen) = 0;
    virtual void setTitle(const std::string& title) = 0;
    virtual unsigned int windowID() const = 0;

    virtual RenderContext* renderContext() = 0;
    virtual PlatformRenderer* platformRenderer() = 0;

    virtual void swapBuffers() = 0;

    virtual float dpiScaleX() const = 0;
    virtual float dpiScaleY() const = 0;

    virtual Size currentSize() const = 0;
    virtual bool isFullscreen() const = 0;

    virtual void processEvents() = 0;
    virtual void waitForEvents(int timeoutMs = -1) = 0;
    virtual bool shouldClose() const = 0;

    virtual void setCursor(CursorType cursor) = 0;
    virtual CursorType currentCursor() const = 0;

    virtual void setFluxWindow(Window* window) = 0;
};

} // namespace flux
