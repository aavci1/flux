#include <Flux/Graphics/NanoVGBackend.hpp>
#include <Flux/Core/FontDiscovery.hpp>
#include <Flux/Core/Log.hpp>
#include <nanovg.h>
#include <cmath>

namespace flux {

void NanoVGBackend::execute(const RenderCommandBuffer& buffer) {
    for (const auto& cmd : buffer.commands()) {
        std::visit([this](const auto& c) { dispatch(c); }, cmd);
    }
}

// State
void NanoVGBackend::dispatch(const CmdSave&) { nvgSave(nvg_); }
void NanoVGBackend::dispatch(const CmdRestore&) { nvgRestore(nvg_); }

// Transforms
void NanoVGBackend::dispatch(const CmdTranslate& c) { nvgTranslate(nvg_, c.x, c.y); }
void NanoVGBackend::dispatch(const CmdRotate& c) { nvgRotate(nvg_, c.angle); }
void NanoVGBackend::dispatch(const CmdScale& c) { nvgScale(nvg_, c.sx, c.sy); }

// Styles
void NanoVGBackend::dispatch(const CmdSetOpacity& c) { nvgGlobalAlpha(nvg_, c.opacity); }

void NanoVGBackend::dispatch(const CmdSetFillStyle& c) {
    currentFill_ = c.style;
    NVGpaint paint;
    switch (c.style.type) {
        case FillStyle::Type::None:
            paint = nvgLinearGradient(nvg_, 0, 0, 0, 0,
                nvgRGBAf(0, 0, 0, 0), nvgRGBAf(0, 0, 0, 0));
            break;
        case FillStyle::Type::Solid:
            paint = nvgLinearGradient(nvg_, 0, 0, 0, 0,
                nvgRGBAf(c.style.color.r, c.style.color.g, c.style.color.b, c.style.color.a),
                nvgRGBAf(c.style.color.r, c.style.color.g, c.style.color.b, c.style.color.a));
            break;
        case FillStyle::Type::LinearGradient:
            paint = nvgLinearGradient(nvg_,
                c.style.startPoint.x, c.style.startPoint.y,
                c.style.endPoint.x, c.style.endPoint.y,
                nvgRGBAf(c.style.startColor.r, c.style.startColor.g, c.style.startColor.b, c.style.startColor.a),
                nvgRGBAf(c.style.endColor.r, c.style.endColor.g, c.style.endColor.b, c.style.endColor.a));
            break;
        case FillStyle::Type::RadialGradient:
            paint = nvgRadialGradient(nvg_,
                c.style.center.x, c.style.center.y,
                c.style.innerRadius, c.style.outerRadius,
                nvgRGBAf(c.style.startColor.r, c.style.startColor.g, c.style.startColor.b, c.style.startColor.a),
                nvgRGBAf(c.style.endColor.r, c.style.endColor.g, c.style.endColor.b, c.style.endColor.a));
            break;
        case FillStyle::Type::BoxGradient:
            paint = nvgBoxGradient(nvg_,
                c.style.bounds.x, c.style.bounds.y,
                c.style.bounds.width, c.style.bounds.height,
                c.style.cornerRadius, c.style.feather,
                nvgRGBAf(c.style.startColor.r, c.style.startColor.g, c.style.startColor.b, c.style.startColor.a),
                nvgRGBAf(c.style.endColor.r, c.style.endColor.g, c.style.endColor.b, c.style.endColor.a));
            break;
        case FillStyle::Type::ImagePattern:
            paint = nvgImagePattern(nvg_,
                c.style.imageOrigin.x, c.style.imageOrigin.y,
                c.style.imageSize.width, c.style.imageSize.height,
                c.style.imageAngle, c.style.imageId, c.style.imageAlpha);
            break;
    }
    nvgFillPaint(nvg_, paint);
    int winding = (c.style.winding == PathWinding::Clockwise) ? NVG_CW : NVG_CCW;
    nvgPathWinding(nvg_, winding);
}

void NanoVGBackend::dispatch(const CmdSetStrokeStyle& c) {
    currentStroke_ = c.style;
    nvgStrokeColor(nvg_, nvgRGBAf(c.style.color.r, c.style.color.g, c.style.color.b, c.style.color.a));
    nvgStrokeWidth(nvg_, c.style.width);
    int cap = NVG_BUTT;
    switch (c.style.cap) {
        case LineCap::Butt: cap = NVG_BUTT; break;
        case LineCap::Round: cap = NVG_ROUND; break;
        case LineCap::Square: cap = NVG_SQUARE; break;
    }
    nvgLineCap(nvg_, cap);
    int join = NVG_MITER;
    switch (c.style.join) {
        case LineJoin::Miter: join = NVG_MITER; break;
        case LineJoin::Round: join = NVG_ROUND; break;
        case LineJoin::Bevel: join = NVG_BEVEL; break;
    }
    nvgLineJoin(nvg_, join);
    nvgMiterLimit(nvg_, c.style.miterLimit);
}

void NanoVGBackend::dispatch(const CmdSetTextStyle& c) {
    currentText_ = c.style;
    int font = resolveFont(c.style.fontName, c.style.weight);
    nvgFontFaceId(nvg_, font);
    nvgFontSize(nvg_, c.style.size);
    nvgTextLetterSpacing(nvg_, c.style.letterSpacing);
    nvgTextLineHeight(nvg_, c.style.lineHeight);
}

// Shapes
void NanoVGBackend::dispatch(const CmdDrawRect& c) {
    nvgBeginPath(nvg_);
    if (c.cornerRadius.isZero()) {
        nvgRect(nvg_, c.bounds.x, c.bounds.y, c.bounds.width, c.bounds.height);
    } else if (c.cornerRadius.isUniform()) {
        nvgRoundedRect(nvg_, c.bounds.x, c.bounds.y, c.bounds.width, c.bounds.height, c.cornerRadius.topLeft);
    } else {
        nvgRoundedRectVarying(nvg_, c.bounds.x, c.bounds.y, c.bounds.width, c.bounds.height,
            c.cornerRadius.topLeft, c.cornerRadius.topRight,
            c.cornerRadius.bottomRight, c.cornerRadius.bottomLeft);
    }
    applyFill();
    applyStroke();
}

void NanoVGBackend::dispatch(const CmdDrawCircle& c) {
    nvgBeginPath(nvg_);
    nvgCircle(nvg_, c.center.x, c.center.y, c.radius);
    applyFill();
    applyStroke();
}

void NanoVGBackend::dispatch(const CmdDrawLine& c) {
    nvgBeginPath(nvg_);
    nvgMoveTo(nvg_, c.from.x, c.from.y);
    nvgLineTo(nvg_, c.to.x, c.to.y);
    applyStroke();
}

void NanoVGBackend::dispatch(const CmdDrawPath& c) {
    if (c.path.isEmpty()) return;
    nvgBeginPath(nvg_);
    for (const auto& cmd : c.path.commands_) {
        switch (cmd.type) {
            case Path::CommandType::SetWinding: {
                int w = (cmd.winding == PathWinding::Clockwise) ? NVG_CW : NVG_CCW;
                nvgPathWinding(nvg_, w);
                break;
            }
            case Path::CommandType::MoveTo:
                if (cmd.data.size() >= 2) nvgMoveTo(nvg_, cmd.data[0], cmd.data[1]);
                break;
            case Path::CommandType::LineTo:
                if (cmd.data.size() >= 2) nvgLineTo(nvg_, cmd.data[0], cmd.data[1]);
                break;
            case Path::CommandType::QuadTo:
                if (cmd.data.size() >= 4) nvgQuadTo(nvg_, cmd.data[0], cmd.data[1], cmd.data[2], cmd.data[3]);
                break;
            case Path::CommandType::BezierTo:
                if (cmd.data.size() >= 6)
                    nvgBezierTo(nvg_, cmd.data[0], cmd.data[1], cmd.data[2], cmd.data[3], cmd.data[4], cmd.data[5]);
                break;
            case Path::CommandType::ArcTo:
                if (cmd.data.size() >= 5)
                    nvgArcTo(nvg_, cmd.data[0], cmd.data[1], cmd.data[2], cmd.data[3], cmd.data[4]);
                break;
            case Path::CommandType::Arc:
                if (cmd.data.size() >= 6) {
                    bool cw = cmd.data[5] > 0.5f;
                    nvgArc(nvg_, cmd.data[0], cmd.data[1], cmd.data[2], cmd.data[3], cmd.data[4], cw ? NVG_CW : NVG_CCW);
                }
                break;
            case Path::CommandType::Rect:
                if (cmd.data.size() >= 8) {
                    CornerRadius cr(cmd.data[4], cmd.data[5], cmd.data[6], cmd.data[7]);
                    if (cr.isZero()) nvgRect(nvg_, cmd.data[0], cmd.data[1], cmd.data[2], cmd.data[3]);
                    else if (cr.isUniform()) nvgRoundedRect(nvg_, cmd.data[0], cmd.data[1], cmd.data[2], cmd.data[3], cr.topLeft);
                    else nvgRoundedRectVarying(nvg_, cmd.data[0], cmd.data[1], cmd.data[2], cmd.data[3],
                             cr.topLeft, cr.topRight, cr.bottomRight, cr.bottomLeft);
                }
                break;
            case Path::CommandType::Circle:
                if (cmd.data.size() >= 3) nvgCircle(nvg_, cmd.data[0], cmd.data[1], cmd.data[2]);
                break;
            case Path::CommandType::Ellipse:
                if (cmd.data.size() >= 4) nvgEllipse(nvg_, cmd.data[0], cmd.data[1], cmd.data[2], cmd.data[3]);
                break;
            case Path::CommandType::Close:
                nvgClosePath(nvg_);
                break;
        }
    }
    applyFill();
    applyStroke();
}

// Text
void NanoVGBackend::dispatch(const CmdDrawText& c) {
    int align = 0;
    switch (c.hAlign) {
        case HorizontalAlignment::leading: align |= NVG_ALIGN_LEFT; break;
        case HorizontalAlignment::center: align |= NVG_ALIGN_CENTER; break;
        case HorizontalAlignment::trailing: align |= NVG_ALIGN_RIGHT; break;
        case HorizontalAlignment::justify: align |= NVG_ALIGN_LEFT; break;
    }
    switch (c.vAlign) {
        case VerticalAlignment::top: align |= NVG_ALIGN_TOP; break;
        case VerticalAlignment::center: align |= NVG_ALIGN_MIDDLE; break;
        case VerticalAlignment::bottom: align |= NVG_ALIGN_BOTTOM; break;
    }
    nvgTextAlign(nvg_, align);
    nvgText(nvg_, c.position.x, c.position.y, c.text.c_str(), nullptr);
}

void NanoVGBackend::dispatch(const CmdDrawTextBox& c) {
    int align = NVG_ALIGN_TOP;
    switch (c.hAlign) {
        case HorizontalAlignment::leading: align |= NVG_ALIGN_LEFT; break;
        case HorizontalAlignment::center: align |= NVG_ALIGN_CENTER; break;
        case HorizontalAlignment::trailing: align |= NVG_ALIGN_RIGHT; break;
        case HorizontalAlignment::justify: align |= NVG_ALIGN_LEFT; break;
    }
    nvgTextAlign(nvg_, align);
    nvgTextBox(nvg_, c.position.x, c.position.y, c.maxWidth, c.text.c_str(), nullptr);
}

// Images
void NanoVGBackend::dispatch(const CmdDrawImage& c) {
    int w, h;
    nvgImageSize(nvg_, c.imageId, &w, &h);
    if (w <= 0 || h <= 0) return;

    Rect imgRect = c.rect;
    float iw = static_cast<float>(w), ih = static_cast<float>(h);
    switch (c.fit) {
        case ImageFit::Fill: break;
        case ImageFit::Cover: {
            float s = std::max(c.rect.width / iw, c.rect.height / ih);
            float sw = iw * s, sh = ih * s;
            imgRect = {c.rect.x + (c.rect.width - sw) / 2, c.rect.y + (c.rect.height - sh) / 2, sw, sh};
            break;
        }
        case ImageFit::Contain: {
            float s = std::min(c.rect.width / iw, c.rect.height / ih);
            float sw = iw * s, sh = ih * s;
            imgRect = {c.rect.x + (c.rect.width - sw) / 2, c.rect.y + (c.rect.height - sh) / 2, sw, sh};
            break;
        }
        case ImageFit::None: {
            imgRect = {c.rect.x + (c.rect.width - iw) / 2, c.rect.y + (c.rect.height - ih) / 2, iw, ih};
            break;
        }
    }

    NVGpaint paint = nvgImagePattern(nvg_, imgRect.x, imgRect.y, imgRect.width, imgRect.height, 0, c.imageId, c.alpha);
    nvgBeginPath(nvg_);
    if (c.cornerRadius.isZero()) {
        nvgRect(nvg_, c.rect.x, c.rect.y, c.rect.width, c.rect.height);
    } else if (c.cornerRadius.isUniform()) {
        nvgRoundedRect(nvg_, c.rect.x, c.rect.y, c.rect.width, c.rect.height, c.cornerRadius.topLeft);
    } else {
        nvgRoundedRectVarying(nvg_, c.rect.x, c.rect.y, c.rect.width, c.rect.height,
            c.cornerRadius.topLeft, c.cornerRadius.topRight, c.cornerRadius.bottomRight, c.cornerRadius.bottomLeft);
    }
    nvgFillPaint(nvg_, paint);
    nvgFill(nvg_);
}

void NanoVGBackend::dispatch(const CmdDrawImagePath& c) {
    auto it = imageCache_.find(c.path);
    int imageId = -1;
    if (it != imageCache_.end()) {
        imageId = it->second;
    } else {
        imageId = nvgCreateImage(nvg_, c.path.c_str(), 0);
        if (imageId >= 0) imageCache_[c.path] = imageId;
    }
    if (imageId >= 0) {
        dispatch(CmdDrawImage{imageId, c.rect, c.fit, c.cornerRadius, c.alpha});
    }
}

// Clipping
void NanoVGBackend::dispatch(const CmdClipPath& c) {
    Rect b = c.path.getBounds();
    nvgSave(nvg_);
    nvgIntersectScissor(nvg_, b.x, b.y, b.width, b.height);
}

// Clear
void NanoVGBackend::dispatch(const CmdClear& c) {
    nvgBeginPath(nvg_);
    nvgRect(nvg_, 0, 0, 99999, 99999);
    nvgFillColor(nvg_, nvgRGBAf(c.color.r, c.color.g, c.color.b, c.color.a));
    nvgFill(nvg_);
}

// Helpers
void NanoVGBackend::applyFill() {
    if (currentFill_.type != FillStyle::Type::None) nvgFill(nvg_);
}

void NanoVGBackend::applyStroke() {
    if (currentStroke_.type != StrokeStyle::Type::None) nvgStroke(nvg_);
}

int NanoVGBackend::resolveFont(const std::string& name, FontWeight weight) {
    std::string key = name + "_" + std::to_string(static_cast<int>(weight));
    auto it = fontCache_.find(key);
    if (it != fontCache_.end()) return it->second;

    std::string family = name;
    if (name == "default" || name == "sans") family = "Helvetica";
    auto discovered = FontDiscovery::findFontPath(family, weight);
    int font = -1;
    if (discovered) {
        font = nvgCreateFont(nvg_, key.c_str(), discovered->c_str());
    }
    if (font == -1) font = 0;
    fontCache_[key] = font;
    return font;
}

} // namespace flux
