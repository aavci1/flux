#pragma once

#include <Flux/Platform/RenderBackend.hpp>

namespace flux {

class PlatformWindowFactory;
class Clipboard;
class FontResolver;
class EventLoopWake;
class PathUtil;
class ShortcutManager;

/// Central access point for platform services. Constructed once per platform in `instance()`.
class PlatformRegistry {
public:
    static PlatformRegistry& instance();

    PlatformWindowFactory* windowFactory();
    Clipboard* clipboard();
    FontResolver* fontResolver();
    EventLoopWake* eventLoopWake();
    PathUtil* pathUtil();

    RenderBackendType defaultRenderBackend() const;
    void setDefaultRenderBackend(RenderBackendType backend);

    void registerWindowShortcuts(ShortcutManager& sm);

    /// Used only from platform `instance()` implementations.
    PlatformRegistry(
        PlatformWindowFactory* windowFactory,
        Clipboard* clipboard,
        FontResolver* fontResolver,
        EventLoopWake* eventLoopWake,
        PathUtil* pathUtil
    );

private:
    PlatformWindowFactory* windowFactory_;
    Clipboard* clipboard_;
    FontResolver* fontResolver_;
    EventLoopWake* eventLoopWake_;
    PathUtil* pathUtil_;
};

inline PlatformWindowFactory* getDefaultPlatformFactory() {
    return PlatformRegistry::instance().windowFactory();
}

} // namespace flux
