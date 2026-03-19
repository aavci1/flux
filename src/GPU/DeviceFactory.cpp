#include <Flux/GPU/Device.hpp>
#include <stdexcept>

namespace flux::gpu {

#ifdef __APPLE__
std::unique_ptr<Device> createMetalDevice(SDL_Window* window);
#endif

#if defined(FLUX_HAS_VULKAN)
std::unique_ptr<Device> createVulkanDevice(SDL_Window* window);
#endif

std::unique_ptr<Device> createDevice(Backend backend, SDL_Window* window) {
    switch (backend) {
#ifdef __APPLE__
        case Backend::Metal:
            return createMetalDevice(window);
#endif
#if defined(FLUX_HAS_VULKAN)
        case Backend::Vulkan:
            return createVulkanDevice(window);
#endif
        default:
            throw std::runtime_error("Requested GPU backend not available on this platform");
    }
}

} // namespace flux::gpu
