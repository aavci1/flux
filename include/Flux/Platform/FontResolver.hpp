#pragma once

#include <Flux/Core/Types.hpp>
#include <cstdint>
#include <optional>
#include <string>

namespace flux {

class FontResolver {
public:
    virtual ~FontResolver() = default;
    virtual std::optional<std::string> findFontPath(const std::string& familyName,
                                                     FontWeight weight) = 0;
    virtual std::optional<std::string> findFontPathForCodepoint(
        uint32_t codepoint, const std::string& baseFontPath = "") = 0;
};

} // namespace flux
