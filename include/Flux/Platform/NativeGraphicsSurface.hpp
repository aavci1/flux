#pragma once

#include <cstdint>

namespace flux::gpu {

enum class NativeGraphicsSurfaceKind : uint8_t {
    None = 0,
    /// Non-macOS: `SDL_Window*` for Vulkan (or legacy SDL Metal — not used in current matrix).
    SdlWindow,
    /// macOS: `NSView*` hosting `CAMetalLayer` (plain pointer; cast in `.mm`).
    AppleNsView,
};

/**
 * Opaque GPU swapchain / layer host for `createDevice`.
 * - macOS + Metal: `kind == AppleNsView`, `ptr` is NSView*.
 * - Linux/Windows + Vulkan: `kind == SdlWindow`, `ptr` is SDL_Window*.
 */
struct NativeGraphicsSurface {
    NativeGraphicsSurfaceKind kind = NativeGraphicsSurfaceKind::None;
    void* ptr = nullptr;

    static NativeGraphicsSurface fromSdlWindow(void* sdlWindow) {
        return {NativeGraphicsSurfaceKind::SdlWindow, sdlWindow};
    }

    static NativeGraphicsSurface fromAppleView(void* nsView) {
        return {NativeGraphicsSurfaceKind::AppleNsView, nsView};
    }
};

} // namespace flux::gpu
