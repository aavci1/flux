#include <Flux/Platform/WaylandWindow.hpp>
#include <Flux/Platform/NanoVGRenderer.hpp>
#include <Flux/Graphics/NanoVGRenderContext.hpp>
#include <Flux/Core/Window.hpp>

#include <wayland-client.h>
#include <wayland-egl.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <iostream>
#include <stdexcept>
#include <cstring>

// Include the generated xdg-shell protocol header
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-client-protocol.h"

namespace flux {

int WaylandWindow::windowCount_ = 0;

// XDG WM Base listener
static void xdg_wm_base_ping(void* data, xdg_wm_base* wm_base, uint32_t serial) {
    xdg_wm_base_pong(wm_base, serial);
}

static const xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

// XDG Surface listener
static void xdg_surface_configure(void* data, xdg_surface* surf, uint32_t serial) {
    WaylandWindow* window = static_cast<WaylandWindow*>(data);
    xdg_surface_ack_configure(surf, serial);
    window->setConfigured(true);
}

static const xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

// XDG Toplevel listener
static void xdg_toplevel_configure(void* data, xdg_toplevel* top,
                                   int32_t width, int32_t height,
                                   wl_array* states) {
    (void)top;
    (void)states;
    WaylandWindow* window = static_cast<WaylandWindow*>(data);
    if (width > 0 && height > 0) {
        window->handleWaylandResize(width, height);
    }
}

static void xdg_toplevel_close(void* data, xdg_toplevel* top) {
    (void)top;
    WaylandWindow* window = static_cast<WaylandWindow*>(data);
    window->handleClose();
}

static const xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_configure,
    .close = xdg_toplevel_close,
};

// Registry listener
static void registry_global(void* data, wl_registry* registry,
                           uint32_t name, const char* interface,
                           uint32_t version) {
    WaylandWindow* window = static_cast<WaylandWindow*>(data);
    
    std::cout << "[WaylandWindow] Registry: " << interface << " v" << version << std::endl;
    
    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        window->compositor_ = static_cast<wl_compositor*>(
            wl_registry_bind(registry, name, &wl_compositor_interface, 4));
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
        window->wm_base_ = static_cast<xdg_wm_base*>(
            wl_registry_bind(registry, name, &xdg_wm_base_interface, 1));
        xdg_wm_base_add_listener(window->wm_base_, &xdg_wm_base_listener, window);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
        window->seat_ = static_cast<wl_seat*>(
            wl_registry_bind(registry, name, &wl_seat_interface, 5));
    } else if (strcmp(interface, zxdg_decoration_manager_v1_interface.name) == 0) {
        window->decoration_manager_ = static_cast<zxdg_decoration_manager_v1*>(
            wl_registry_bind(registry, name, &zxdg_decoration_manager_v1_interface, 1));
    }
}

static void registry_global_remove(void* data, wl_registry* registry, uint32_t name) {
    (void)data;
    (void)registry;
    (void)name;
    // Handle global object removal if needed
}

static const wl_registry_listener registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove,
};

WaylandWindow::WaylandWindow(const std::string& title, const Size& size, bool resizable, bool fullscreen)
    : display_(nullptr)
    , registry_(nullptr)
    , compositor_(nullptr)
    , surface_(nullptr)
    , wm_base_(nullptr)
    , xdg_surface_(nullptr)
    , xdg_toplevel_(nullptr)
    , egl_window_(nullptr)
    , seat_(nullptr)
    , pointer_(nullptr)
    , keyboard_(nullptr)
    , decoration_manager_(nullptr)
    , toplevel_decoration_(nullptr)
    , egl_display_(nullptr)
    , egl_context_(nullptr)
    , egl_surface_(nullptr)
    , currentSize_(size)
    , isFullscreen_(fullscreen)
    , configured_(false)
    , dpiScaleX_(1.0f)
    , dpiScaleY_(1.0f)
    , fluxWindow_(nullptr) {

    (void)resizable; // Wayland doesn't use this flag in the same way

    if (windowCount_ == 0) {
        std::cout << "[WaylandWindow] Initializing Wayland backend\n";
    }
    windowCount_++;

    if (!initWayland(title, size)) {
        throw std::runtime_error("Failed to initialize Wayland window");
    }

    if (!initEGL()) {
        cleanup();
        throw std::runtime_error("Failed to initialize EGL");
    }

    // Create renderer with window dimensions
    renderer_ = std::make_unique<NanoVGRenderer>();
    if (!renderer_->initialize(static_cast<int>(size.width), static_cast<int>(size.height), 
                               dpiScaleX_, dpiScaleY_)) {
        cleanup();
        throw std::runtime_error("Failed to initialize NanoVG renderer");
    }

    std::cout << "[WaylandWindow] Created window \"" << title
              << "\" size: " << size.width << "x" << size.height << "\n";
}

WaylandWindow::~WaylandWindow() {
    std::cout << "[WaylandWindow] Destroying window\n";
    cleanup();

    windowCount_--;
    if (windowCount_ == 0) {
        std::cout << "[WaylandWindow] Last window destroyed\n";
    }
}

bool WaylandWindow::initWayland(const std::string& title, const Size& size) {
    // Connect to Wayland display
    display_ = wl_display_connect(nullptr);
    if (!display_) {
        std::cerr << "[WaylandWindow] Failed to connect to Wayland display\n";
        return false;
    }

    std::cout << "[WaylandWindow] Connected to Wayland display\n";

    // Get registry
    registry_ = wl_display_get_registry(display_);
    wl_registry_add_listener(registry_, &registry_listener, this);

    // Roundtrip to get all globals
    wl_display_roundtrip(display_);

    if (!compositor_ || !wm_base_) {
        std::cerr << "[WaylandWindow] Failed to bind required Wayland globals\n";
        return false;
    }

    // Create surface
    surface_ = wl_compositor_create_surface(compositor_);
    if (!surface_) {
        std::cerr << "[WaylandWindow] Failed to create surface\n";
        return false;
    }

    // Create xdg_surface
    xdg_surface_ = xdg_wm_base_get_xdg_surface(wm_base_, surface_);
    xdg_surface_add_listener(xdg_surface_, &xdg_surface_listener, this);

    // Create xdg_toplevel
    xdg_toplevel_ = xdg_surface_get_toplevel(xdg_surface_);
    xdg_toplevel_add_listener(xdg_toplevel_, &xdg_toplevel_listener, this);
    xdg_toplevel_set_title(xdg_toplevel_, title.c_str());

    // Request server-side decorations if available
    if (decoration_manager_) {
        toplevel_decoration_ = zxdg_decoration_manager_v1_get_toplevel_decoration(
            decoration_manager_, xdg_toplevel_);
        if (toplevel_decoration_) {
            zxdg_toplevel_decoration_v1_set_mode(toplevel_decoration_,
                ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
            std::cout << "[WaylandWindow] Requested server-side decorations\n";
        }
    } else {
        std::cout << "[WaylandWindow] Decoration manager not available, windows will have no decorations\n";
    }

    // Commit surface
    wl_surface_commit(surface_);

    // Wait for configure event
    while (!configured_ && wl_display_dispatch(display_) != -1) {
        // Wait for configuration
    }

    return true;
}

bool WaylandWindow::initEGL() {
    egl_display_ = eglGetDisplay((EGLNativeDisplayType)display_);
    if (egl_display_ == EGL_NO_DISPLAY) {
        std::cerr << "[WaylandWindow] Failed to get EGL display\n";
        return false;
    }

    EGLint major, minor;
    if (!eglInitialize(egl_display_, &major, &minor)) {
        std::cerr << "[WaylandWindow] Failed to initialize EGL\n";
        return false;
    }

    std::cout << "[WaylandWindow] EGL version: " << major << "." << minor << "\n";

    // Choose EGL config
    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_NONE
    };

    EGLConfig config;
    EGLint num_configs;
    if (!eglChooseConfig(egl_display_, config_attribs, &config, 1, &num_configs)) {
        std::cerr << "[WaylandWindow] Failed to choose EGL config\n";
        return false;
    }

    // Bind OpenGL ES API
    if (!eglBindAPI(EGL_OPENGL_ES_API)) {
        std::cerr << "[WaylandWindow] Failed to bind OpenGL ES API\n";
        return false;
    }

    // Create EGL context
    EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    egl_context_ = eglCreateContext(egl_display_, config, EGL_NO_CONTEXT, context_attribs);
    if (egl_context_ == EGL_NO_CONTEXT) {
        std::cerr << "[WaylandWindow] Failed to create EGL context\n";
        return false;
    }

    // Create EGL window surface
    egl_window_ = wl_egl_window_create(surface_, 
                                       static_cast<int>(currentSize_.width), 
                                       static_cast<int>(currentSize_.height));
    if (!egl_window_) {
        std::cerr << "[WaylandWindow] Failed to create EGL window\n";
        return false;
    }

    egl_surface_ = eglCreateWindowSurface(egl_display_, config,
                                         (EGLNativeWindowType)egl_window_, nullptr);
    if (egl_surface_ == EGL_NO_SURFACE) {
        std::cerr << "[WaylandWindow] Failed to create EGL surface\n";
        return false;
    }

    // Make context current
    if (!eglMakeCurrent(egl_display_, egl_surface_, egl_surface_, egl_context_)) {
        std::cerr << "[WaylandWindow] Failed to make EGL context current\n";
        return false;
    }

    std::cout << "[WaylandWindow] EGL initialized successfully\n";
    return true;
}

void WaylandWindow::cleanup() {
    renderer_.reset();

    if (egl_display_ != EGL_NO_DISPLAY) {
        eglMakeCurrent(egl_display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        
        if (egl_context_ != EGL_NO_CONTEXT) {
            eglDestroyContext(egl_display_, egl_context_);
            egl_context_ = EGL_NO_CONTEXT;
        }
        if (egl_surface_ != EGL_NO_SURFACE) {
            eglDestroySurface(egl_display_, egl_surface_);
            egl_surface_ = EGL_NO_SURFACE;
        }
        eglTerminate(egl_display_);
        egl_display_ = EGL_NO_DISPLAY;
    }

    if (egl_window_) {
        wl_egl_window_destroy(egl_window_);
        egl_window_ = nullptr;
    }

    if (toplevel_decoration_) {
        zxdg_toplevel_decoration_v1_destroy(toplevel_decoration_);
        toplevel_decoration_ = nullptr;
    }

    if (xdg_toplevel_) {
        xdg_toplevel_destroy(xdg_toplevel_);
        xdg_toplevel_ = nullptr;
    }

    if (xdg_surface_) {
        xdg_surface_destroy(xdg_surface_);
        xdg_surface_ = nullptr;
    }

    if (surface_) {
        wl_surface_destroy(surface_);
        surface_ = nullptr;
    }

    if (decoration_manager_) {
        zxdg_decoration_manager_v1_destroy(decoration_manager_);
        decoration_manager_ = nullptr;
    }

    if (wm_base_) {
        xdg_wm_base_destroy(wm_base_);
        wm_base_ = nullptr;
    }

    if (compositor_) {
        wl_compositor_destroy(compositor_);
        compositor_ = nullptr;
    }

    if (seat_) {
        wl_seat_destroy(seat_);
        seat_ = nullptr;
    }

    if (registry_) {
        wl_registry_destroy(registry_);
        registry_ = nullptr;
    }

    if (display_) {
        wl_display_disconnect(display_);
        display_ = nullptr;
    }
}

void WaylandWindow::resize(const Size& newSize) {
    currentSize_ = newSize;

    if (egl_window_) {
        wl_egl_window_resize(egl_window_, 
                           static_cast<int>(newSize.width), 
                           static_cast<int>(newSize.height), 
                           0, 0);
    }

    if (renderer_) {
        auto* nanoVGRenderer = static_cast<NanoVGRenderer*>(renderer_.get());
        nanoVGRenderer->updateDPIScale(dpiScaleX_, dpiScaleY_);
        renderer_->resize(static_cast<int>(newSize.width), static_cast<int>(newSize.height));

        auto* renderContext = static_cast<NanoVGRenderContext*>(renderer_->renderContext());
        renderContext->updateDPIScale(dpiScaleX_, dpiScaleY_);
    }

    std::cout << "[WaylandWindow] Resized to " << newSize.width << "x" << newSize.height << "\n";
}

void WaylandWindow::handleWaylandResize(int32_t width, int32_t height) {
    if (width > 0 && height > 0) {
        currentSize_.width = static_cast<float>(width);
        currentSize_.height = static_cast<float>(height);
        
        if (egl_window_) {
            wl_egl_window_resize(egl_window_, width, height, 0, 0);
        }

        if (fluxWindow_) {
            fluxWindow_->handleResize(currentSize_);
        }
    }
}

void WaylandWindow::handleClose() {
    // Window close requested by compositor
    // The application will handle this in processEvents
}

void WaylandWindow::setFullscreen(bool fullscreen) {
    isFullscreen_ = fullscreen;

    if (xdg_toplevel_) {
        if (fullscreen) {
            xdg_toplevel_set_fullscreen(xdg_toplevel_, nullptr);
        } else {
            xdg_toplevel_unset_fullscreen(xdg_toplevel_);
        }
    }

    std::cout << "[WaylandWindow] Fullscreen: " << (fullscreen ? "ON" : "OFF") << "\n";
}

void WaylandWindow::setTitle(const std::string& title) {
    if (xdg_toplevel_) {
        xdg_toplevel_set_title(xdg_toplevel_, title.c_str());
    }
    std::cout << "[WaylandWindow] Title changed to \"" << title << "\"\n";
}

unsigned int WaylandWindow::windowID() const {
    return static_cast<unsigned int>(reinterpret_cast<uintptr_t>(surface_));
}

RenderContext* WaylandWindow::renderContext() {
    return renderer_ ? renderer_->renderContext() : nullptr;
}

void WaylandWindow::swapBuffers() {
    if (egl_display_ != EGL_NO_DISPLAY && egl_surface_ != EGL_NO_SURFACE) {
        eglSwapBuffers(egl_display_, egl_surface_);
    }
}

} // namespace flux

