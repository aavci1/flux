#include <Flux/Platform/MacWindowFactory.hpp>
#include <Flux/Platform/MacWindow.hpp>
#include <stdexcept>

namespace flux {

void MacWindowFactory::setRenderBackend(RenderBackendType backend) {
    if (backend == RenderBackendType::GPU_Vulkan) {
        throw std::runtime_error("Vulkan backend not available on macOS (use Metal)");
    }
    if (backend == RenderBackendType::GPU_Auto || backend == RenderBackendType::GPU_Metal) {
        backend_ = RenderBackendType::GPU_Metal;
    } else {
        backend_ = backend;
    }
}

std::unique_ptr<PlatformWindow> MacWindowFactory::createWindow(
    const std::string& title,
    const Size& size,
    bool resizable,
    bool fullscreen
) {
    return std::make_unique<MacWindow>(title, size, resizable, fullscreen, backend_);
}

} // namespace flux
