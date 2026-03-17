#pragma once

#include <Flux/Core/Types.hpp>
#include <variant>
#include <vector>
#include <string>
#include <array>

namespace flux {

struct CmdDrawRect {
    Rect bounds;
    Color fillColor;
    Color strokeColor;
    float strokeWidth = 0;
    CornerRadius cornerRadius = {0, 0, 0, 0};
};

struct CmdDrawCircle {
    Point center;
    float radius;
    Color fillColor;
    Color strokeColor;
    float strokeWidth = 0;
};

struct CmdDrawLine {
    Point from;
    Point to;
    Color color;
    float width = 1;
};

struct CmdDrawText {
    std::string text;
    Point position;
    float fontSize;
    Color color;
    std::string fontFamily;
    int align = 0;
};

struct CmdDrawImage {
    int imageHandle;
    Rect srcRect;
    Rect destRect;
    float opacity = 1.0f;
};

struct CmdPushClip {
    Rect bounds;
    CornerRadius cornerRadius = {0, 0, 0, 0};
};

struct CmdPopClip {};

struct CmdPushTransform {
    std::array<float, 6> matrix = {1, 0, 0, 1, 0, 0};
};

struct CmdPopTransform {};

struct CmdSetOpacity {
    float opacity;
};

using RenderCommand = std::variant<
    CmdDrawRect,
    CmdDrawCircle,
    CmdDrawLine,
    CmdDrawText,
    CmdDrawImage,
    CmdPushClip,
    CmdPopClip,
    CmdPushTransform,
    CmdPopTransform,
    CmdSetOpacity
>;

class RenderCommandBuffer {
public:
    void drawRect(const CmdDrawRect& cmd) { commands_.push_back(cmd); }
    void drawCircle(const CmdDrawCircle& cmd) { commands_.push_back(cmd); }
    void drawLine(const CmdDrawLine& cmd) { commands_.push_back(cmd); }
    void drawText(const CmdDrawText& cmd) { commands_.push_back(cmd); }
    void drawImage(const CmdDrawImage& cmd) { commands_.push_back(cmd); }
    void pushClip(const CmdPushClip& cmd) { commands_.push_back(cmd); }
    void popClip() { commands_.push_back(CmdPopClip{}); }
    void pushTransform(const CmdPushTransform& cmd) { commands_.push_back(cmd); }
    void popTransform() { commands_.push_back(CmdPopTransform{}); }
    void setOpacity(float opacity) { commands_.push_back(CmdSetOpacity{opacity}); }

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
