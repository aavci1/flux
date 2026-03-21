#pragma once

#include <Flux/Graphics/FontProvider.hpp>
#include <Flux/GPU/Device.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <list>
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
    float rotation; // radians, 0 = axis-aligned
    float _pad;
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

    /// Bytes written to the GPU texture in the last \ref uploadIfDirty (R8: one byte per texel). Zero if no upload ran.
    uint64_t lastGpuUploadBytes() const { return lastGpuUploadBytes_; }

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
    /// Bounding box of CPU atlas texels touched since the last upload (half-open ranges).
    bool dirtyRectValid_ = false;
    uint32_t dirtyX0_ = 0, dirtyY0_ = 0, dirtyX1_ = 0, dirtyY1_ = 0;
    uint64_t lastGpuUploadBytes_ = 0;

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

    void expandDirtyRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h);
    void markFullAtlasDirty();
    void clearTextLayoutCaches();

    struct MeasureKey {
        std::string text;
        uint16_t fsz = 0;
        uint16_t fontIndex = 0;
        bool operator==(const MeasureKey& o) const {
            return fsz == o.fsz && fontIndex == o.fontIndex && text == o.text;
        }
    };
    struct MeasureKeyHash {
        size_t operator()(const MeasureKey& k) const {
            size_t h = std::hash<std::string>()(k.text);
            h ^= k.fsz + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= k.fontIndex + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };

    struct WrapKey {
        std::string text;
        uint16_t fsz = 0;
        int32_t maxWQ = 0;
        uint16_t fontIndex = 0;
        bool operator==(const WrapKey& o) const {
            return fsz == o.fsz && maxWQ == o.maxWQ && fontIndex == o.fontIndex && text == o.text;
        }
    };
    struct WrapKeyHash {
        size_t operator()(const WrapKey& k) const {
            size_t h = std::hash<std::string>()(k.text);
            h ^= k.fsz + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= static_cast<size_t>(k.maxWQ) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= k.fontIndex + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };

    static constexpr std::size_t kAtlasTextCacheMax = 2048;

    using MeasureEntry = std::pair<MeasureKey, Size>;
    std::list<MeasureEntry> measureLru_;
    std::unordered_map<MeasureKey,
                       std::list<MeasureEntry>::iterator,
                       MeasureKeyHash> measureIndex_;

    using WrapEntry = std::pair<WrapKey, std::vector<std::string>>;
    std::list<WrapEntry> wrapLru_;
    std::unordered_map<WrapKey,
                       std::list<WrapEntry>::iterator,
                       WrapKeyHash> wrapIndex_;
};

} // namespace flux
