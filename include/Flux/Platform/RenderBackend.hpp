#pragma once

namespace flux {

enum class RenderBackendType {
    NanoVG,
    GPU_Metal,
    GPU_Vulkan,
    GPU_Auto
};

/// Default GPU/render backend for this build — use `PlatformRegistry::instance().defaultRenderBackend()`.
/// Header defaults (e.g. `GPU_Auto`) are resolved by the platform window factory.

} // namespace flux
