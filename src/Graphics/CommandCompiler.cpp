#include <Flux/Graphics/CommandCompiler.hpp>
#include <cstring>
#include <cmath>

namespace flux {

CompiledBatches CommandCompiler::compile(const RenderCommandBuffer& buffer,
                                         float vpWidth, float vpHeight) {
    CompiledBatches out;
    out.viewportWidth = vpWidth;
    out.viewportHeight = vpHeight;

    current_ = State{};
    stateStack_.clear();

    for (const auto& cmd : buffer.commands()) {
        std::visit([&](const auto& c) {
            using T = std::decay_t<decltype(c)>;

            if constexpr (std::is_same_v<T, CmdClear>) {
                out.clearColor = {c.color.r, c.color.g, c.color.b, c.color.a};
            }
            else if constexpr (std::is_same_v<T, CmdSave>) {
                stateStack_.push_back(current_);
            }
            else if constexpr (std::is_same_v<T, CmdRestore>) {
                if (!stateStack_.empty()) {
                    current_ = stateStack_.back();
                    stateStack_.pop_back();
                }
            }
            else if constexpr (std::is_same_v<T, CmdTranslate>) {
                current_.translateX += c.x * current_.scaleX;
                current_.translateY += c.y * current_.scaleY;
            }
            else if constexpr (std::is_same_v<T, CmdScale>) {
                current_.scaleX *= c.sx;
                current_.scaleY *= c.sy;
            }
            else if constexpr (std::is_same_v<T, CmdSetOpacity>) {
                current_.opacity = c.opacity;
            }
            else if constexpr (std::is_same_v<T, CmdSetFillStyle>) {
                current_.fill = c.style;
            }
            else if constexpr (std::is_same_v<T, CmdSetStrokeStyle>) {
                current_.stroke = c.style;
            }
            else if constexpr (std::is_same_v<T, CmdDrawRect>) {
                pushRect(out, c);
            }
            else if constexpr (std::is_same_v<T, CmdDrawCircle>) {
                pushCircle(out, c);
            }
            else if constexpr (std::is_same_v<T, CmdDrawLine>) {
                pushLine(out, c);
            }
            else if constexpr (std::is_same_v<T, CmdSetTextStyle>) {
                currentText_ = c.style;
            }
            else if constexpr (std::is_same_v<T, CmdDrawText>) {
                pushText(out, c);
            }
            else if constexpr (std::is_same_v<T, CmdDrawTextBox>) {
                pushTextBox(out, c);
            }
            else if constexpr (std::is_same_v<T, CmdDrawPath>) {
                pushPath(out, c);
            }
            else if constexpr (std::is_same_v<T, CmdClipPath>) {
                auto bounds = c.path.getBounds();
                float bx = bounds.x, by = bounds.y;
                applyTransform(bx, by);
                out.scissor = {bx, by, bounds.width * current_.scaleX, bounds.height * current_.scaleY};
                out.hasScissor = true;
            }
            else if constexpr (std::is_same_v<T, CmdDrawImage>) {
                pushImage(out, c);
            }
            else if constexpr (std::is_same_v<T, CmdDrawImagePath>) {
                pushImagePath(out, c);
            }
            // CmdRotate — future
        }, cmd);
    }

    return out;
}

void CommandCompiler::applyTransform(float& x, float& y) const {
    x = x * current_.scaleX + current_.translateX;
    y = y * current_.scaleY + current_.translateY;
}

void CommandCompiler::fillInstanceColors(SDFQuadInstance& inst) const {
    const auto& fc = current_.fill;
    if (fc.type == FillStyle::Type::Solid || fc.type == FillStyle::Type::None) {
        inst.fillColor[0] = fc.color.r;
        inst.fillColor[1] = fc.color.g;
        inst.fillColor[2] = fc.color.b;
        inst.fillColor[3] = (fc.type == FillStyle::Type::None) ? 0.0f : fc.color.a;
    } else {
        inst.fillColor[0] = fc.startColor.r;
        inst.fillColor[1] = fc.startColor.g;
        inst.fillColor[2] = fc.startColor.b;
        inst.fillColor[3] = fc.startColor.a;
    }

    const auto& sc = current_.stroke;
    if (sc.type != StrokeStyle::Type::None) {
        inst.strokeColor[0] = sc.color.r;
        inst.strokeColor[1] = sc.color.g;
        inst.strokeColor[2] = sc.color.b;
        inst.strokeColor[3] = sc.color.a;
        inst.strokeWidth = sc.width * current_.scaleX;
    } else {
        std::memset(inst.strokeColor, 0, sizeof(inst.strokeColor));
        inst.strokeWidth = 0;
    }

    inst.opacity = current_.opacity;
}

void CommandCompiler::pushRect(CompiledBatches& out, const CmdDrawRect& cmd) {
    SDFQuadInstance inst{};
    float x = cmd.bounds.x, y = cmd.bounds.y;
    applyTransform(x, y);

    inst.rect[0] = x;
    inst.rect[1] = y;
    inst.rect[2] = cmd.bounds.width * current_.scaleX;
    inst.rect[3] = cmd.bounds.height * current_.scaleY;

    float s = current_.scaleX;
    inst.corners[0] = cmd.cornerRadius.topLeft * s;
    inst.corners[1] = cmd.cornerRadius.topRight * s;
    inst.corners[2] = cmd.cornerRadius.bottomRight * s;
    inst.corners[3] = cmd.cornerRadius.bottomLeft * s;

    inst.viewport[0] = out.viewportWidth;
    inst.viewport[1] = out.viewportHeight;
    fillInstanceColors(inst);
    out.rects.push_back(inst);
}

void CommandCompiler::pushCircle(CompiledBatches& out, const CmdDrawCircle& cmd) {
    SDFQuadInstance inst{};
    float cx = cmd.center.x, cy = cmd.center.y;
    applyTransform(cx, cy);
    float r = cmd.radius * current_.scaleX;
    float diam = r * 2.0f;

    inst.rect[0] = cx - r;
    inst.rect[1] = cy - r;
    inst.rect[2] = diam;
    inst.rect[3] = diam;

    inst.viewport[0] = out.viewportWidth;
    inst.viewport[1] = out.viewportHeight;
    fillInstanceColors(inst);
    out.circles.push_back(inst);
}

void CommandCompiler::pushLine(CompiledBatches& out, const CmdDrawLine& cmd) {
    SDFQuadInstance inst{};
    float x0 = cmd.from.x, y0 = cmd.from.y;
    float x1 = cmd.to.x, y1 = cmd.to.y;
    applyTransform(x0, y0);
    applyTransform(x1, y1);

    float dx = x1 - x0, dy = y1 - y0;
    float len = std::sqrt(dx * dx + dy * dy);
    float midX = (x0 + x1) * 0.5f, midY = (y0 + y1) * 0.5f;

    float strokeW = current_.stroke.width * current_.scaleX;
    float halfW = strokeW * 0.5f + 1.0f;

    inst.rect[0] = midX - len * 0.5f - halfW;
    inst.rect[1] = midY - halfW;
    inst.rect[2] = len + halfW * 2.0f;
    inst.rect[3] = halfW * 2.0f;

    inst.strokeWidth = strokeW;
    inst.strokeColor[0] = current_.stroke.color.r;
    inst.strokeColor[1] = current_.stroke.color.g;
    inst.strokeColor[2] = current_.stroke.color.b;
    inst.strokeColor[3] = current_.stroke.color.a;
    inst.opacity = current_.opacity;
    inst.viewport[0] = out.viewportWidth;
    inst.viewport[1] = out.viewportHeight;

    out.lines.push_back(inst);
}

void CommandCompiler::pushText(CompiledBatches& out, const CmdDrawText& cmd) {
    if (!atlas_) return;

    float x = cmd.position.x, y = cmd.position.y;
    applyTransform(x, y);

    float fontSize = currentText_.size * current_.scaleX;
    Color textColor = current_.fill.color;
    textColor.a *= current_.opacity;

    auto sz = atlas_->measureText(cmd.text, fontSize);

    float drawX = x;
    if (cmd.hAlign == HorizontalAlignment::center) drawX -= sz.width * 0.5f;
    else if (cmd.hAlign == HorizontalAlignment::trailing) drawX -= sz.width;

    float drawY = y;
    if (cmd.vAlign == VerticalAlignment::center) drawY += fontSize * 0.35f;
    else if (cmd.vAlign == VerticalAlignment::top) drawY += fontSize;

    auto glyphs = atlas_->layoutText(cmd.text, drawX, drawY, fontSize, textColor,
                                      out.viewportWidth, out.viewportHeight);
    out.glyphs.insert(out.glyphs.end(), glyphs.begin(), glyphs.end());
}

void CommandCompiler::pushPath(CompiledBatches& out, const CmdDrawPath& cmd) {
    auto polyline = PathFlattener::flatten(cmd.path);
    if (polyline.size() < 2) return;

    // Apply transform to polyline
    for (auto& p : polyline) applyTransform(p.x, p.y);

    // Fill
    if (current_.fill.type != FillStyle::Type::None) {
        Color fc = current_.fill.color;
        fc.a *= current_.opacity;
        auto filled = PathFlattener::tessellateFill(polyline, fc, out.viewportWidth, out.viewportHeight);
        out.pathVertices.insert(out.pathVertices.end(), filled.vertices.begin(), filled.vertices.end());
    }

    // Stroke
    if (current_.stroke.type != StrokeStyle::Type::None && current_.stroke.width > 0) {
        Color sc = current_.stroke.color;
        sc.a *= current_.opacity;
        float sw = current_.stroke.width * current_.scaleX;
        auto stroked = PathFlattener::tessellateStroke(polyline, sw, sc, out.viewportWidth, out.viewportHeight);
        out.pathVertices.insert(out.pathVertices.end(), stroked.vertices.begin(), stroked.vertices.end());
    }
}

void CommandCompiler::pushTextBox(CompiledBatches& out, const CmdDrawTextBox& cmd) {
    if (!atlas_) return;

    float x = cmd.position.x, y = cmd.position.y;
    applyTransform(x, y);

    float fontSize = currentText_.size * current_.scaleX;
    float maxWidth = cmd.maxWidth * current_.scaleX;
    Color textColor = current_.fill.color;
    textColor.a *= current_.opacity;

    auto glyphs = atlas_->layoutTextBox(cmd.text, x, y + fontSize, fontSize, maxWidth,
                                         textColor, out.viewportWidth, out.viewportHeight,
                                         cmd.hAlign);
    out.glyphs.insert(out.glyphs.end(), glyphs.begin(), glyphs.end());
}

ImageInstance CommandCompiler::makeImageInstance(const Rect& rect, float alpha) const {
    ImageInstance inst{};
    float x = rect.x, y = rect.y;
    // Note: don't applyTransform here — caller already did it
    inst.screenRect[0] = x;
    inst.screenRect[1] = y;
    inst.screenRect[2] = rect.width;
    inst.screenRect[3] = rect.height;
    inst.uvRect[0] = 0;
    inst.uvRect[1] = 0;
    inst.uvRect[2] = 1;
    inst.uvRect[3] = 1;
    inst.tint[0] = 1;
    inst.tint[1] = 1;
    inst.tint[2] = 1;
    inst.tint[3] = alpha * current_.opacity;
    return inst;
}

void CommandCompiler::pushImage(CompiledBatches& out, const CmdDrawImage& cmd) {
    float x = cmd.rect.x, y = cmd.rect.y;
    applyTransform(x, y);
    Rect transformed{x, y, cmd.rect.width * current_.scaleX, cmd.rect.height * current_.scaleY};

    ImageDrawCmd draw;
    draw.imageId = cmd.imageId;
    draw.instance = makeImageInstance(transformed, cmd.alpha);
    draw.instance.viewport[0] = out.viewportWidth;
    draw.instance.viewport[1] = out.viewportHeight;
    out.imageDraws.push_back(std::move(draw));
}

void CommandCompiler::pushImagePath(CompiledBatches& out, const CmdDrawImagePath& cmd) {
    float x = cmd.rect.x, y = cmd.rect.y;
    applyTransform(x, y);
    Rect transformed{x, y, cmd.rect.width * current_.scaleX, cmd.rect.height * current_.scaleY};

    ImageDrawCmd draw;
    draw.path = cmd.path;
    draw.instance = makeImageInstance(transformed, cmd.alpha);
    draw.instance.viewport[0] = out.viewportWidth;
    draw.instance.viewport[1] = out.viewportHeight;
    out.imageDraws.push_back(std::move(draw));
}

} // namespace flux
