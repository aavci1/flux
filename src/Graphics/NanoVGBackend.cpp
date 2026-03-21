#include <Flux/Graphics/NanoVGBackend.hpp>
#include <Flux/Graphics/FontProvider.hpp>
#include <Flux/Core/Log.hpp>
#include <nanovg.h>
#include <cmath>

namespace flux {

void NanoVGBackend::execute(const RenderCommandBuffer& buffer) {
    auto r = buffer.reader();
    while (r.hasNext()) {
        CmdOp op = r.nextOp();
        switch (op) {
            case CmdOp::Save:    nvgSave(nvg_);    break;
            case CmdOp::Restore: nvgRestore(nvg_);  break;

            case CmdOp::Translate: {
                float x = r.readFloat(), y = r.readFloat();
                nvgTranslate(nvg_, x, y);
                break;
            }
            case CmdOp::Rotate:
                nvgRotate(nvg_, r.readFloat());
                break;
            case CmdOp::Scale: {
                float sx = r.readFloat(), sy = r.readFloat();
                nvgScale(nvg_, sx, sy);
                break;
            }

            case CmdOp::SetOpacity:
                nvgGlobalAlpha(nvg_, r.readFloat());
                break;

            case CmdOp::SetFillStyle: {
                const FillStyle& s = buffer.fillStyle(r.readUint32());
                currentFill_ = s;
                NVGpaint paint;
                switch (s.type) {
                    case FillStyle::Type::None:
                        paint = nvgLinearGradient(nvg_, 0, 0, 0, 0,
                            nvgRGBAf(0, 0, 0, 0), nvgRGBAf(0, 0, 0, 0));
                        break;
                    case FillStyle::Type::Solid:
                        paint = nvgLinearGradient(nvg_, 0, 0, 0, 0,
                            nvgRGBAf(s.color.r, s.color.g, s.color.b, s.color.a),
                            nvgRGBAf(s.color.r, s.color.g, s.color.b, s.color.a));
                        break;
                    case FillStyle::Type::LinearGradient:
                        paint = nvgLinearGradient(nvg_,
                            s.startPoint.x, s.startPoint.y, s.endPoint.x, s.endPoint.y,
                            nvgRGBAf(s.startColor.r, s.startColor.g, s.startColor.b, s.startColor.a),
                            nvgRGBAf(s.endColor.r, s.endColor.g, s.endColor.b, s.endColor.a));
                        break;
                    case FillStyle::Type::RadialGradient:
                        paint = nvgRadialGradient(nvg_,
                            s.center.x, s.center.y, s.innerRadius, s.outerRadius,
                            nvgRGBAf(s.startColor.r, s.startColor.g, s.startColor.b, s.startColor.a),
                            nvgRGBAf(s.endColor.r, s.endColor.g, s.endColor.b, s.endColor.a));
                        break;
                    case FillStyle::Type::BoxGradient:
                        paint = nvgBoxGradient(nvg_,
                            s.bounds.x, s.bounds.y, s.bounds.width, s.bounds.height,
                            s.cornerRadius, s.feather,
                            nvgRGBAf(s.startColor.r, s.startColor.g, s.startColor.b, s.startColor.a),
                            nvgRGBAf(s.endColor.r, s.endColor.g, s.endColor.b, s.endColor.a));
                        break;
                    case FillStyle::Type::ImagePattern:
                        paint = nvgImagePattern(nvg_,
                            s.imageOrigin.x, s.imageOrigin.y,
                            s.imageSize.width, s.imageSize.height,
                            s.imageAngle, s.imageId, s.imageAlpha);
                        break;
                }
                nvgFillPaint(nvg_, paint);
                int winding = (s.winding == PathWinding::Clockwise) ? NVG_CW : NVG_CCW;
                nvgPathWinding(nvg_, winding);
                break;
            }

            case CmdOp::SetStrokeStyle: {
                const StrokeStyle& s = buffer.strokeStyle(r.readUint32());
                currentStroke_ = s;
                nvgStrokeColor(nvg_, nvgRGBAf(s.color.r, s.color.g, s.color.b, s.color.a));
                nvgStrokeWidth(nvg_, s.width);
                int cap = NVG_BUTT;
                switch (s.cap) {
                    case LineCap::Butt:   cap = NVG_BUTT;   break;
                    case LineCap::Round:  cap = NVG_ROUND;  break;
                    case LineCap::Square: cap = NVG_SQUARE; break;
                }
                nvgLineCap(nvg_, cap);
                int join = NVG_MITER;
                switch (s.join) {
                    case LineJoin::Miter: join = NVG_MITER; break;
                    case LineJoin::Round: join = NVG_ROUND; break;
                    case LineJoin::Bevel: join = NVG_BEVEL; break;
                }
                nvgLineJoin(nvg_, join);
                nvgMiterLimit(nvg_, s.miterLimit);
                break;
            }

            case CmdOp::SetTextStyle: {
                const TextStyle& s = buffer.textStyle(r.readUint32());
                currentText_ = s;
                int font = resolveFont(s.fontName, s.weight);
                nvgFontFaceId(nvg_, font);
                nvgFontSize(nvg_, s.size);
                nvgTextLetterSpacing(nvg_, s.letterSpacing);
                nvgTextLineHeight(nvg_, s.lineHeight);
                break;
            }

            case CmdOp::DrawRect: {
                Rect b{r.readFloat(), r.readFloat(), r.readFloat(), r.readFloat()};
                CornerRadius cr{r.readFloat(), r.readFloat(), r.readFloat(), r.readFloat()};
                nvgBeginPath(nvg_);
                if (cr.isZero()) {
                    nvgRect(nvg_, b.x, b.y, b.width, b.height);
                } else if (cr.isUniform()) {
                    nvgRoundedRect(nvg_, b.x, b.y, b.width, b.height, cr.topLeft);
                } else {
                    nvgRoundedRectVarying(nvg_, b.x, b.y, b.width, b.height,
                        cr.topLeft, cr.topRight, cr.bottomRight, cr.bottomLeft);
                }
                applyFill();
                applyStroke();
                break;
            }

            case CmdOp::DrawCircle: {
                float cx = r.readFloat(), cy = r.readFloat(), radius = r.readFloat();
                nvgBeginPath(nvg_);
                nvgCircle(nvg_, cx, cy, radius);
                applyFill();
                applyStroke();
                break;
            }

            case CmdOp::DrawLine: {
                float x0 = r.readFloat(), y0 = r.readFloat();
                float x1 = r.readFloat(), y1 = r.readFloat();
                nvgBeginPath(nvg_);
                nvgMoveTo(nvg_, x0, y0);
                nvgLineTo(nvg_, x1, y1);
                applyStroke();
                break;
            }

            case CmdOp::DrawPath: {
                const Path& path = buffer.path(r.readUint32());
                drawPath(path);
                break;
            }

            case CmdOp::DrawText: {
                uint32_t strId = r.readUint32();
                float px = r.readFloat(), py = r.readFloat();
                auto hAlign = static_cast<HorizontalAlignment>(r.readUint32());
                auto vAlign = static_cast<VerticalAlignment>(r.readUint32());
                const std::string& text = buffer.str(strId);
                int align = 0;
                switch (hAlign) {
                    case HorizontalAlignment::leading:  align |= NVG_ALIGN_LEFT;   break;
                    case HorizontalAlignment::center:   align |= NVG_ALIGN_CENTER; break;
                    case HorizontalAlignment::trailing: align |= NVG_ALIGN_RIGHT;  break;
                    case HorizontalAlignment::justify:  align |= NVG_ALIGN_LEFT;   break;
                }
                switch (vAlign) {
                    case VerticalAlignment::top:    align |= NVG_ALIGN_TOP;    break;
                    case VerticalAlignment::center: align |= NVG_ALIGN_MIDDLE; break;
                    case VerticalAlignment::bottom: align |= NVG_ALIGN_BOTTOM; break;
                }
                nvgTextAlign(nvg_, align);
                nvgText(nvg_, px, py, text.c_str(), nullptr);
                break;
            }

            case CmdOp::DrawTextBox: {
                uint32_t strId = r.readUint32();
                float px = r.readFloat(), py = r.readFloat();
                float maxWidth = r.readFloat();
                auto hAlign = static_cast<HorizontalAlignment>(r.readUint32());
                const std::string& text = buffer.str(strId);
                int align = NVG_ALIGN_TOP;
                switch (hAlign) {
                    case HorizontalAlignment::leading:  align |= NVG_ALIGN_LEFT;   break;
                    case HorizontalAlignment::center:   align |= NVG_ALIGN_CENTER; break;
                    case HorizontalAlignment::trailing: align |= NVG_ALIGN_RIGHT;  break;
                    case HorizontalAlignment::justify:  align |= NVG_ALIGN_LEFT;   break;
                }
                nvgTextAlign(nvg_, align);
                nvgTextBox(nvg_, px, py, maxWidth, text.c_str(), nullptr);
                break;
            }

            case CmdOp::DrawImage: {
                int imageId = r.readInt32();
                Rect rect{r.readFloat(), r.readFloat(), r.readFloat(), r.readFloat()};
                auto fit = static_cast<ImageFit>(r.readUint32());
                CornerRadius cr{r.readFloat(), r.readFloat(), r.readFloat(), r.readFloat()};
                float alpha = r.readFloat();
                drawImage(imageId, rect, fit, cr, alpha);
                break;
            }

            case CmdOp::DrawImagePath: {
                uint32_t pathStrId = r.readUint32();
                Rect rect{r.readFloat(), r.readFloat(), r.readFloat(), r.readFloat()};
                auto fit = static_cast<ImageFit>(r.readUint32());
                CornerRadius cr{r.readFloat(), r.readFloat(), r.readFloat(), r.readFloat()};
                float alpha = r.readFloat();
                const std::string& imgPath = buffer.str(pathStrId);
                auto it = imageCache_.find(imgPath);
                int imgId = -1;
                if (it != imageCache_.end()) {
                    imgId = it->second;
                } else {
                    imgId = nvgCreateImage(nvg_, imgPath.c_str(), 0);
                    if (imgId >= 0) imageCache_[imgPath] = imgId;
                }
                if (imgId >= 0) {
                    drawImage(imgId, rect, fit, cr, alpha);
                }
                break;
            }

            case CmdOp::ClipPath: {
                const Path& clipPath = buffer.path(r.readUint32());
                Rect b = clipPath.getBounds();
                nvgIntersectScissor(nvg_, b.x, b.y, b.width, b.height);
                break;
            }

            case CmdOp::Clear: {
                float cr = r.readFloat(), cg = r.readFloat(), cb = r.readFloat(), ca = r.readFloat();
                nvgBeginPath(nvg_);
                nvgRect(nvg_, 0, 0, 99999, 99999);
                nvgFillColor(nvg_, nvgRGBAf(cr, cg, cb, ca));
                nvgFill(nvg_);
                break;
            }
        }
    }
}

void NanoVGBackend::drawPath(const Path& path) {
    if (path.isEmpty()) return;
    nvgBeginPath(nvg_);
    for (size_t ci = 0; ci < path.commandCount(); ++ci) {
        auto cv = path.command(ci);
        switch (cv.type) {
            case Path::CommandType::SetWinding: {
                int w = (cv.winding == PathWinding::Clockwise) ? NVG_CW : NVG_CCW;
                nvgPathWinding(nvg_, w);
                break;
            }
            case Path::CommandType::MoveTo:
                if (cv.dataCount >= 2) nvgMoveTo(nvg_, cv.data[0], cv.data[1]);
                break;
            case Path::CommandType::LineTo:
                if (cv.dataCount >= 2) nvgLineTo(nvg_, cv.data[0], cv.data[1]);
                break;
            case Path::CommandType::QuadTo:
                if (cv.dataCount >= 4)
                    nvgQuadTo(nvg_, cv.data[0], cv.data[1], cv.data[2], cv.data[3]);
                break;
            case Path::CommandType::BezierTo:
                if (cv.dataCount >= 6)
                    nvgBezierTo(nvg_, cv.data[0], cv.data[1], cv.data[2],
                               cv.data[3], cv.data[4], cv.data[5]);
                break;
            case Path::CommandType::ArcTo:
                if (cv.dataCount >= 5)
                    nvgArcTo(nvg_, cv.data[0], cv.data[1], cv.data[2], cv.data[3], cv.data[4]);
                break;
            case Path::CommandType::Arc:
                if (cv.dataCount >= 6) {
                    bool cw = cv.data[5] > 0.5f;
                    nvgArc(nvg_, cv.data[0], cv.data[1], cv.data[2],
                          cv.data[3], cv.data[4], cw ? NVG_CW : NVG_CCW);
                }
                break;
            case Path::CommandType::Rect:
                if (cv.dataCount >= 8) {
                    CornerRadius cr(cv.data[4], cv.data[5], cv.data[6], cv.data[7]);
                    if (cr.isZero()) nvgRect(nvg_, cv.data[0], cv.data[1], cv.data[2], cv.data[3]);
                    else if (cr.isUniform()) nvgRoundedRect(nvg_, cv.data[0], cv.data[1], cv.data[2], cv.data[3], cr.topLeft);
                    else nvgRoundedRectVarying(nvg_, cv.data[0], cv.data[1], cv.data[2], cv.data[3],
                             cr.topLeft, cr.topRight, cr.bottomRight, cr.bottomLeft);
                }
                break;
            case Path::CommandType::Circle:
                if (cv.dataCount >= 3) nvgCircle(nvg_, cv.data[0], cv.data[1], cv.data[2]);
                break;
            case Path::CommandType::Ellipse:
                if (cv.dataCount >= 4) nvgEllipse(nvg_, cv.data[0], cv.data[1], cv.data[2], cv.data[3]);
                break;
            case Path::CommandType::Close:
                nvgClosePath(nvg_);
                break;
        }
    }
    applyFill();
    applyStroke();
}

void NanoVGBackend::drawImage(int imageId, const Rect& rect, ImageFit fit,
                               const CornerRadius& cr, float alpha) {
    int w, h;
    nvgImageSize(nvg_, imageId, &w, &h);
    if (w <= 0 || h <= 0) return;

    Rect imgRect = rect;
    float iw = static_cast<float>(w), ih = static_cast<float>(h);
    switch (fit) {
        case ImageFit::Fill: break;
        case ImageFit::Cover: {
            float s = std::max(rect.width / iw, rect.height / ih);
            float sw = iw * s, sh = ih * s;
            imgRect = {rect.x + (rect.width - sw) / 2, rect.y + (rect.height - sh) / 2, sw, sh};
            break;
        }
        case ImageFit::Contain: {
            float s = std::min(rect.width / iw, rect.height / ih);
            float sw = iw * s, sh = ih * s;
            imgRect = {rect.x + (rect.width - sw) / 2, rect.y + (rect.height - sh) / 2, sw, sh};
            break;
        }
        case ImageFit::None: {
            imgRect = {rect.x + (rect.width - iw) / 2, rect.y + (rect.height - ih) / 2, iw, ih};
            break;
        }
    }

    NVGpaint paint = nvgImagePattern(nvg_, imgRect.x, imgRect.y, imgRect.width, imgRect.height,
                                     0, imageId, alpha);
    nvgBeginPath(nvg_);
    if (cr.isZero()) {
        nvgRect(nvg_, rect.x, rect.y, rect.width, rect.height);
    } else if (cr.isUniform()) {
        nvgRoundedRect(nvg_, rect.x, rect.y, rect.width, rect.height, cr.topLeft);
    } else {
        nvgRoundedRectVarying(nvg_, rect.x, rect.y, rect.width, rect.height,
            cr.topLeft, cr.topRight, cr.bottomRight, cr.bottomLeft);
    }
    nvgFillPaint(nvg_, paint);
    nvgFill(nvg_);
}

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
    auto discovered = FontProvider::findFontPath(family, weight);
    int font = -1;
    if (discovered) {
        font = nvgCreateFont(nvg_, key.c_str(), discovered->c_str());
    }
    if (font == -1) font = 0;
    fontCache_[key] = font;
    return font;
}

} // namespace flux
