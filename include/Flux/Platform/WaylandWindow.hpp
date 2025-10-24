#pragma once

#include <Flux/Platform/PlatformWindow.hpp>
#include <Flux/Platform/PlatformRenderer.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <string>
#include <memory>
#include <wayland-client-core.h>

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
struct wl_output;
struct zxdg_decoration_manager_v1;
struct zxdg_toplevel_decoration_v1;
struct wl_cursor_theme;
struct wl_cursor;
struct wl_cursor_image;
struct wl_buffer;
struct wl_shm;

// Forward declarations for Wayland listener types
struct wl_pointer_listener;
struct wl_keyboard_listener;
struct wl_seat_listener;

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
    wl_output* output_;
    int32_t output_scale_;
    
    // Keyboard state
    KeyModifier currentModifiers_ = KeyModifier::None;

    // Input state
    bool pointerEntered_;
    double pointerX_;
    double pointerY_;
    uint32_t pointerButtons_;

    // Cursor support
    wl_shm* shm_;
    wl_cursor_theme* cursor_theme_;
    wl_surface* cursor_surface_;
    CursorType currentCursorType_;
    uint32_t pointerSerial_;  // Serial for setting cursor

public:
    // Accessors for static callbacks
    wl_pointer* pointer() const { return pointer_; }
    wl_keyboard* keyboard() const { return keyboard_; }
    void setPointer(wl_pointer* pointer) { pointer_ = pointer; }
    void setKeyboard(wl_keyboard* keyboard) { keyboard_ = keyboard; }
    void clearPointer() { pointer_ = nullptr; }
    void clearKeyboard() { keyboard_ = nullptr; }
    
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
    bool shouldClose_;
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

    void processEvents() override;
    bool shouldClose() const override;
    void setFluxWindow(Window* window) override { fluxWindow_ = window; }

    // Cursor management
    void setCursor(CursorType cursor) override;
    CursorType currentCursor() const override { return currentCursorType_; }

    // Method to get the Flux Window instance
    Window* fluxWindow() const { return fluxWindow_; }

    // Wayland-specific accessors
    wl_display* display() const { return display_; }
    bool isConfigured() const { return configured_; }
    void setConfigured(bool configured) { configured_ = configured; }

    // Window resize from Wayland events
    void handleWaylandResize(int32_t width, int32_t height);
    void handleClose();

    // Input event handling
    void handlePointerEnter(uint32_t serial, double x, double y);
    void handlePointerLeave(uint32_t serial);
    void handlePointerMotion(uint32_t time, double x, double y);
    void handlePointerButton(uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
    void handleKeyboardKeymap(uint32_t format, int32_t fd, uint32_t size);
    void handleKeyboardEnter(uint32_t serial, const wl_array* keys);
    void handleKeyboardLeave(uint32_t serial);
    void handleKeyboardKey(uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    void handleKeyboardModifiers(uint32_t serial, uint32_t mods_depressed,
                                 uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
    void updateModifiers(uint32_t key, bool pressed);
    
    // Scale handling
    void updateScale(int32_t scale);

private:
    // Cursor helper methods
    bool initCursors();
    void cleanupCursors();
    const char* getCursorName(CursorType type) const;
    void applyCursor(CursorType type);
};

} // namespace flux

