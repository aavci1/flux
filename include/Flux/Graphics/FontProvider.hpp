#pragma once

#include <Flux/Core/Types.hpp>
#include <string>
#include <optional>
#include <cstdint>

namespace flux {

/// Abstract font subsystem interface.
///
/// Provides font loading, text measurement, and platform font resolution.
/// Concrete implementations (e.g. GlyphAtlas) add rendering-specific features
/// on top of this interface.
///
/// Pattern follows the same abstract-base approach used by gpu::Device and
/// PlatformWindow elsewhere in the framework.
class FontProvider {
public:
    virtual ~FontProvider() = default;

    // -- Font loading ----------------------------------------------------------

    virtual bool loadFont(const std::string& path, uint16_t fontIndex = 0) = 0;
    virtual bool loadFontByName(const std::string& name, FontWeight weight,
                                uint16_t fontIndex = 0) = 0;

    /// Resolve a stable font slot for (family, weight). Loads the font on first
    /// call and reuses the slot thereafter.
    [[nodiscard]]
    virtual std::optional<uint16_t> ensureFontLoaded(const std::string& name,
                                                      FontWeight weight) = 0;

    virtual bool hasFont(uint16_t fontIndex = 0) const = 0;

    // -- Text measurement ------------------------------------------------------

    virtual Size measureText(const std::string& text, float fontSize,
                             uint16_t fontIndex = 0) = 0;
    virtual Size measureTextBox(const std::string& text, float fontSize,
                                float maxWidth, uint16_t fontIndex = 0) = 0;

    // -- Platform font resolution (static utilities) ---------------------------

    /// Ask the OS for the file path of a font matching `familyName` + `weight`.
    static std::optional<std::string> findFontPath(
        const std::string& familyName, FontWeight weight = FontWeight::regular);

    /// Ask the OS for a font that can render `codepoint`, seeded from
    /// `baseFontPath` for style matching. Returns the resolved font file path.
    static std::optional<std::string> findFontPathForCodepoint(
        uint32_t codepoint, const std::string& baseFontPath = "");
};

} // namespace flux
