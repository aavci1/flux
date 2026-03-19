#pragma once

#include <Flux/Graphics/RenderContext.hpp>
#include <variant>
#include <vector>
#include <string>

namespace flux {

// =============================================================================
// State commands
// =============================================================================
struct CmdSave {};
struct CmdRestore {};

// =============================================================================
// Transform commands
// =============================================================================
struct CmdTranslate { float x, y; };
struct CmdRotate { float angle; };
struct CmdScale { float sx, sy; };

// =============================================================================
// Style commands (stateful — affect subsequent draws)
// =============================================================================
struct CmdSetOpacity { float opacity; };
struct CmdSetFillStyle { FillStyle style; };
struct CmdSetStrokeStyle { StrokeStyle style; };
struct CmdSetTextStyle { TextStyle style; };

// =============================================================================
// Shape drawing (uses current fill / stroke)
// =============================================================================
struct CmdDrawRect { Rect bounds; CornerRadius cornerRadius; };
struct CmdDrawCircle { Point center; float radius; };
struct CmdDrawLine { Point from; Point to; };
struct CmdDrawPath { Path path; };

// =============================================================================
// Text drawing (uses current text style and fill color)
// =============================================================================
struct CmdDrawText {
    std::string text;
    Point position;
    HorizontalAlignment hAlign;
    VerticalAlignment vAlign;
};

struct CmdDrawTextBox {
    std::string text;
    Point position;
    float maxWidth;
    HorizontalAlignment hAlign;
};

// =============================================================================
// Image drawing
// =============================================================================
struct CmdDrawImage {
    int imageId;
    Rect rect;
    ImageFit fit;
    CornerRadius cornerRadius;
    float alpha;
};

struct CmdDrawImagePath {
    std::string path;
    Rect rect;
    ImageFit fit;
    CornerRadius cornerRadius;
    float alpha;
};

// =============================================================================
// Clipping
// =============================================================================
struct CmdClipPath { Path path; };

// =============================================================================
// Frame
// =============================================================================
struct CmdClear { Color color; };

// The variant of all command types
using RenderCommand = std::variant<
    CmdSave, CmdRestore,
    CmdTranslate, CmdRotate, CmdScale,
    CmdSetOpacity, CmdSetFillStyle, CmdSetStrokeStyle, CmdSetTextStyle,
    CmdDrawRect, CmdDrawCircle, CmdDrawLine, CmdDrawPath,
    CmdDrawText, CmdDrawTextBox,
    CmdDrawImage, CmdDrawImagePath,
    CmdClipPath,
    CmdClear
>;

class RenderCommandBuffer {
public:
    void push(RenderCommand cmd) { commands_.push_back(std::move(cmd)); }

    const std::vector<RenderCommand>& commands() const { return commands_; }
    size_t size() const { return commands_.size(); }
    bool empty() const { return commands_.empty(); }
    void clear() { commands_.clear(); }
    void reserve(size_t n) { commands_.reserve(n); }

private:
    std::vector<RenderCommand> commands_;
};

class RenderBackend {
public:
    virtual ~RenderBackend() = default;
    virtual void execute(const RenderCommandBuffer& buffer) = 0;
};

} // namespace flux
