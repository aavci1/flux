#include <Flux/Graphics/FontProvider.hpp>
#include <Flux/Platform/FontResolver.hpp>
#include <Flux/Platform/PlatformRegistry.hpp>

namespace flux {

std::optional<std::string> FontProvider::findFontPath(const std::string& familyName, FontWeight weight) {
    if (auto* resolver = PlatformRegistry::instance().fontResolver()) {
        return resolver->findFontPath(familyName, weight);
    }
    return std::nullopt;
}

std::optional<std::string> FontProvider::findFontPathForCodepoint(uint32_t codepoint,
                                                                 const std::string& baseFontPath) {
    if (auto* resolver = PlatformRegistry::instance().fontResolver()) {
        return resolver->findFontPathForCodepoint(codepoint, baseFontPath);
    }
    return std::nullopt;
}

} // namespace flux
