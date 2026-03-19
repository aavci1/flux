#pragma once

namespace flux {

enum class RenderBackendType {
    NanoVG,
    GPU_Metal,
    GPU_Vulkan,
    GPU_Auto
};

#if defined(__APPLE__)
inline constexpr RenderBackendType kDefaultRenderBackend = RenderBackendType::GPU_Metal;
#elif defined(FLUX_HAS_NANOVG)
inline constexpr RenderBackendType kDefaultRenderBackend = RenderBackendType::NanoVG;
#else
inline constexpr RenderBackendType kDefaultRenderBackend = RenderBackendType::GPU_Vulkan;
#endif

} // namespace flux
