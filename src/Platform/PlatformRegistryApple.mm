#include <Flux/Platform/PlatformRegistry.hpp>
#include <Flux/Platform/MacWindowFactory.hpp>
#include <Flux/Platform/AppleClipboard.hpp>
#include <Flux/Platform/AppleFontResolver.hpp>
#include <Flux/Platform/AppleEventLoopWake.hpp>
#include <Flux/Platform/ApplePathUtil.hpp>

namespace flux {

PlatformRegistry& PlatformRegistry::instance() {
    static MacWindowFactory windowFactory;
    static AppleClipboard clipboard;
    static AppleFontResolver fontResolver;
    static AppleEventLoopWake eventLoopWake;
    static ApplePathUtil pathUtil;
    static PlatformRegistry registry(
        &windowFactory, &clipboard, &fontResolver, &eventLoopWake, &pathUtil);
    return registry;
}

} // namespace flux
