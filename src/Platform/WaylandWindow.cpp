#include <Flux/Platform/WaylandWindow.hpp>
#include <Flux/Platform/NanoVGRenderer.hpp>
#include <Flux/Graphics/NanoVGRenderContext.hpp>
#include <Flux/Core/Window.hpp>

#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <wayland-egl.h>
#include <wayland-cursor.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <iostream>
#include <stdexcept>
#include <cstring>
#include <unistd.h>

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

// Output listener
static void output_geometry(void* data, wl_output* output,
                           int32_t x, int32_t y,
                           int32_t physical_width, int32_t physical_height,
                           int32_t subpixel,
                           const char* make, const char* model,
                           int32_t transform) {
    (void)data;
    (void)output;
    (void)x;
    (void)y;
    (void)physical_width;
    (void)physical_height;
    (void)subpixel;
    (void)make;
    (void)model;
    (void)transform;
}

static void output_mode(void* data, wl_output* output,
                       uint32_t flags, int32_t width, int32_t height,
                       int32_t refresh) {
    (void)data;
    (void)output;
    (void)flags;
    (void)width;
    (void)height;
    (void)refresh;
}

static void output_done(void* data, wl_output* output) {
    (void)data;
    (void)output;
}

static void output_scale(void* data, wl_output* output, int32_t factor) {
    (void)output;
    WaylandWindow* window = static_cast<WaylandWindow*>(data);
    window->updateScale(factor);
}

static void output_name(void* data, wl_output* output, const char* name) {
    (void)output;
    (void)name;
    WaylandWindow* window = static_cast<WaylandWindow*>(data);
    std::cout << "[WaylandWindow] Output name: " << name << std::endl;
}

static void output_description(void* data, wl_output* output, const char* description) {
    (void)output;
    (void)description;
    WaylandWindow* window = static_cast<WaylandWindow*>(data);
    std::cout << "[WaylandWindow] Output description: " << description << std::endl;
}

static const wl_output_listener output_listener = {
    .geometry = output_geometry,
    .mode = output_mode,
    .done = output_done,
    .scale = output_scale,
    .name = output_name,
    .description = output_description,
};

// Surface enter/leave listeners
static void surface_enter(void* data, wl_surface* surface, wl_output* output) {
    (void)surface;
    (void)output;
    WaylandWindow* window = static_cast<WaylandWindow*>(data);
    std::cout << "[WaylandWindow] Surface entered output\n";
    // The scale will be updated via output listener
}

static void surface_leave(void* data, wl_surface* surface, wl_output* output) {
    (void)surface;
    (void)output;
    (void)data;
    std::cout << "[WaylandWindow] Surface left output\n";
}

static void surface_preferred_buffer_scale(void* data, wl_surface* surface, int32_t factor) {
    (void)surface;
    WaylandWindow* window = static_cast<WaylandWindow*>(data);
    std::cout << "[WaylandWindow] Preferred buffer scale: " << factor << std::endl;
    // TODO: Handle buffer scale changes if needed
}

static void surface_preferred_buffer_transform(void* data, wl_surface* surface, uint32_t transform) {
    (void)surface;
    (void)transform;
    (void)data;
    std::cout << "[WaylandWindow] Preferred buffer transform: " << transform << std::endl;
    // TODO: Handle buffer transform changes if needed
}

static const wl_surface_listener surface_listener = {
    .enter = surface_enter,
    .leave = surface_leave,
    .preferred_buffer_scale = surface_preferred_buffer_scale,
    .preferred_buffer_transform = surface_preferred_buffer_transform,
};


// Pointer listener
static void pointer_enter(void* data, wl_pointer* pointer,
                         uint32_t serial, wl_surface* surface,
                         wl_fixed_t surface_x, wl_fixed_t surface_y) {
    (void)pointer;
    (void)surface;
    WaylandWindow* window = static_cast<WaylandWindow*>(data);
    window->handlePointerEnter(serial,
                              wl_fixed_to_double(surface_x),
                              wl_fixed_to_double(surface_y));
}

static void pointer_leave(void* data, wl_pointer* pointer,
                         uint32_t serial, wl_surface* surface) {
    (void)pointer;
    (void)surface;
    WaylandWindow* window = static_cast<WaylandWindow*>(data);
    window->handlePointerLeave(serial);
}

static void pointer_motion(void* data, wl_pointer* pointer,
                          uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
    (void)pointer;
    WaylandWindow* window = static_cast<WaylandWindow*>(data);
    window->handlePointerMotion(time,
                               wl_fixed_to_double(surface_x),
                               wl_fixed_to_double(surface_y));
}

static void pointer_button(void* data, wl_pointer* pointer,
                          uint32_t serial, uint32_t time,
                          uint32_t button, uint32_t state) {
    (void)pointer;
    WaylandWindow* window = static_cast<WaylandWindow*>(data);
    window->handlePointerButton(serial, time, button, state);
}

static void pointer_axis(void* data, wl_pointer* pointer,
                        uint32_t time, uint32_t axis, wl_fixed_t value) {
    (void)data;
    (void)pointer;
    (void)time;
    (void)axis;
    (void)value;
    // TODO: Implement axis (scroll) events
}

static void pointer_frame(void* data, wl_pointer* pointer) {
    (void)data;
    (void)pointer;
    // Frame events are sent at the end of a batch of events
}

static void pointer_axis_source(void* data, wl_pointer* pointer, uint32_t axis_source) {
    (void)data;
    (void)pointer;
    (void)axis_source;
    // Axis source information
}

static void pointer_axis_stop(void* data, wl_pointer* pointer,
                             uint32_t time, uint32_t axis) {
    (void)data;
    (void)pointer;
    (void)time;
    (void)axis;
    // Axis stop events
}

static void pointer_axis_discrete(void* data, wl_pointer* pointer,
                                 uint32_t axis, int32_t discrete) {
    (void)data;
    (void)pointer;
    (void)axis;
    (void)discrete;
    // Discrete axis events (for mouse wheels)
}

static void pointer_axis_value120(void* data, wl_pointer* pointer,
                                 uint32_t axis, int32_t value120) {
    (void)data;
    (void)pointer;
    (void)axis;
    (void)value120;
    // Axis value120 events (high-resolution scrolling)
}

static void pointer_axis_relative_direction(void* data, wl_pointer* pointer,
                                           uint32_t axis, uint32_t direction) {
    (void)data;
    (void)pointer;
    (void)axis;
    (void)direction;
    // Axis relative direction events
}

static const wl_pointer_listener pointer_listener = {
    .enter = pointer_enter,
    .leave = pointer_leave,
    .motion = pointer_motion,
    .button = pointer_button,
    .axis = pointer_axis,
    .frame = pointer_frame,
    .axis_source = pointer_axis_source,
    .axis_stop = pointer_axis_stop,
    .axis_discrete = pointer_axis_discrete,
    .axis_value120 = pointer_axis_value120,
    .axis_relative_direction = pointer_axis_relative_direction,
};

// Keyboard listener
static void keyboard_keymap(void* data, wl_keyboard* keyboard,
                           uint32_t format, int32_t fd, uint32_t size) {
    (void)keyboard;
    (void)format;
    (void)fd;
    (void)size;
    WaylandWindow* window = static_cast<WaylandWindow*>(data);
    window->handleKeyboardKeymap(format, fd, size);
}

static void keyboard_enter(void* data, wl_keyboard* keyboard,
                          uint32_t serial, wl_surface* surface, wl_array* keys) {
    (void)keyboard;
    (void)surface;
    WaylandWindow* window = static_cast<WaylandWindow*>(data);
    window->handleKeyboardEnter(serial, keys);
}

static void keyboard_leave(void* data, wl_keyboard* keyboard,
                          uint32_t serial, wl_surface* surface) {
    (void)keyboard;
    (void)surface;
    WaylandWindow* window = static_cast<WaylandWindow*>(data);
    window->handleKeyboardLeave(serial);
}

static void keyboard_key(void* data, wl_keyboard* keyboard,
                        uint32_t serial, uint32_t time, uint32_t key, uint32_t state) {
    (void)keyboard;
    WaylandWindow* window = static_cast<WaylandWindow*>(data);
    window->handleKeyboardKey(serial, time, key, state);
}

static void keyboard_modifiers(void* data, wl_keyboard* keyboard,
                              uint32_t serial, uint32_t mods_depressed,
                              uint32_t mods_latched, uint32_t mods_locked, uint32_t group) {
    (void)keyboard;
    WaylandWindow* window = static_cast<WaylandWindow*>(data);
    window->handleKeyboardModifiers(serial, mods_depressed, mods_latched, mods_locked, group);
}

static void keyboard_repeat_info(void* data, wl_keyboard* keyboard,
                                int32_t rate, int32_t delay) {
    (void)data;
    (void)keyboard;
    (void)rate;
    (void)delay;
    // Keyboard repeat information
}

static const wl_keyboard_listener keyboard_listener = {
    .keymap = keyboard_keymap,
    .enter = keyboard_enter,
    .leave = keyboard_leave,
    .key = keyboard_key,
    .modifiers = keyboard_modifiers,
    .repeat_info = keyboard_repeat_info,
};

// Seat listener
static void seat_capabilities(void* data, wl_seat* seat, uint32_t capabilities) {
    WaylandWindow* window = static_cast<WaylandWindow*>(data);

    std::cout << "[WaylandWindow] Seat capabilities: " << capabilities << std::endl;

    if (capabilities & WL_SEAT_CAPABILITY_POINTER) {
        if (!window->pointer()) {
            wl_pointer* pointer = wl_seat_get_pointer(seat);
            window->setPointer(pointer);
            wl_pointer_add_listener(pointer, &pointer_listener, window);
            std::cout << "[WaylandWindow] Got pointer\n";
        }
    } else if (window->pointer()) {
        wl_pointer_destroy(window->pointer());
        window->clearPointer();
        std::cout << "[WaylandWindow] Lost pointer\n";
    }

    if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
        if (!window->keyboard()) {
            wl_keyboard* keyboard = wl_seat_get_keyboard(seat);
            window->setKeyboard(keyboard);
            wl_keyboard_add_listener(keyboard, &keyboard_listener, window);
            std::cout << "[WaylandWindow] Got keyboard\n";
        }
    } else if (window->keyboard()) {
        wl_keyboard_destroy(window->keyboard());
        window->clearKeyboard();
        std::cout << "[WaylandWindow] Lost keyboard\n";
    }
}

static void seat_name(void* data, wl_seat* seat, const char* name) {
    (void)data;
    (void)seat;
    std::cout << "[WaylandWindow] Seat name: " << name << std::endl;
}

static const wl_seat_listener seat_listener = {
    .capabilities = seat_capabilities,
    .name = seat_name,
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
        wl_seat_add_listener(window->seat_, &seat_listener, window);
    } else if (strcmp(interface, wl_shm_interface.name) == 0) {
        window->shm_ = static_cast<wl_shm*>(
            wl_registry_bind(registry, name, &wl_shm_interface, 1));
        std::cout << "[WaylandWindow] Bound wl_shm for cursor support\n";
    } else if (strcmp(interface, wl_output_interface.name) == 0) {
        window->output_ = static_cast<wl_output*>(
            wl_registry_bind(registry, name, &wl_output_interface, 2));
        wl_output_add_listener(window->output_, &output_listener, window);
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
    , output_(nullptr)
    , output_scale_(1)
    , pointerEntered_(false)
    , pointerX_(0.0)
    , pointerY_(0.0)
    , pointerButtons_(0)
    , shm_(nullptr)
    , cursor_theme_(nullptr)
    , cursor_surface_(nullptr)
    , currentCursorType_(CursorType::Default)
    , pointerSerial_(0)
    , decoration_manager_(nullptr)
    , toplevel_decoration_(nullptr)
    , egl_display_(nullptr)
    , egl_context_(nullptr)
    , egl_surface_(nullptr)
    , currentSize_(size)
    , isFullscreen_(fullscreen)
    , configured_(false)
    , shouldClose_(false)
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

    if (!initCursors()) {
        std::cerr << "[WaylandWindow] Warning: Failed to initialize cursors\n";
        // Non-fatal, continue without cursor support
    }

    if (!initEGL()) {
        cleanup();
        throw std::runtime_error("Failed to initialize EGL");
    }

    // Create renderer with window dimensions and DPI scale
    renderer_ = std::make_unique<NanoVGRenderer>();
    if (!renderer_->initialize(static_cast<int>(size.width), static_cast<int>(size.height), 
                               dpiScaleX_, dpiScaleY_)) {
        cleanup();
        throw std::runtime_error("Failed to initialize NanoVG renderer");
    }

    std::cout << "[WaylandWindow] Created window \"" << title
              << "\" size: " << size.width << "x" << size.height 
              << ", DPI scale: " << dpiScaleX_ << "x" << dpiScaleY_ << "\n";
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
    
    // Do another roundtrip to ensure we get output scale events
    wl_display_roundtrip(display_);

    // Create surface
    surface_ = wl_compositor_create_surface(compositor_);
    if (!surface_) {
        std::cerr << "[WaylandWindow] Failed to create surface\n";
        return false;
    }
    
    // Set buffer scale if we already know it from the output
    if (output_scale_ > 1) {
        wl_surface_set_buffer_scale(surface_, output_scale_);
        std::cout << "[WaylandWindow] Set initial buffer scale to " << output_scale_ << "\n";
    }
    
    // Add surface listener to track enter/leave events
    wl_surface_add_listener(surface_, &surface_listener, this);

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

    // Choose EGL config with stencil buffer for NanoVG antialiasing
    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 0,
        EGL_STENCIL_SIZE, 8,  // Stencil buffer for NanoVG antialiasing
        EGL_SAMPLE_BUFFERS, 0,  // No MSAA, NanoVG handles antialiasing
        EGL_SAMPLES, 0,
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

    // Create EGL window surface with scaled dimensions
    // The EGL window must be created with the actual buffer size (logical size * scale)
    int bufferWidth = static_cast<int>(currentSize_.width * dpiScaleX_);
    int bufferHeight = static_cast<int>(currentSize_.height * dpiScaleY_);
    
    egl_window_ = wl_egl_window_create(surface_, bufferWidth, bufferHeight);
    if (!egl_window_) {
        std::cerr << "[WaylandWindow] Failed to create EGL window\n";
        return false;
    }
    
    std::cout << "[WaylandWindow] Created EGL window with buffer size " 
              << bufferWidth << "x" << bufferHeight << "\n";

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

    // Set OpenGL viewport to match the buffer size (scaled dimensions)
    glViewport(0, 0, bufferWidth, bufferHeight);
    
    // Configure OpenGL for high-quality rendering
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    
    std::cout << "[WaylandWindow] EGL initialized successfully, viewport: " 
              << bufferWidth << "x" << bufferHeight << "\n";
    return true;
}

void WaylandWindow::cleanup() {
    renderer_.reset();

    // Cleanup cursors
    cleanupCursors();

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

    if (keyboard()) {
        wl_keyboard_destroy(keyboard());
        clearKeyboard();
    }

    if (pointer()) {
        wl_pointer_destroy(pointer());
        clearPointer();
    }

    if (seat_) {
        wl_seat_destroy(seat_);
        seat_ = nullptr;
    }

    if (output_) {
        wl_output_destroy(output_);
        output_ = nullptr;
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

    // Calculate actual buffer size with DPI scaling
    int bufferWidth = static_cast<int>(newSize.width * dpiScaleX_);
    int bufferHeight = static_cast<int>(newSize.height * dpiScaleY_);

    if (egl_window_) {
        wl_egl_window_resize(egl_window_, bufferWidth, bufferHeight, 0, 0);
    }

    // Update OpenGL viewport
    glViewport(0, 0, bufferWidth, bufferHeight);

    if (renderer_) {
        auto* nanoVGRenderer = static_cast<NanoVGRenderer*>(renderer_.get());
        nanoVGRenderer->updateDPIScale(dpiScaleX_, dpiScaleY_);
        renderer_->resize(static_cast<int>(newSize.width), static_cast<int>(newSize.height));

        auto* renderContext = static_cast<NanoVGRenderContext*>(renderer_->renderContext());
        renderContext->updateDPIScale(dpiScaleX_, dpiScaleY_);
    }

    std::cout << "[WaylandWindow] Resized to " << newSize.width << "x" << newSize.height 
              << " (buffer: " << bufferWidth << "x" << bufferHeight << ")\n";
}

void WaylandWindow::handleWaylandResize(int32_t width, int32_t height) {
    if (width > 0 && height > 0) {
        currentSize_.width = static_cast<float>(width);
        currentSize_.height = static_cast<float>(height);
        
        // Calculate actual buffer size with DPI scaling
        int bufferWidth = static_cast<int>(width * dpiScaleX_);
        int bufferHeight = static_cast<int>(height * dpiScaleY_);
        
        if (egl_window_) {
            wl_egl_window_resize(egl_window_, bufferWidth, bufferHeight, 0, 0);
        }

        // Update OpenGL viewport
        glViewport(0, 0, bufferWidth, bufferHeight);

        if (fluxWindow_) {
            fluxWindow_->handleResize(currentSize_);
        }
    }
}

void WaylandWindow::handleClose() {
    // Window close requested by compositor
    shouldClose_ = true;
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

void WaylandWindow::updateScale(int32_t scale) {
    output_scale_ = scale;
    dpiScaleX_ = static_cast<float>(scale);
    dpiScaleY_ = static_cast<float>(scale);
    
    std::cout << "[WaylandWindow] Output scale changed to " << scale << "\n";
    
    // Set the buffer scale on the surface
    if (surface_) {
        wl_surface_set_buffer_scale(surface_, scale);
    }
    
    // Calculate actual buffer size with new DPI scaling
    int bufferWidth = static_cast<int>(currentSize_.width * dpiScaleX_);
    int bufferHeight = static_cast<int>(currentSize_.height * dpiScaleY_);
    
    // Update the EGL window size to match the new scaled buffer
    if (egl_window_) {
        wl_egl_window_resize(egl_window_, bufferWidth, bufferHeight, 0, 0);
    }
    
    // Update OpenGL viewport
    glViewport(0, 0, bufferWidth, bufferHeight);
    
    // Update renderer with new DPI scale
    if (renderer_) {
        auto* nanoVGRenderer = static_cast<NanoVGRenderer*>(renderer_.get());
        nanoVGRenderer->updateDPIScale(dpiScaleX_, dpiScaleY_);
        renderer_->resize(static_cast<int>(currentSize_.width), static_cast<int>(currentSize_.height));
        
        auto* renderContext = static_cast<NanoVGRenderContext*>(renderer_->renderContext());
        renderContext->updateDPIScale(dpiScaleX_, dpiScaleY_);
    }
    
    // Trigger a re-render with the flux window
    if (fluxWindow_) {
        fluxWindow_->handleResize(currentSize_);
    }
    
    std::cout << "[WaylandWindow] Updated to scale " << scale 
              << " buffer size: " << bufferWidth << "x" << bufferHeight << "\n";
}

void WaylandWindow::processEvents() {
    if (display_) {
        // Process pending events without blocking
        while (wl_display_prepare_read(display_) != 0) {
            wl_display_dispatch_pending(display_);
        }
        wl_display_flush(display_);
        wl_display_read_events(display_);
        wl_display_dispatch_pending(display_);
    }
}

bool WaylandWindow::shouldClose() const {
    return shouldClose_;
}

void WaylandWindow::handlePointerEnter(uint32_t serial, double x, double y) {
    pointerSerial_ = serial;  // Save serial for cursor setting
    pointerEntered_ = true;
    pointerX_ = x;
    pointerY_ = y;

    // Apply current cursor when pointer enters
    applyCursor(currentCursorType_);

    if (fluxWindow_) {
        fluxWindow_->handleMouseMove(static_cast<float>(x), static_cast<float>(y));
    }

    std::cout << "[WaylandWindow] Pointer entered at (" << x << ", " << y << ")\n";
}

void WaylandWindow::handlePointerLeave(uint32_t serial) {
    (void)serial;
    pointerEntered_ = false;
    pointerButtons_ = 0;

    std::cout << "[WaylandWindow] Pointer left\n";
}

void WaylandWindow::handlePointerMotion(uint32_t time, double x, double y) {
    (void)time;
    pointerX_ = x;
    pointerY_ = y;

    if (fluxWindow_) {
        fluxWindow_->handleMouseMove(static_cast<float>(x), static_cast<float>(y));
    }

    std::cout << "[WaylandWindow] Pointer moved to (" << x << ", " << y << ")\n";
}

void WaylandWindow::handlePointerButton(uint32_t serial, uint32_t time,
                                       uint32_t button, uint32_t state) {
    (void)serial;
    (void)time;

    // Convert Wayland button codes to Flux button codes
    // Wayland uses Linux input event codes: BTN_LEFT = 0x110, BTN_RIGHT = 0x111, etc.
    int fluxButton = 0;
    if (button == 0x110) { // BTN_LEFT
        fluxButton = 1;
    } else if (button == 0x111) { // BTN_RIGHT
        fluxButton = 2;
    } else if (button == 0x112) { // BTN_MIDDLE
        fluxButton = 3;
    } else {
        fluxButton = static_cast<int>(button);
    }

    if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
        pointerButtons_ |= (1 << (button - 0x110));
        if (fluxWindow_) {
            fluxWindow_->handleMouseDown(fluxButton,
                                       static_cast<float>(pointerX_),
                                       static_cast<float>(pointerY_));
        }
        std::cout << "[WaylandWindow] Button " << fluxButton << " pressed at ("
                  << pointerX_ << ", " << pointerY_ << ")\n";
    } else {
        pointerButtons_ &= ~(1 << (button - 0x110));
        if (fluxWindow_) {
            fluxWindow_->handleMouseUp(fluxButton,
                                     static_cast<float>(pointerX_),
                                     static_cast<float>(pointerY_));
        }
        std::cout << "[WaylandWindow] Button " << fluxButton << " released at ("
                  << pointerX_ << ", " << pointerY_ << ")\n";
    }
}

void WaylandWindow::handleKeyboardKeymap(uint32_t format, int32_t fd, uint32_t size) {
    (void)format;
    (void)size;
    
    // Always close the file descriptor to avoid leaks
    if (fd >= 0) {
        close(fd);
    }
    
    std::cout << "[WaylandWindow] Keyboard keymap received (format: " << format 
              << ", size: " << size << ")\n";
    // TODO: Implement XKB keymap handling for proper keyboard input
}

void WaylandWindow::handleKeyboardEnter(uint32_t serial, const wl_array* keys) {
    (void)serial;
    (void)keys;
    std::cout << "[WaylandWindow] Keyboard focus entered\n";
}

void WaylandWindow::handleKeyboardLeave(uint32_t serial) {
    (void)serial;
    std::cout << "[WaylandWindow] Keyboard focus left\n";
}

void WaylandWindow::handleKeyboardKey(uint32_t serial, uint32_t time,
                                     uint32_t key, uint32_t state) {
    (void)serial;
    (void)time;
    (void)key;
    (void)state;
    // TODO: Implement keyboard key events
    std::cout << "[WaylandWindow] Keyboard key event: " << key << " state: " << state << "\n";
}

void WaylandWindow::handleKeyboardModifiers(uint32_t serial, uint32_t mods_depressed,
                                           uint32_t mods_latched, uint32_t mods_locked,
                                           uint32_t group) {
    (void)serial;
    (void)mods_depressed;
    (void)mods_latched;
    (void)mods_locked;
    (void)group;
    // TODO: Implement keyboard modifier events
    std::cout << "[WaylandWindow] Keyboard modifiers changed\n";
}

// Cursor implementation

bool WaylandWindow::initCursors() {
    if (!shm_) {
        std::cerr << "[WaylandWindow] wl_shm not available, cannot load cursors\n";
        return false;
    }

    // Load cursor theme (size 24 is a reasonable default)
    cursor_theme_ = wl_cursor_theme_load(nullptr, 24, shm_);
    if (!cursor_theme_) {
        std::cerr << "[WaylandWindow] Failed to load cursor theme\n";
        return false;
    }

    // Create cursor surface
    cursor_surface_ = wl_compositor_create_surface(compositor_);
    if (!cursor_surface_) {
        std::cerr << "[WaylandWindow] Failed to create cursor surface\n";
        wl_cursor_theme_destroy(cursor_theme_);
        cursor_theme_ = nullptr;
        return false;
    }

    std::cout << "[WaylandWindow] Cursor support initialized\n";
    return true;
}

void WaylandWindow::cleanupCursors() {
    if (cursor_surface_) {
        wl_surface_destroy(cursor_surface_);
        cursor_surface_ = nullptr;
    }

    if (cursor_theme_) {
        wl_cursor_theme_destroy(cursor_theme_);
        cursor_theme_ = nullptr;
    }
}

const char* WaylandWindow::getCursorName(CursorType type) const {
    switch (type) {
        case CursorType::Default:       return "default";
        case CursorType::Pointer:       return "pointer";
        case CursorType::Text:          return "text";
        case CursorType::Crosshair:     return "crosshair";
        case CursorType::Move:          return "move";
        case CursorType::ResizeNS:      return "ns-resize";
        case CursorType::ResizeEW:      return "ew-resize";
        case CursorType::ResizeNESW:    return "nesw-resize";
        case CursorType::ResizeNWSE:    return "nwse-resize";
        case CursorType::NotAllowed:    return "not-allowed";
        case CursorType::Wait:          return "wait";
        case CursorType::Progress:      return "progress";
        case CursorType::Help:          return "help";
        case CursorType::ContextMenu:   return "context-menu";
        case CursorType::Cell:          return "cell";
        case CursorType::VerticalText:  return "vertical-text";
        case CursorType::Alias:         return "alias";
        case CursorType::Copy:          return "copy";
        case CursorType::NoDrop:        return "no-drop";
        case CursorType::Grab:          return "grab";
        case CursorType::Grabbing:      return "grabbing";
        case CursorType::AllScroll:     return "all-scroll";
        case CursorType::ZoomIn:        return "zoom-in";
        case CursorType::ZoomOut:       return "zoom-out";
        default:                        return "default";
    }
}

void WaylandWindow::applyCursor(CursorType type) {
    if (!cursor_theme_ || !cursor_surface_ || !pointer_) {
        return;
    }

    const char* cursorName = getCursorName(type);
    wl_cursor* cursor = wl_cursor_theme_get_cursor(cursor_theme_, cursorName);
    
    if (!cursor) {
        // Fallback to default cursor if requested cursor not found
        std::cerr << "[WaylandWindow] Cursor '" << cursorName << "' not found, using default\n";
        cursor = wl_cursor_theme_get_cursor(cursor_theme_, "default");
        if (!cursor) {
            std::cerr << "[WaylandWindow] Default cursor not found\n";
            return;
        }
    }

    wl_cursor_image* image = cursor->images[0];
    wl_buffer* buffer = wl_cursor_image_get_buffer(image);
    
    if (!buffer) {
        std::cerr << "[WaylandWindow] Failed to get cursor buffer\n";
        return;
    }

    wl_pointer_set_cursor(pointer_, pointerSerial_, cursor_surface_,
                         image->hotspot_x, image->hotspot_y);
    wl_surface_attach(cursor_surface_, buffer, 0, 0);
    wl_surface_damage(cursor_surface_, 0, 0, image->width, image->height);
    wl_surface_commit(cursor_surface_);
}

void WaylandWindow::setCursor(CursorType cursor) {
    currentCursorType_ = cursor;
    
    // Apply immediately if pointer is in the window
    if (pointerEntered_) {
        applyCursor(cursor);
    }
}

} // namespace flux

