#pragma once

#include <Flux/Platform/Clipboard.hpp>
#include <Flux/Platform/PlatformRegistry.hpp>
#include <string>

namespace flux {

inline void setClipboardText(const std::string& text) {
    if (auto* c = PlatformRegistry::instance().clipboard()) {
        c->setText(text);
    }
}

inline bool hasClipboardText() {
    if (auto* c = PlatformRegistry::instance().clipboard()) {
        return c->hasText();
    }
    return false;
}

inline std::string getClipboardText() {
    if (auto* c = PlatformRegistry::instance().clipboard()) {
        return c->getText();
    }
    return {};
}

} // namespace flux
