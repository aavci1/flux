#include <Flux/Platform/EventLoopWake.hpp>
#include <Flux/Platform/PlatformRegistry.hpp>

namespace flux {

void wakePlatformEventLoop() {
    if (auto* w = PlatformRegistry::instance().eventLoopWake()) {
        w->wake();
    }
}

} // namespace flux
