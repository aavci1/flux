#pragma once

#include <Flux/Graphics/RenderContext.hpp>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

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
    uint32_t textStrId = 0;
    Point position;
    HorizontalAlignment hAlign;
    VerticalAlignment vAlign;
};

struct CmdDrawTextBox {
    uint32_t textStrId = 0;
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
    uint32_t pathStrId = 0;
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

    /// Store string in the frame pool and return a stable id (deduplicated within the frame).
    uint32_t internString(std::string s) {
        auto it = stringLookup_.find(s);
        if (it != stringLookup_.end()) return it->second;
        uint32_t id = static_cast<uint32_t>(stringPool_.size());
        stringPool_.push_back(std::move(s));
        stringLookup_[stringPool_.back()] = id;
        return id;
    }

    const std::string& str(uint32_t id) const { return stringPool_.at(id); }

    const std::vector<RenderCommand>& commands() const { return commands_; }
    size_t size() const { return commands_.size(); }
    bool empty() const { return commands_.empty(); }
    void clear() {
        commands_.clear();
        stringPool_.clear();
        stringLookup_.clear();
    }
    void reserve(size_t n) { commands_.reserve(n); }

private:
    std::vector<RenderCommand> commands_;
    std::vector<std::string> stringPool_;
    std::unordered_map<std::string, uint32_t> stringLookup_;
};

class RenderBackend {
public:
    virtual ~RenderBackend() = default;
    virtual void execute(const RenderCommandBuffer& buffer) = 0;
};

} // namespace flux
