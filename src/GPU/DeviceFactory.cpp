#include <Flux/GPU/Device.hpp>
#include <stdexcept>

namespace flux::gpu {

#ifdef __APPLE__
std::unique_ptr<Device> createMetalDevice(void* nsView);
#endif

#if defined(FLUX_HAS_VULKAN)
std::unique_ptr<Device> createVulkanDevice(void* sdlWindow);
#endif

std::unique_ptr<Device> createDevice(Backend backend, NativeGraphicsSurface surface) {
    switch (backend) {
#ifdef __APPLE__
        case Backend::Metal:
            if (surface.kind != NativeGraphicsSurfaceKind::AppleNsView || !surface.ptr) {
                throw std::runtime_error("Metal backend requires NSView graphics surface");
            }
            return createMetalDevice(surface.ptr);
#endif
#if defined(FLUX_HAS_VULKAN)
        case Backend::Vulkan:
            if (surface.kind != NativeGraphicsSurfaceKind::SdlWindow || !surface.ptr) {
                throw std::runtime_error("Vulkan backend requires SDL_Window graphics surface");
            }
            return createVulkanDevice(surface.ptr);
#endif
        default:
            throw std::runtime_error("Requested GPU backend not available on this platform");
    }
}

} // namespace flux::gpu
