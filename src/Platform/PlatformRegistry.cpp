#include <Flux/Platform/PlatformRegistry.hpp>
#include <Flux/Platform/PlatformWindowFactory.hpp>
#include <Flux/Core/ShortcutManager.hpp>

namespace flux {

PlatformRegistry::PlatformRegistry(
    PlatformWindowFactory* windowFactory,
    Clipboard* clipboard,
    FontResolver* fontResolver,
    EventLoopWake* eventLoopWake,
    PathUtil* pathUtil
)
    : windowFactory_(windowFactory)
    , clipboard_(clipboard)
    , fontResolver_(fontResolver)
    , eventLoopWake_(eventLoopWake)
    , pathUtil_(pathUtil) {}

PlatformWindowFactory* PlatformRegistry::windowFactory() {
    return windowFactory_;
}

Clipboard* PlatformRegistry::clipboard() {
    return clipboard_;
}

FontResolver* PlatformRegistry::fontResolver() {
    return fontResolver_;
}

EventLoopWake* PlatformRegistry::eventLoopWake() {
    return eventLoopWake_;
}

PathUtil* PlatformRegistry::pathUtil() {
    return pathUtil_;
}

RenderBackendType PlatformRegistry::defaultRenderBackend() const {
    if (windowFactory_) {
        return windowFactory_->renderBackend();
    }
    return RenderBackendType::GPU_Auto;
}

void PlatformRegistry::setDefaultRenderBackend(RenderBackendType backend) {
    if (windowFactory_) {
        windowFactory_->setRenderBackend(backend);
    }
}

void PlatformRegistry::registerWindowShortcuts(ShortcutManager& sm) {
    sm.registerShortcut({Key::Q, KeyModifier::Ctrl}, std::make_unique<QuitCommand>());
#if defined(__APPLE__)
    sm.registerShortcut({Key::Q, KeyModifier::Super}, std::make_unique<QuitCommand>());
    sm.registerShortcut({Key::C, KeyModifier::Super}, std::make_unique<CopyCommand>());
    sm.registerShortcut({Key::V, KeyModifier::Super}, std::make_unique<PasteCommand>());
    sm.registerShortcut({Key::X, KeyModifier::Super}, std::make_unique<CutCommand>());
    sm.registerShortcut({Key::A, KeyModifier::Super}, std::make_unique<SelectAllCommand>());
#else
    sm.registerShortcut({Key::C, KeyModifier::Ctrl}, std::make_unique<CopyCommand>());
    sm.registerShortcut({Key::V, KeyModifier::Ctrl}, std::make_unique<PasteCommand>());
    sm.registerShortcut({Key::X, KeyModifier::Ctrl}, std::make_unique<CutCommand>());
    sm.registerShortcut({Key::A, KeyModifier::Ctrl}, std::make_unique<SelectAllCommand>());
#endif
}

} // namespace flux
