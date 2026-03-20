#pragma once

#include <Flux/Platform/FontResolver.hpp>

namespace flux {

class AppleFontResolver : public FontResolver {
public:
    std::optional<std::string> findFontPath(const std::string& familyName, FontWeight weight) override;
    std::optional<std::string> findFontPathForCodepoint(uint32_t codepoint,
                                                       const std::string& baseFontPath) override;
};

} // namespace flux
