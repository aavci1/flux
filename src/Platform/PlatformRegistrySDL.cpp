#include <Flux/Platform/PlatformRegistry.hpp>
#include <Flux/Platform/SDLWindowFactory.hpp>
#include <Flux/Platform/SDLClipboard.hpp>
#include <Flux/Platform/SDLEventLoopWake.hpp>

#if defined(__linux__)
#include <Flux/Platform/LinuxFontResolver.hpp>
#include <Flux/Platform/LinuxPathUtil.hpp>
#else
#include <Flux/Platform/StubFontResolver.hpp>
#include <Flux/Platform/StubPathUtil.hpp>
#endif

namespace flux {

PlatformRegistry& PlatformRegistry::instance() {
    static SDLWindowFactory windowFactory;
    static SDLClipboard clipboard;
#if defined(__linux__)
    static LinuxFontResolver fontResolver;
    static LinuxPathUtil pathUtil;
#else
    static StubFontResolver fontResolver;
    static StubPathUtil pathUtil;
#endif
    static SDLEventLoopWake eventLoopWake;
    static PlatformRegistry registry(
        &windowFactory, &clipboard, &fontResolver, &eventLoopWake, &pathUtil);
    return registry;
}

} // namespace flux
