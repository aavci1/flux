#include <Flux/Graphics/CommandCompiler.hpp>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <optional>

namespace flux {

static constexpr std::size_t kPathTessCacheMaxEntries = 512;

static int32_t quantizeAffine(float v) {
    return static_cast<int32_t>(std::lround(v * 4096.0));
}

static uint32_t packArgb(const Color& c) {
    auto u8 = [](float x) {
        return static_cast<uint32_t>(std::clamp(x, 0.f, 1.f) * 255.f + 0.5f);
    };
    return (u8(c.a) << 24) | (u8(c.b) << 16) | (u8(c.g) << 8) | u8(c.r);
}

size_t CommandCompiler::PathTessCacheKeyHash::operator()(const PathTessCacheKey& k) const noexcept {
    uint64_t h = k.pathHash;
    auto mix = [&](uint64_t v) {
        h ^= v + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
    };
    mix(static_cast<uint64_t>(static_cast<uint32_t>(k.m00)));
    mix(static_cast<uint64_t>(static_cast<uint32_t>(k.m01)));
    mix(static_cast<uint64_t>(static_cast<uint32_t>(k.m02)));
    mix(static_cast<uint64_t>(static_cast<uint32_t>(k.m10)));
    mix(static_cast<uint64_t>(static_cast<uint32_t>(k.m11)));
    mix(static_cast<uint64_t>(static_cast<uint32_t>(k.m12)));
    mix(static_cast<uint64_t>(static_cast<uint32_t>(k.qStrokeW)));
    mix(static_cast<uint64_t>(k.fillArgb));
    mix(static_cast<uint64_t>(k.strokeArgb));
    mix(static_cast<uint64_t>(k.vpW | (static_cast<uint32_t>(k.vpH) << 16)));
    mix(static_cast<uint64_t>(k.hasFill | (k.hasStroke << 8) | (k.strokeCap << 16) | (k.strokeJoin << 24)));
    mix(static_cast<uint64_t>(static_cast<uint32_t>(k.qMiterLimit)));
    return static_cast<size_t>(h);
}

std::optional<CommandCompiler::PathTessCacheKey> CommandCompiler::makePathTessCacheKey(
    const Path& path, float vpW, float vpH) const {
    if (current_.fill.type != FillStyle::Type::None && current_.fill.type != FillStyle::Type::Solid) {
        return std::nullopt;
    }
    if (current_.stroke.type == StrokeStyle::Type::Dashed) return std::nullopt;
    if (!current_.stroke.dashPattern.empty()) return std::nullopt;

    PathTessCacheKey k{};
    k.pathHash = path.contentHash();
    k.m00 = quantizeAffine(current_.m00);
    k.m01 = quantizeAffine(current_.m01);
    k.m02 = quantizeAffine(current_.m02);
    k.m10 = quantizeAffine(current_.m10);
    k.m11 = quantizeAffine(current_.m11);
    k.m12 = quantizeAffine(current_.m12);

    k.hasFill = (current_.fill.type == FillStyle::Type::Solid) ? 1 : 0;
    if (k.hasFill) {
        Color fc = current_.fill.color;
        fc.a *= current_.opacity;
        k.fillArgb = packArgb(fc);
    }

    const bool hasStroke = current_.stroke.type != StrokeStyle::Type::None && current_.stroke.width > 0.f;
    k.hasStroke = hasStroke ? 1 : 0;
    if (hasStroke) {
        Color sc = current_.stroke.color;
        sc.a *= current_.opacity;
        k.strokeArgb = packArgb(sc);
        k.qStrokeW = quantizeAffine(current_.stroke.width * linearScale());
        k.strokeCap = static_cast<uint8_t>(current_.stroke.cap);
        k.strokeJoin = static_cast<uint8_t>(current_.stroke.join);
        k.qMiterLimit = quantizeAffine(current_.stroke.miterLimit);
    }

    k.vpW = static_cast<uint16_t>(std::clamp(vpW, 0.f, 65535.f));
    k.vpH = static_cast<uint16_t>(std::clamp(vpH, 0.f, 65535.f));
    return k;
}

static ScissorState intersectScissor(const ScissorState& a, const ScissorState& b) {
    if (!a.active) return b;
    if (!b.active) return a;
    float x1 = std::max(a.x, b.x);
    float y1 = std::max(a.y, b.y);
    float x2 = std::min(a.x + a.width, b.x + b.width);
    float y2 = std::min(a.y + a.height, b.y + b.height);
    return {true, x1, y1, std::max(0.0f, x2 - x1), std::max(0.0f, y2 - y1)};
}

void CommandCompiler::startNewGroup(CompiledBatches& out) {
    DrawGroup g;
    g.scissor = current_.scissor;
    g.rectOffset   = static_cast<uint32_t>(out.rects.size());
    g.circleOffset = static_cast<uint32_t>(out.circles.size());
    g.lineOffset   = static_cast<uint32_t>(out.lines.size());
    g.glyphOffset  = static_cast<uint32_t>(out.glyphs.size());
    g.pathOffset   = static_cast<uint32_t>(out.pathVertices.size());
    out.groups.push_back(std::move(g));
}

void CommandCompiler::compile(const RenderCommandBuffer& buffer,
                              float vpWidth, float vpHeight,
                              float dpiScaleX, float dpiScaleY,
                              CompiledBatches& out) {
    out.rects.clear();
    out.circles.clear();
    out.lines.clear();
    out.glyphs.clear();
    out.pathVertices.clear();
    out.groups.clear();
    out.rects.reserve(rectPeak_);
    out.circles.reserve(circlePeak_);
    out.lines.reserve(linePeak_);
    out.glyphs.reserve(glyphPeak_);
    out.pathVertices.reserve(pathVertPeak_);
    out.groups.reserve(groupPeak_);
    out.viewportWidth = vpWidth;
    out.viewportHeight = vpHeight;
    out.clearColor = {};

    current_ = State{};
    current_.m00 = dpiScaleX;
    current_.m11 = dpiScaleY;
    current_.fill = FillStyle::none();
    current_.stroke = StrokeStyle::none();
    current_.textStyle = TextStyle{};
    stateStack_.clear();

    startNewGroup(out);

    auto r = buffer.reader();
    while (r.hasNext()) {
        CmdOp op = r.nextOp();
        switch (op) {
            case CmdOp::Clear: {
                Color col{r.readFloat(), r.readFloat(), r.readFloat(), r.readFloat()};
                out.clearColor = {col.r, col.g, col.b, col.a};
                break;
            }
            case CmdOp::Save:
                stateStack_.push_back(current_);
                break;
            case CmdOp::Restore:
                if (!stateStack_.empty()) {
                    ScissorState prevScissor = current_.scissor;
                    current_ = stateStack_.back();
                    stateStack_.pop_back();
                    if (current_.scissor != prevScissor)
                        startNewGroup(out);
                }
                break;
            case CmdOp::Translate: {
                float x = r.readFloat(), y = r.readFloat();
                current_.m02 += current_.m00 * x + current_.m01 * y;
                current_.m12 += current_.m10 * x + current_.m11 * y;
                break;
            }
            case CmdOp::Rotate: {
                float angle = r.readFloat();
                const float co = std::cos(angle);
                const float si = std::sin(angle);
                const float n00 = current_.m00 * co + current_.m01 * si;
                const float n01 = -current_.m00 * si + current_.m01 * co;
                const float n10 = current_.m10 * co + current_.m11 * si;
                const float n11 = -current_.m10 * si + current_.m11 * co;
                current_.m00 = n00;
                current_.m01 = n01;
                current_.m10 = n10;
                current_.m11 = n11;
                break;
            }
            case CmdOp::Scale: {
                float sx = r.readFloat(), sy = r.readFloat();
                current_.m00 *= sx;
                current_.m01 *= sy;
                current_.m10 *= sx;
                current_.m11 *= sy;
                break;
            }
            case CmdOp::SetOpacity:
                current_.opacity = r.readFloat();
                break;
            case CmdOp::SetFillStyle:
                current_.fill = buffer.fillStyle(r.readUint32());
                break;
            case CmdOp::SetStrokeStyle:
                current_.stroke = buffer.strokeStyle(r.readUint32());
                break;
            case CmdOp::SetTextStyle:
                current_.textStyle = buffer.textStyle(r.readUint32());
                break;
            case CmdOp::DrawRect: {
                Rect bounds{r.readFloat(), r.readFloat(), r.readFloat(), r.readFloat()};
                CornerRadius cr{r.readFloat(), r.readFloat(), r.readFloat(), r.readFloat()};
                pushRect(out, bounds, cr);
                break;
            }
            case CmdOp::DrawCircle: {
                Point center{r.readFloat(), r.readFloat()};
                float radius = r.readFloat();
                pushCircle(out, center, radius);
                break;
            }
            case CmdOp::DrawLine: {
                Point from{r.readFloat(), r.readFloat()};
                Point to{r.readFloat(), r.readFloat()};
                pushLine(out, from, to);
                break;
            }
            case CmdOp::DrawPath: {
                const Path& path = buffer.path(r.readUint32());
                pushPath(out, path);
                break;
            }
            case CmdOp::DrawText: {
                uint32_t strId = r.readUint32();
                Point pos{r.readFloat(), r.readFloat()};
                auto hAlign = static_cast<HorizontalAlignment>(r.readUint32());
                auto vAlign = static_cast<VerticalAlignment>(r.readUint32());
                pushText(out, buffer.str(strId), pos, hAlign, vAlign);
                break;
            }
            case CmdOp::DrawTextBox: {
                uint32_t strId = r.readUint32();
                Point pos{r.readFloat(), r.readFloat()};
                float maxWidth = r.readFloat();
                auto hAlign = static_cast<HorizontalAlignment>(r.readUint32());
                pushTextBox(out, buffer.str(strId), pos, maxWidth, hAlign);
                break;
            }
            case CmdOp::ClipPath: {
                const Path& clipPath = buffer.path(r.readUint32());
                auto bounds = clipPath.getBounds();
                float bx = bounds.x, by = bounds.y, bw = bounds.width, bh = bounds.height;
                float x0 = bx, y0 = by;
                float x1 = bx + bw, y1 = by;
                float x2 = bx + bw, y2 = by + bh;
                float x3 = bx, y3 = by + bh;
                applyTransform(x0, y0);
                applyTransform(x1, y1);
                applyTransform(x2, y2);
                applyTransform(x3, y3);
                const float minX = std::min({x0, x1, x2, x3});
                const float maxX = std::max({x0, x1, x2, x3});
                const float minY = std::min({y0, y1, y2, y3});
                const float maxY = std::max({y0, y1, y2, y3});
                ScissorState newClip{true, minX, minY, maxX - minX, maxY - minY};
                ScissorState merged = intersectScissor(current_.scissor, newClip);
                if (merged != current_.scissor) {
                    current_.scissor = merged;
                    startNewGroup(out);
                }
                break;
            }
            case CmdOp::DrawImage: {
                int imageId = r.readInt32();
                Rect rect{r.readFloat(), r.readFloat(), r.readFloat(), r.readFloat()};
                auto fit = static_cast<ImageFit>(r.readUint32());
                CornerRadius cr{r.readFloat(), r.readFloat(), r.readFloat(), r.readFloat()};
                float alpha = r.readFloat();
                pushImage(out, imageId, rect, fit, cr, alpha);
                break;
            }
            case CmdOp::DrawImagePath: {
                uint32_t pathStrId = r.readUint32();
                Rect rect{r.readFloat(), r.readFloat(), r.readFloat(), r.readFloat()};
                auto fit = static_cast<ImageFit>(r.readUint32());
                CornerRadius cr{r.readFloat(), r.readFloat(), r.readFloat(), r.readFloat()};
                float alpha = r.readFloat();
                pushImagePath(out, buffer.str(pathStrId), rect, fit, cr, alpha);
                break;
            }
        }
    }

    rectPeak_ = std::max(rectPeak_, out.rects.size());
    circlePeak_ = std::max(circlePeak_, out.circles.size());
    linePeak_ = std::max(linePeak_, out.lines.size());
    glyphPeak_ = std::max(glyphPeak_, out.glyphs.size());
    pathVertPeak_ = std::max(pathVertPeak_, out.pathVertices.size());
    groupPeak_ = std::max(groupPeak_, out.groups.size());
}

void CommandCompiler::applyTransform(float& x, float& y) const {
    const float ox = current_.m00 * x + current_.m01 * y + current_.m02;
    const float oy = current_.m10 * x + current_.m11 * y + current_.m12;
    x = ox;
    y = oy;
}

float CommandCompiler::linearScale() const {
    const float det = current_.m00 * current_.m11 - current_.m01 * current_.m10;
    return std::sqrt(std::max(0.f, std::abs(det)));
}

float CommandCompiler::horizontalScale() const {
    return std::hypot(current_.m00, current_.m10);
}

bool CommandCompiler::isAxisAligned() const {
    constexpr float eps = 1e-5f;
    return std::abs(current_.m01) < eps && std::abs(current_.m10) < eps;
}

void CommandCompiler::transformGlyphInstance(GlyphInstance& gi) const {
    const float w = gi.screenRect[2];
    const float h = gi.screenRect[3];
    float cx = gi.screenRect[0] + w * 0.5f;
    float cy = gi.screenRect[1] + h * 0.5f;
    applyTransform(cx, cy);
    gi.screenRect[0] = cx - w * 0.5f;
    gi.screenRect[1] = cy - h * 0.5f;
    gi.screenRect[2] = w;
    gi.screenRect[3] = h;
    gi.rotation = std::atan2(current_.m10, current_.m00);
    gi.atlasPage = 0.f;
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
        inst.strokeWidth = sc.width * linearScale();
    } else {
        std::memset(inst.strokeColor, 0, sizeof(inst.strokeColor));
        inst.strokeWidth = 0;
    }

    inst.opacity = current_.opacity;
}

void CommandCompiler::pushRect(CompiledBatches& out, const Rect& bounds, const CornerRadius& cornerRadius) {
    SDFQuadInstance inst{};
    const float cx = bounds.x + bounds.width * 0.5f;
    const float cy = bounds.y + bounds.height * 0.5f;
    float ox = cx, oy = cy;
    applyTransform(ox, oy);

    const float hw = bounds.width * 0.5f;
    const float hh = bounds.height * 0.5f;
    const float hwScr = std::hypot(current_.m00 * hw, current_.m10 * hw);
    const float hhScr = std::hypot(current_.m01 * hh, current_.m11 * hh);
    inst.rect[0] = ox - hwScr;
    inst.rect[1] = oy - hhScr;
    inst.rect[2] = 2.f * hwScr;
    inst.rect[3] = 2.f * hhScr;
    inst.rotation = std::atan2(current_.m10, current_.m00);
    inst._pad[0] = inst._pad[1] = inst._pad[2] = 0.f;

    const float col0 = std::hypot(current_.m00, current_.m10);
    const float col1 = std::hypot(current_.m01, current_.m11);
    const float s = 0.5f * (col0 + col1);
    inst.corners[0] = cornerRadius.topLeft * s;
    inst.corners[1] = cornerRadius.topRight * s;
    inst.corners[2] = cornerRadius.bottomRight * s;
    inst.corners[3] = cornerRadius.bottomLeft * s;

    inst.viewport[0] = out.viewportWidth;
    inst.viewport[1] = out.viewportHeight;
    fillInstanceColors(inst);
    out.rects.push_back(inst);
    auto& g = out.groups.back();
    g.drawOps.push_back({DrawOpType::Rect, g.rectCount, 1});
    g.rectCount++;
}

void CommandCompiler::pushCircle(CompiledBatches& out, const Point& center, float radius) {
    SDFQuadInstance inst{};
    float cx = center.x, cy = center.y;
    applyTransform(cx, cy);
    const float rScr = radius * linearScale();
    const float diam = rScr * 2.0f;

    inst.rect[0] = cx - rScr;
    inst.rect[1] = cy - rScr;
    inst.rect[2] = diam;
    inst.rect[3] = diam;
    inst.rotation = 0.f;
    inst._pad[0] = inst._pad[1] = inst._pad[2] = 0.f;

    inst.viewport[0] = out.viewportWidth;
    inst.viewport[1] = out.viewportHeight;
    fillInstanceColors(inst);
    out.circles.push_back(inst);
    auto& g = out.groups.back();
    g.drawOps.push_back({DrawOpType::Circle, g.circleCount, 1});
    g.circleCount++;
}

void CommandCompiler::pushLine(CompiledBatches& out, const Point& from, const Point& to) {
    SDFQuadInstance inst{};
    float x0 = from.x, y0 = from.y;
    float x1 = to.x, y1 = to.y;
    applyTransform(x0, y0);
    applyTransform(x1, y1);

    float dx = x1 - x0, dy = y1 - y0;
    float len = std::sqrt(dx * dx + dy * dy);
    float midX = (x0 + x1) * 0.5f, midY = (y0 + y1) * 0.5f;

    float strokeW = current_.stroke.width * linearScale();
    float halfW = strokeW * 0.5f + 1.0f;

    inst.rect[0] = midX - len * 0.5f - halfW;
    inst.rect[1] = midY - halfW;
    inst.rect[2] = len + halfW * 2.0f;
    inst.rect[3] = halfW * 2.0f;

    float angle = (len > 1e-6f) ? std::atan2(dy, dx) : 0.0f;
    inst.corners[0] = std::cos(angle);
    inst.corners[1] = std::sin(angle);
    inst.corners[2] = 0.0f;
    inst.corners[3] = 0.0f;

    inst.strokeWidth = strokeW;
    inst.strokeColor[0] = current_.stroke.color.r;
    inst.strokeColor[1] = current_.stroke.color.g;
    inst.strokeColor[2] = current_.stroke.color.b;
    inst.strokeColor[3] = current_.stroke.color.a;
    inst.opacity = current_.opacity;
    inst.viewport[0] = out.viewportWidth;
    inst.viewport[1] = out.viewportHeight;
    inst.rotation = 0.f;
    inst._pad[0] = inst._pad[1] = inst._pad[2] = 0.f;

    out.lines.push_back(inst);
    auto& g = out.groups.back();
    g.drawOps.push_back({DrawOpType::Line, g.lineCount, 1});
    g.lineCount++;
}

void CommandCompiler::pushText(CompiledBatches& out, const std::string& text,
                               const Point& position,
                               HorizontalAlignment hAlign, VerticalAlignment vAlign) {
    if (!atlas_) return;

    auto fontIndex = atlas_->ensureFontLoaded(current_.textStyle.fontName, current_.textStyle.weight);
    if (!fontIndex) return;

    const float fontSize = current_.textStyle.size * horizontalScale();
    Color textColor = current_.fill.color;
    textColor.a *= current_.opacity;

    auto sz = atlas_->measureText(text, fontSize, *fontIndex);

    std::vector<GlyphInstance> glyphs;

    if (isAxisAligned()) {
        float x = position.x, y = position.y;
        applyTransform(x, y);

        float drawX = x;
        if (hAlign == HorizontalAlignment::center) drawX -= sz.width * 0.5f;
        else if (hAlign == HorizontalAlignment::trailing) drawX -= sz.width;

        float drawY = y;
        if (vAlign == VerticalAlignment::center) drawY += fontSize * 0.35f;
        else if (vAlign == VerticalAlignment::top) drawY += fontSize;

        glyphs = atlas_->layoutText(text, drawX, drawY, fontSize, textColor,
                                    out.viewportWidth, out.viewportHeight, *fontIndex);
    } else {
        float drawX = position.x;
        if (hAlign == HorizontalAlignment::center) drawX -= sz.width * 0.5f;
        else if (hAlign == HorizontalAlignment::trailing) drawX -= sz.width;

        float drawY = position.y;
        if (vAlign == VerticalAlignment::center) drawY += fontSize * 0.35f;
        else if (vAlign == VerticalAlignment::top) drawY += fontSize;

        glyphs = atlas_->layoutText(text, drawX, drawY, fontSize, textColor,
                                    out.viewportWidth, out.viewportHeight, *fontIndex);
        for (auto& g : glyphs) {
            transformGlyphInstance(g);
        }
    }

    if (glyphs.empty()) return;

    auto& gr = out.groups.back();
    std::sort(glyphs.begin(), glyphs.end(), [](const GlyphInstance& a, const GlyphInstance& b) {
        return a.atlasPage < b.atlasPage;
    });
    uint8_t curPage = static_cast<uint8_t>(glyphs[0].atlasPage);
    uint32_t runStart = 0;
    for (uint32_t i = 1; i <= static_cast<uint32_t>(glyphs.size()); ++i) {
        uint8_t pg = (i < glyphs.size()) ? static_cast<uint8_t>(glyphs[i].atlasPage) : 255;
        if (pg != curPage) {
            uint32_t count = i - runStart;
            gr.drawOps.push_back({DrawOpType::Glyph, gr.glyphCount, count, curPage});
            out.glyphs.insert(out.glyphs.end(), glyphs.begin() + runStart, glyphs.begin() + i);
            gr.glyphCount += count;
            runStart = i;
            curPage = pg;
        }
    }
}

void CommandCompiler::pushPath(CompiledBatches& out, const Path& path) {
    auto keyOpt = makePathTessCacheKey(path, out.viewportWidth, out.viewportHeight);
    if (keyOpt) {
        auto idxIt = pathTessIndex_.find(*keyOpt);
        if (idxIt != pathTessIndex_.end()) {
            auto lruIt = idxIt->second;
            pathTessLru_.splice(pathTessLru_.end(), pathTessLru_, lruIt);
            const auto& cached = lruIt->second;
            size_t pathStart = out.pathVertices.size();
            auto& g = out.groups.back();
            out.pathVertices.insert(out.pathVertices.end(), cached.begin(), cached.end());
            g.pathCount += static_cast<uint32_t>(cached.size());
            const size_t pathEnd = out.pathVertices.size();
            if (pathEnd > pathStart) {
                g.drawOps.push_back({DrawOpType::Path, static_cast<uint32_t>(pathStart - g.pathOffset),
                                     static_cast<uint32_t>(pathEnd - pathStart)});
            }
            return;
        }
    }

    auto subpaths = PathFlattener::flattenSubpaths(path);
    for (auto& sub : subpaths) {
        for (auto& p : sub) applyTransform(p.x, p.y);
    }

    std::vector<PathVertex> built;
    auto appendPathVerts = [&](TessellatedPath&& t) {
        const auto n = t.vertices.size();
        if (n == 0) return;
        built.insert(built.end(), t.vertices.begin(), t.vertices.end());
    };

    if (current_.fill.type != FillStyle::Type::None) {
        Color fc = current_.fill.color;
        fc.a *= current_.opacity;
        for (const auto& sub : subpaths) {
            if (sub.size() < 3) continue;
            appendPathVerts(PathFlattener::tessellateFill(sub, fc, out.viewportWidth, out.viewportHeight));
        }
    }

    if (current_.stroke.type != StrokeStyle::Type::None && current_.stroke.width > 0) {
        Color sc = current_.stroke.color;
        sc.a *= current_.opacity;
        const float sw = current_.stroke.width * linearScale();
        for (const auto& sub : subpaths) {
            if (sub.size() < 2) continue;
            appendPathVerts(PathFlattener::tessellateStroke(sub, sw, sc, out.viewportWidth, out.viewportHeight));
        }
    }

    size_t pathStart = out.pathVertices.size();
    auto& g = out.groups.back();
    out.pathVertices.insert(out.pathVertices.end(), built.begin(), built.end());
    g.pathCount += static_cast<uint32_t>(built.size());

    if (keyOpt && !built.empty() && built.size() <= 512000) {
        while (pathTessIndex_.size() >= kPathTessCacheMaxEntries) {
            auto& oldest = pathTessLru_.front();
            pathTessIndex_.erase(oldest.first);
            pathTessLru_.pop_front();
        }
        pathTessLru_.push_back({*keyOpt, std::move(built)});
        pathTessIndex_[*keyOpt] = std::prev(pathTessLru_.end());
    }

    const size_t pathEnd = out.pathVertices.size();
    if (pathEnd > pathStart) {
        g.drawOps.push_back({DrawOpType::Path, static_cast<uint32_t>(pathStart - g.pathOffset),
                             static_cast<uint32_t>(pathEnd - pathStart)});
    }
}

void CommandCompiler::pushTextBox(CompiledBatches& out, const std::string& text,
                                  const Point& position,
                                  float maxWidth, HorizontalAlignment hAlign) {
    if (!atlas_) return;

    auto fontIndex = atlas_->ensureFontLoaded(current_.textStyle.fontName, current_.textStyle.weight);
    if (!fontIndex) return;

    const float fontSize = current_.textStyle.size * horizontalScale();
    const float scaledMaxWidth = maxWidth * horizontalScale();
    Color textColor = current_.fill.color;
    textColor.a *= current_.opacity;

    std::vector<GlyphInstance> glyphs;

    if (isAxisAligned()) {
        float x = position.x, y = position.y;
        applyTransform(x, y);
        glyphs = atlas_->layoutTextBox(text, x, y + fontSize, fontSize, scaledMaxWidth, textColor,
                                        out.viewportWidth, out.viewportHeight, hAlign, *fontIndex);
    } else {
        glyphs = atlas_->layoutTextBox(text, position.x, position.y + fontSize, fontSize,
                                        scaledMaxWidth, textColor, out.viewportWidth, out.viewportHeight,
                                        hAlign, *fontIndex);
        for (auto& g : glyphs) {
            transformGlyphInstance(g);
        }
    }

    if (glyphs.empty()) return;

    auto& gr = out.groups.back();
    std::sort(glyphs.begin(), glyphs.end(), [](const GlyphInstance& a, const GlyphInstance& b) {
        return a.atlasPage < b.atlasPage;
    });
    uint8_t curPage = static_cast<uint8_t>(glyphs[0].atlasPage);
    uint32_t runStart = 0;
    for (uint32_t i = 1; i <= static_cast<uint32_t>(glyphs.size()); ++i) {
        uint8_t pg = (i < glyphs.size()) ? static_cast<uint8_t>(glyphs[i].atlasPage) : 255;
        if (pg != curPage) {
            uint32_t count = i - runStart;
            gr.drawOps.push_back({DrawOpType::Glyph, gr.glyphCount, count, curPage});
            out.glyphs.insert(out.glyphs.end(), glyphs.begin() + runStart, glyphs.begin() + i);
            gr.glyphCount += count;
            runStart = i;
            curPage = pg;
        }
    }
}

ImageInstance CommandCompiler::makeImageInstance(const Rect& rect, float alpha) const {
    ImageInstance inst{};
    inst.screenRect[0] = rect.x;
    inst.screenRect[1] = rect.y;
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
    inst.rotation = 0.f;
    inst._pad = 0.f;
    return inst;
}

void CommandCompiler::pushImage(CompiledBatches& out, int imageId, const Rect& rect,
                                ImageFit fit, const CornerRadius& cr, float alpha) {
    const float cx = rect.x + rect.width * 0.5f;
    const float cy = rect.y + rect.height * 0.5f;
    float ox = cx, oy = cy;
    applyTransform(ox, oy);
    const float hw = rect.width * 0.5f;
    const float hh = rect.height * 0.5f;
    const float hwScr = std::hypot(current_.m00 * hw, current_.m10 * hw);
    const float hhScr = std::hypot(current_.m01 * hh, current_.m11 * hh);
    const float rot = std::atan2(current_.m10, current_.m00);
    Rect transformed{ox - hwScr, oy - hhScr, 2.f * hwScr, 2.f * hhScr};

    ImageDrawCmd draw;
    draw.imageId = imageId;
    draw.instance = makeImageInstance(transformed, alpha);
    draw.instance.rotation = rot;
    draw.instance.viewport[0] = out.viewportWidth;
    draw.instance.viewport[1] = out.viewportHeight;
    auto& g = out.groups.back();
    g.drawOps.push_back({DrawOpType::Image, static_cast<uint32_t>(g.imageDraws.size()), 1});
    g.imageDraws.push_back(std::move(draw));
}

void CommandCompiler::pushImagePath(CompiledBatches& out, const std::string& imgPath, const Rect& rect,
                                    ImageFit fit, const CornerRadius& cr, float alpha) {
    const float cx = rect.x + rect.width * 0.5f;
    const float cy = rect.y + rect.height * 0.5f;
    float ox = cx, oy = cy;
    applyTransform(ox, oy);
    const float hw = rect.width * 0.5f;
    const float hh = rect.height * 0.5f;
    const float hwScr = std::hypot(current_.m00 * hw, current_.m10 * hw);
    const float hhScr = std::hypot(current_.m01 * hh, current_.m11 * hh);
    const float rot = std::atan2(current_.m10, current_.m00);
    Rect transformed{ox - hwScr, oy - hhScr, 2.f * hwScr, 2.f * hhScr};

    ImageDrawCmd draw;
    draw.path = imgPath;
    draw.instance = makeImageInstance(transformed, alpha);
    draw.instance.rotation = rot;
    draw.instance.viewport[0] = out.viewportWidth;
    draw.instance.viewport[1] = out.viewportHeight;
    auto& g = out.groups.back();
    g.drawOps.push_back({DrawOpType::Image, static_cast<uint32_t>(g.imageDraws.size()), 1});
    g.imageDraws.push_back(std::move(draw));
}

} // namespace flux
