#pragma once

#include <Flux/Graphics/RenderCommandBuffer.hpp>
#include <Flux/Graphics/GlyphAtlas.hpp>
#include <Flux/Graphics/PathFlattener.hpp>
#include <Flux/GPU/Types.hpp>
#include <Flux/Core/Types.hpp>
#include <cstdint>
#include <array>
#include <list>
#include <optional>
#include <unordered_map>
#include <vector>

namespace flux {

struct SDFQuadInstance {
    float rect[4];         // x, y, width, height (min corner + size; axis-aligned in local SDF space)
    float corners[4];      // topLeft, topRight, bottomRight, bottomLeft radius (line: cos/sin in xy)
    float fillColor[4];    // r, g, b, a
    float strokeColor[4];  // r, g, b, a
    float strokeWidth;
    float opacity;
    float viewport[2];     // viewport width, height
    float rotation;        // radians — rotates local quad in screen space (lines: 0)
    float _pad[3];
};
static_assert(sizeof(SDFQuadInstance) == 96);

struct ImageInstance {
    float screenRect[4];
    float uvRect[4];
    float tint[4];
    float viewport[2];
    float rotation; // radians
    float _pad;
};
static_assert(sizeof(ImageInstance) == 64);

struct ImageDrawCmd {
    std::string path;
    int imageId = 0;
    ImageInstance instance;
};

struct ScissorState {
    bool active = false;
    float x = 0, y = 0, width = 0, height = 0;

    bool operator==(const ScissorState& o) const {
        if (active != o.active) return false;
        if (!active) return true;
        return x == o.x && y == o.y && width == o.width && height == o.height;
    }
    bool operator!=(const ScissorState& o) const { return !(*this == o); }
};

enum class DrawOpType { Rect, Circle, Line, Glyph, Path, Image };

struct DrawOp {
    DrawOpType type = DrawOpType::Rect;
    uint32_t offset = 0;  // index into the type's buffer (for Image: index into imageDraws)
    uint32_t count = 0;
};

struct DrawGroup {
    ScissorState scissor;
    uint32_t rectOffset = 0, rectCount = 0;
    uint32_t circleOffset = 0, circleCount = 0;
    uint32_t lineOffset = 0, lineCount = 0;
    uint32_t glyphOffset = 0, glyphCount = 0;
    uint32_t pathOffset = 0, pathCount = 0;
    std::vector<ImageDrawCmd> imageDraws;
    std::vector<DrawOp> drawOps;  // order of draws within this group (command order)
};

struct CompiledBatches {
    std::vector<SDFQuadInstance> rects;
    std::vector<SDFQuadInstance> circles;
    std::vector<SDFQuadInstance> lines;
    std::vector<GlyphInstance> glyphs;
    std::vector<PathVertex> pathVertices;
    std::vector<DrawGroup> groups;
    gpu::ClearColor clearColor;
    float viewportWidth = 0;
    float viewportHeight = 0;
};

/** Walks a recorded command buffer and produces batched GPU instance data (SDF quads, glyphs, tessellated paths). */
class CommandCompiler {
public:
    void setGlyphAtlas(GlyphAtlas* atlas) { atlas_ = atlas; }
    /// Clears and fills \p out while retaining vector capacity across frames for fewer allocations.
    /// Always walks the full command buffer. Incremental compile (dirty subtrees / retained
    /// command slices) requires Element/renderer integration and is not implemented here.
    void compile(const RenderCommandBuffer& buffer, float vpWidth, float vpHeight,
                 float dpiScaleX, float dpiScaleY, CompiledBatches& out);

private:
    /// Key for tessellation cache (solid fill/stroke, no dash; quantized transform).
    struct PathTessCacheKey {
        uint64_t pathHash = 0;
        int32_t m00 = 0, m01 = 0, m02 = 0, m10 = 0, m11 = 0, m12 = 0;
        int32_t qStrokeW = 0;
        uint32_t fillArgb = 0;
        uint32_t strokeArgb = 0;
        uint16_t vpW = 0, vpH = 0;
        uint8_t hasFill = 0;
        uint8_t hasStroke = 0;
        uint8_t strokeCap = 0;
        uint8_t strokeJoin = 0;
        int32_t qMiterLimit = 0;
        bool operator==(const PathTessCacheKey& o) const = default;
    };

    struct PathTessCacheKeyHash {
        size_t operator()(const PathTessCacheKey& k) const noexcept;
    };

    using PathTessEntry = std::pair<PathTessCacheKey, std::vector<PathVertex>>;
    std::list<PathTessEntry> pathTessLru_;
    std::unordered_map<PathTessCacheKey,
                       std::list<PathTessEntry>::iterator,
                       PathTessCacheKeyHash> pathTessIndex_;
    /// 2×3 affine (column-major linear part): (x,y)' -> (m00*x+m01*y+m02, m10*x+m11*y+m12)
    struct State {
        float m00 = 1, m01 = 0, m02 = 0;
        float m10 = 0, m11 = 1, m12 = 0;
        float opacity = 1;
        FillStyle fill;
        StrokeStyle stroke;
        TextStyle textStyle;
        ScissorState scissor;
    };

    GlyphAtlas* atlas_ = nullptr;
    std::vector<State> stateStack_;
    State current_;

    /// High-water marks for \ref CompiledBatches vectors (explicit reserve after clear).
    size_t rectPeak_ = 0;
    size_t circlePeak_ = 0;
    size_t linePeak_ = 0;
    size_t glyphPeak_ = 0;
    size_t pathVertPeak_ = 0;
    size_t groupPeak_ = 0;

    void applyTransform(float& x, float& y) const;
    void transformGlyphInstance(GlyphInstance& gi) const;
    float linearScale() const;
    /// Legacy horizontal scale (||column 0||); matches pre-affine `scaleX` for text sizing.
    float horizontalScale() const;
    /// True if linear part is axis-aligned (scale + translation only, no rotation/shear).
    bool isAxisAligned() const;
    void startNewGroup(CompiledBatches& out);
    void pushRect(CompiledBatches& out, const Rect& bounds, const CornerRadius& cr);
    void pushCircle(CompiledBatches& out, const Point& center, float radius);
    void pushLine(CompiledBatches& out, const Point& from, const Point& to);
    void pushText(CompiledBatches& out, const std::string& text,
                  const Point& pos, HorizontalAlignment hAlign, VerticalAlignment vAlign);
    void pushTextBox(CompiledBatches& out, const std::string& text,
                     const Point& pos, float maxWidth, HorizontalAlignment hAlign);
    void pushPath(CompiledBatches& out, const Path& path);
    void pushImage(CompiledBatches& out, int imageId, const Rect& rect,
                   ImageFit fit, const CornerRadius& cr, float alpha);
    void pushImagePath(CompiledBatches& out, const std::string& imgPath, const Rect& rect,
                       ImageFit fit, const CornerRadius& cr, float alpha);

    void fillInstanceColors(SDFQuadInstance& inst) const;
    ImageInstance makeImageInstance(const Rect& rect, float alpha) const;

    std::optional<PathTessCacheKey> makePathTessCacheKey(const Path& path, float vpW, float vpH) const;
};

} // namespace flux
