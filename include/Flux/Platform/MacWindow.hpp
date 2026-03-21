#pragma once

#include <Flux/Platform/PlatformWindow.hpp>
#include <Flux/Platform/PlatformRenderer.hpp>
#include <Flux/Platform/RenderBackend.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <memory>
#include <string>

namespace flux {

class Window;

struct MacWindowImpl;

/**
 * AppKit-backed PlatformWindow (macOS only): NSWindow + NSView with CAMetalLayer, Metal renderer.
 */
class MacWindow : public PlatformWindow {
public:
    MacWindow(const std::string& title, const Size& size, bool resizable, bool fullscreen,
              RenderBackendType backend = RenderBackendType::GPU_Auto);
    ~MacWindow() override;

    void resize(const Size& newSize) override;
    void setFullscreen(bool fullscreen) override;
    void setTitle(const std::string& title) override;
    unsigned int windowID() const override;

    RenderContext* renderContext() override;
    PlatformRenderer* platformRenderer() override;
    void swapBuffers() override;

    float dpiScaleX() const override;
    float dpiScaleY() const override;

    Size currentSize() const override;
    bool isFullscreen() const override;

    void processEvents() override;
    void waitForEvents(int timeoutMs = -1) override;
    bool shouldClose() const override;

    void setCursor(CursorType cursor) override;
    CursorType currentCursor() const override;

    void setFluxWindow(Window* window) override;
    void setGpuReadbackEnabled(bool enabled) override;

    /// NSView* for `NativeGraphicsSurface::fromAppleView` (Metal layer host).
    void* metalContentView() const;

    /// Used by AppKit view callbacks (MacWindow.mm).
    Window* eventTarget() const { return fluxWindow_; }

    void handleFramebufferResize();

    /// Invoked from `FluxWindowDelegate` when the user clicks the close button.
    void notifyCloseRequested();

private:
    std::unique_ptr<MacWindowImpl> impl_;
    std::unique_ptr<PlatformRenderer> renderer_;

    Size size_{};
    bool fullscreen_ = false;
    bool shouldClose_ = false;
    CursorType currentCursor_ = CursorType::Default;
    Window* fluxWindow_ = nullptr;
    RenderBackendType backendType_ = RenderBackendType::GPU_Auto;
};

} // namespace flux
