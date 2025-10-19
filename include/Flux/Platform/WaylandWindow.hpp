#pragma once

#include <Flux/Platform/PlatformWindow.hpp>
#include <Flux/Platform/PlatformRenderer.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <string>
#include <memory>

// Forward declarations for Wayland types
struct wl_display;
struct wl_registry;
struct wl_compositor;
struct wl_surface;
struct xdg_wm_base;
struct xdg_surface;
struct xdg_toplevel;
struct wl_egl_window;
struct wl_seat;
struct wl_pointer;
struct wl_keyboard;
struct zxdg_decoration_manager_v1;
struct zxdg_toplevel_decoration_v1;

// Forward declarations for EGL types
typedef void *EGLDisplay;
typedef void *EGLContext;
typedef void *EGLSurface;

namespace flux {

// Forward declaration
class Window;

// Friend declarations for Wayland listener callbacks
static void registry_global(void* data, wl_registry* registry,
                           uint32_t name, const char* interface,
                           uint32_t version);

class WaylandWindow : public PlatformWindow {
    // Friend declarations for listener callbacks
    friend void registry_global(void* data, wl_registry* registry,
                               uint32_t name, const char* interface,
                               uint32_t version);

private:
    // Wayland objects
    wl_display* display_;
    wl_registry* registry_;
    wl_compositor* compositor_;
    wl_surface* surface_;
    xdg_wm_base* wm_base_;
    xdg_surface* xdg_surface_;
    xdg_toplevel* xdg_toplevel_;
    wl_egl_window* egl_window_;
    wl_seat* seat_;
    wl_pointer* pointer_;
    wl_keyboard* keyboard_;
    
    // Decoration objects
    zxdg_decoration_manager_v1* decoration_manager_;
    zxdg_toplevel_decoration_v1* toplevel_decoration_;
    
    // EGL objects
    EGLDisplay egl_display_;
    EGLContext egl_context_;
    EGLSurface egl_surface_;
    
    // Renderer
    std::unique_ptr<PlatformRenderer> renderer_;
    
    // Window state
    Size currentSize_;
    bool isFullscreen_;
    bool configured_;
    float dpiScaleX_;
    float dpiScaleY_;
    Window* fluxWindow_;
    
    // Static initialization counter
    static int windowCount_;
    
    // Helper methods
    bool initWayland(const std::string& title, const Size& size);
    bool initEGL();
    void cleanup();

public:
    WaylandWindow(const std::string& title, const Size& size, bool resizable, bool fullscreen);
    ~WaylandWindow() override;

    // PlatformWindow interface
    void resize(const Size& newSize) override;
    void setFullscreen(bool fullscreen) override;
    void setTitle(const std::string& title) override;
    unsigned int windowID() const override;

    RenderContext* renderContext() override;
    void swapBuffers() override;

    float dpiScaleX() const override { return dpiScaleX_; }
    float dpiScaleY() const override { return dpiScaleY_; }

    Size currentSize() const override { return currentSize_; }
    bool isFullscreen() const override { return isFullscreen_; }

    // Method to get the Flux Window instance
    Window* fluxWindow() const { return fluxWindow_; }
    void setFluxWindow(Window* window) { fluxWindow_ = window; }

    // Wayland-specific accessors
    wl_display* display() const { return display_; }
    bool isConfigured() const { return configured_; }
    void setConfigured(bool configured) { configured_ = configured; }
    
    // Window resize from Wayland events
    void handleWaylandResize(int32_t width, int32_t height);
    void handleClose();
};

} // namespace flux

