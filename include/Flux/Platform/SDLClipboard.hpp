#pragma once

#include <Flux/Platform/Clipboard.hpp>

namespace flux {

class SDLClipboard : public Clipboard {
public:
    void setText(const std::string& text) override;
    bool hasText() override;
    std::string getText() override;
};

} // namespace flux
