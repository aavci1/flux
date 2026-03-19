#pragma once

#include <Flux/Core/Types.hpp>
#include <string>
#include <optional>

namespace flux {

class FontDiscovery {
public:
    static std::optional<std::string> findFontPath(const std::string& familyName,
                                                    FontWeight weight = FontWeight::regular);
};

} // namespace flux
