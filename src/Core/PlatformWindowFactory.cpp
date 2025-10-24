#include <Flux/Core/PlatformWindowFactory.hpp>
#include <Flux/Platform/WaylandWindow.hpp>
#include <memory>

namespace flux {

std::unique_ptr<PlatformWindow> WaylandWindowFactory::createWindow(
    const std::string& title,
    const Size& size,
    bool resizable,
    bool fullscreen
) {
    return std::make_unique<WaylandWindow>(title, size, resizable, fullscreen);
}

// Default factory (Wayland for now, could auto-detect platform in future)
PlatformWindowFactory* getDefaultPlatformFactory() {
    static WaylandWindowFactory factory;
    return &factory;
}

} // namespace flux

