#pragma once

#include <Flux/Graphics/RenderCommandBuffer.hpp>
#include <Flux/Graphics/GlyphAtlas.hpp>
#include <Flux/Graphics/PathFlattener.hpp>
#include <Flux/GPU/Types.hpp>
#include <Flux/Core/Types.hpp>
#include <vector>
#include <array>

namespace flux {

struct SDFQuadInstance {
    float rect[4];         // x, y, width, height
    float corners[4];      // topLeft, topRight, bottomRight, bottomLeft
    float fillColor[4];    // r, g, b, a
    float strokeColor[4];  // r, g, b, a
    float strokeWidth;
    float opacity;
    float viewport[2];     // viewport width, height
};
static_assert(sizeof(SDFQuadInstance) == 80);

struct ImageInstance {
    float screenRect[4];
    float uvRect[4];
    float tint[4];
    float viewport[2];
    float _pad[2];
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
    CompiledBatches compile(const RenderCommandBuffer& buffer, float vpWidth, float vpHeight,
                            float dpiScaleX = 1.0f, float dpiScaleY = 1.0f);

private:
    struct State {
        float translateX = 0, translateY = 0;
        float scaleX = 1, scaleY = 1;
        float opacity = 1;
        FillStyle fill;
        StrokeStyle stroke;
        TextStyle textStyle;  // must save/restore with save/restore like NanoVG
        ScissorState scissor;
    };

    GlyphAtlas* atlas_ = nullptr;
    std::vector<State> stateStack_;
    State current_;

    void applyTransform(float& x, float& y) const;
    void startNewGroup(CompiledBatches& out);
    void pushRect(CompiledBatches& out, const CmdDrawRect& cmd);
    void pushCircle(CompiledBatches& out, const CmdDrawCircle& cmd);
    void pushLine(CompiledBatches& out, const CmdDrawLine& cmd);
    void pushText(CompiledBatches& out, const CmdDrawText& cmd);
    void pushTextBox(CompiledBatches& out, const CmdDrawTextBox& cmd);
    void pushPath(CompiledBatches& out, const CmdDrawPath& cmd);
    void pushImage(CompiledBatches& out, const CmdDrawImage& cmd);
    void pushImagePath(CompiledBatches& out, const CmdDrawImagePath& cmd);

    void fillInstanceColors(SDFQuadInstance& inst) const;
    ImageInstance makeImageInstance(const Rect& rect, float alpha) const;
};

} // namespace flux
