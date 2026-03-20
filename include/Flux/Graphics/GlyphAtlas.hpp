#pragma once

#include <Flux/Graphics/FontProvider.hpp>
#include <Flux/GPU/Device.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cstdint>

namespace flux {

// ---------------------------------------------------------------------------
// GPU-specific glyph types (atlas UV coordinates, instanced draw data)
// ---------------------------------------------------------------------------

struct GlyphInfo {
    float u0, v0, u1, v1;
    float width, height;
    float bearingX, bearingY;
    float advance;
};

struct GlyphKey {
    uint32_t codepoint;
    uint16_t fontSize;
    uint16_t fontIndex;

    bool operator==(const GlyphKey& other) const = default;
};

struct GlyphKeyHash {
    size_t operator()(const GlyphKey& k) const {
        return std::hash<uint64_t>{}(
            (static_cast<uint64_t>(k.codepoint) << 32) |
            (static_cast<uint64_t>(k.fontSize) << 16) |
            k.fontIndex);
    }
};

struct GlyphInstance {
    float screenRect[4];
    float uvRect[4];
    float color[4];
    float viewport[2];
    float _pad[2];
};
static_assert(sizeof(GlyphInstance) == 64);

// ---------------------------------------------------------------------------
// GlyphAtlas — concrete FontProvider backed by FreeType + a GPU texture atlas
// ---------------------------------------------------------------------------

class GlyphAtlas final : public FontProvider {
public:
    GlyphAtlas(gpu::Device* device, uint32_t atlasSize = 1024);
    ~GlyphAtlas() override;

    // -- FontProvider interface ------------------------------------------------

    bool loadFont(const std::string& path, uint16_t fontIndex = 0) override;
    bool loadFontByName(const std::string& name, FontWeight weight,
                        uint16_t fontIndex = 0) override;
    [[nodiscard]] std::optional<uint16_t> ensureFontLoaded(const std::string& name,
                                                            FontWeight weight) override;
    bool hasFont(uint16_t fontIndex = 0) const override { return faces_.count(fontIndex) > 0; }
    Size measureText(const std::string& text, float fontSize,
                     uint16_t fontIndex = 0) override;
    Size measureTextBox(const std::string& text, float fontSize, float maxWidth,
                        uint16_t fontIndex = 0) override;

    // -- GPU-specific API (glyph rasterization + atlas) ------------------------

    const GlyphInfo* getGlyph(uint32_t codepoint, uint16_t fontSize, uint16_t fontIndex = 0);

    std::vector<GlyphInstance> layoutText(const std::string& text, float x, float y,
                                           float fontSize, const Color& color,
                                           float viewportW, float viewportH,
                                           uint16_t fontIndex = 0);

    std::vector<GlyphInstance> layoutTextBox(const std::string& text, float x, float y,
                                              float fontSize, float maxWidth,
                                              const Color& color,
                                              float viewportW, float viewportH,
                                              HorizontalAlignment hAlign,
                                              uint16_t fontIndex = 0);

    gpu::Texture* texture() const { return texture_.get(); }
    bool dirty() const { return dirty_; }
    void uploadIfDirty();

private:
    gpu::Device* device_;
    FT_Library ftLib_ = nullptr;
    std::unordered_map<uint16_t, FT_Face> faces_;
    std::unordered_map<GlyphKey, GlyphInfo, GlyphKeyHash> cache_;
    std::unordered_map<std::string, uint16_t> fontKeyToIndex_;
    uint16_t nextFontIndex_{0};

    uint32_t atlasSize_;
    std::vector<uint8_t> atlasData_;
    std::unique_ptr<gpu::Texture> texture_;
    bool dirty_ = false;

    uint32_t cursorX_ = 0;
    uint32_t cursorY_ = 0;
    uint32_t rowHeight_ = 0;

    std::unordered_map<std::string, uint16_t> fallbackPathToIndex_;
    std::unordered_set<uint32_t> codepointMisses_;

    std::string faceFilePath(FT_Face face) const;
    std::optional<uint16_t> loadFallbackForCodepoint(uint32_t codepoint, uint16_t baseFontIndex);

    bool rasterizeGlyph(const GlyphKey& key, GlyphInfo& out);
    std::vector<std::string> wrapText(const std::string& text, float fontSize,
                                       float maxWidth, uint16_t fontIndex);
};

} // namespace flux
