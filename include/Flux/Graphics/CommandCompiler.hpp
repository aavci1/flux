#pragma once

#include <Flux/Graphics/RenderCommandBuffer.hpp>
#include <Flux/Graphics/GlyphAtlas.hpp>
#include <Flux/Graphics/PathFlattener.hpp>
#include <Flux/GPU/Types.hpp>
#include <Flux/Core/Types.hpp>
#include <vector>
#include <array>
#include <cmath>

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

struct CompiledBatches {
    std::vector<SDFQuadInstance> rects;
    std::vector<SDFQuadInstance> circles;
    std::vector<SDFQuadInstance> lines;
    std::vector<GlyphInstance> glyphs;
    std::vector<PathVertex> pathVertices;
    std::vector<ImageDrawCmd> imageDraws;
    Rect scissor;
    bool hasScissor = false;
    gpu::ClearColor clearColor;
    float viewportWidth = 0;
    float viewportHeight = 0;
};

class CommandCompiler {
public:
    void setGlyphAtlas(GlyphAtlas* atlas) { atlas_ = atlas; }
    CompiledBatches compile(const RenderCommandBuffer& buffer, float vpWidth, float vpHeight);

private:
    struct State {
        float translateX = 0, translateY = 0;
        float scaleX = 1, scaleY = 1;
        float opacity = 1;
        FillStyle fill;
        StrokeStyle stroke;
    };

    GlyphAtlas* atlas_ = nullptr;
    std::vector<State> stateStack_;
    State current_;
    TextStyle currentText_;

    void applyTransform(float& x, float& y) const;
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
