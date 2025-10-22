#include <Flux/Graphics/NanoVGRenderContext.hpp>
#include <iostream>
#include <cmath>
#include <sstream>

namespace flux {

NanoVGRenderContext::NanoVGRenderContext(NVGcontext* nvgContext, int width, int height, float dpiScaleX, float dpiScaleY)
    : nvgContext_(nvgContext), width_(width), height_(height), dpiScaleX_(dpiScaleX), dpiScaleY_(dpiScaleY),
      currentFillStyle_(FillStyle::none()), currentStrokeStyle_(StrokeStyle::none()) {

    if (!nvgContext_) {
        throw std::runtime_error("NanoVG context is null");
    }

    // Calculate framebuffer dimensions
    framebufferWidth_ = static_cast<int>(width_ * dpiScaleX_);
    framebufferHeight_ = static_cast<int>(height_ * dpiScaleY_);

    std::cout << "[NanoVGRenderContext] Created with size " << width_ << "x" << height_
              << " DPI scale " << dpiScaleX_ << "x" << dpiScaleY_
              << " Framebuffer " << framebufferWidth_ << "x" << framebufferHeight_ << "\n";
}

// ============================================================================
// FRAME MANAGEMENT
// ============================================================================

void NanoVGRenderContext::beginFrame() {
    nvgBeginFrame(nvgContext_, width_, height_, dpiScaleX_);
}

void NanoVGRenderContext::clear(const Color& color) {
    nvgBeginPath(nvgContext_);
    nvgRect(nvgContext_, 0, 0, width_, height_);
    nvgFillColor(nvgContext_, toNVGColor(color));
    nvgFill(nvgContext_);
}

void NanoVGRenderContext::present() {
    nvgEndFrame(nvgContext_);
}

void NanoVGRenderContext::resize(int width, int height) {
    width_ = width;
    height_ = height;

    // Recalculate framebuffer dimensions
    framebufferWidth_ = static_cast<int>(width_ * dpiScaleX_);
    framebufferHeight_ = static_cast<int>(height_ * dpiScaleY_);

    std::cout << "[NanoVGRenderContext] Resized to " << width_ << "x" << height_
              << " Framebuffer " << framebufferWidth_ << "x" << framebufferHeight_ << "\n";
}

void NanoVGRenderContext::updateDPIScale(float dpiScaleX, float dpiScaleY) {
    dpiScaleX_ = dpiScaleX;
    dpiScaleY_ = dpiScaleY;

    // Recalculate framebuffer dimensions
    framebufferWidth_ = static_cast<int>(width_ * dpiScaleX_);
    framebufferHeight_ = static_cast<int>(height_ * dpiScaleY_);

    std::cout << "[NanoVGRenderContext] Updated DPI scale to " << dpiScaleX_ << "x" << dpiScaleY_
              << " Framebuffer " << framebufferWidth_ << "x" << framebufferHeight_ << "\n";
}

// ============================================================================
// STATE MANAGEMENT
// ============================================================================

void NanoVGRenderContext::save() {
    nvgSave(nvgContext_);
}

void NanoVGRenderContext::restore() {
    nvgRestore(nvgContext_);
}

void NanoVGRenderContext::reset() {
    nvgReset(nvgContext_);
}

// ============================================================================
// TRANSFORMATIONS
// ============================================================================

void NanoVGRenderContext::translate(float x, float y) {
    nvgTranslate(nvgContext_, x, y);
}

void NanoVGRenderContext::rotate(float angle) {
    nvgRotate(nvgContext_, angle);
}

void NanoVGRenderContext::scale(float sx, float sy) {
    nvgScale(nvgContext_, sx, sy);
}

void NanoVGRenderContext::skewX(float angle) {
    nvgSkewX(nvgContext_, angle);
}

void NanoVGRenderContext::skewY(float angle) {
    nvgSkewY(nvgContext_, angle);
}

void NanoVGRenderContext::setTransform(float a, float b, float c, float d, float e, float f) {
    nvgTransform(nvgContext_, a, b, c, d, e, f);
}

void NanoVGRenderContext::resetTransform() {
    nvgResetTransform(nvgContext_);
}

void NanoVGRenderContext::getTransform(float* matrix) {
    nvgCurrentTransform(nvgContext_, matrix);
}

// ============================================================================
// COMPOSITE OPERATIONS
// ============================================================================

void NanoVGRenderContext::setCompositeOperation(CompositeOperation op) {
    nvgGlobalCompositeOperation(nvgContext_, getNVGCompositeOperation(op));
}

void NanoVGRenderContext::setOpacity(float alpha) {
    nvgGlobalAlpha(nvgContext_, alpha);
}

void NanoVGRenderContext::setShapeAntiAlias(bool enabled) {
    nvgShapeAntiAlias(nvgContext_, enabled ? 1 : 0);
}

// ============================================================================
// STROKE STYLING
// ============================================================================

void NanoVGRenderContext::setStrokeColor(const Color& color) {
    nvgStrokeColor(nvgContext_, toNVGColor(color));
}

void NanoVGRenderContext::setStrokeWidth(float width) {
    nvgStrokeWidth(nvgContext_, width);
}

void NanoVGRenderContext::setLineCap(LineCap cap) {
    nvgLineCap(nvgContext_, getNVGLineCap(cap));
}

void NanoVGRenderContext::setLineJoin(LineJoin join) {
    nvgLineJoin(nvgContext_, getNVGLineJoin(join));
}

void NanoVGRenderContext::setMiterLimit(float limit) {
    nvgMiterLimit(nvgContext_, limit);
}

void NanoVGRenderContext::setDashPattern(const std::vector<float>& pattern, float offset) {
    if (pattern.empty()) {
        // Reset dash pattern to solid line - NanoVG doesn't have a way to reset dash patterns
        // This is a limitation of the current NanoVG implementation
        std::cout << "[NanoVGRenderContext] Dash pattern reset not supported by NanoVG\n";
    } else {
        // Note: NanoVG doesn't have direct dash pattern support in the public API
        // This would need to be implemented using the internal API or custom shaders
        // For now, we'll store the pattern and use it in stroke operations
        std::cout << "[NanoVGRenderContext] Dash patterns not fully implemented yet\n";
    }
    (void)offset; // Suppress unused parameter warning
}

void NanoVGRenderContext::setStrokeStyle(const StrokeStyle& style) {
    currentStrokeStyle_ = style;
    setStrokeColor(style.color);
    setStrokeWidth(style.width);
    setLineCap(style.cap);
    setLineJoin(style.join);
    setMiterLimit(style.miterLimit);
    if (style.type == StrokeStyle::Type::Dashed) {
        setDashPattern(style.dashPattern, style.dashOffset);
    }
}

// ============================================================================
// FILL STYLING
// ============================================================================

void NanoVGRenderContext::setFillColor(const Color& color) {
    currentFillStyle_.color = color;
    nvgFillPaint(nvgContext_, toNVGFillStyle(currentFillStyle_));
}

void NanoVGRenderContext::setPathWinding(PathWinding winding) {
    currentFillStyle_.winding = winding;
    nvgPathWinding(nvgContext_, getNVGPathWinding(winding));
}

void NanoVGRenderContext::setFillStyle(const FillStyle& style) {
    currentFillStyle_ = style;
    nvgFillPaint(nvgContext_, toNVGFillStyle(style));
    nvgPathWinding(nvgContext_, getNVGPathWinding(style.winding));
}

// ============================================================================
// PATH BUILDING
// ============================================================================

// ============================================================================
// PATH RENDERING
// ============================================================================

void NanoVGRenderContext::drawPath(const Path& path) {
    if (path.isEmpty()) {
        return;
    }
    
    nvgBeginPath(nvgContext_);
    
    // Replay path commands to NanoVG
    for (const auto& cmd : path.commands_) {
        switch (cmd.type) {
            case Path::CommandType::MoveTo:
                if (cmd.data.size() >= 2) {
                    nvgMoveTo(nvgContext_, cmd.data[0], cmd.data[1]);
                }
                break;
                
            case Path::CommandType::LineTo:
                if (cmd.data.size() >= 2) {
                    nvgLineTo(nvgContext_, cmd.data[0], cmd.data[1]);
                }
                break;
                
            case Path::CommandType::QuadTo:
                if (cmd.data.size() >= 4) {
                    nvgQuadTo(nvgContext_, cmd.data[0], cmd.data[1], cmd.data[2], cmd.data[3]);
                }
                break;
                
            case Path::CommandType::BezierTo:
                if (cmd.data.size() >= 6) {
                    nvgBezierTo(nvgContext_, cmd.data[0], cmd.data[1], cmd.data[2], 
                               cmd.data[3], cmd.data[4], cmd.data[5]);
                }
                break;
                
            case Path::CommandType::ArcTo:
                if (cmd.data.size() >= 5) {
                    nvgArcTo(nvgContext_, cmd.data[0], cmd.data[1], cmd.data[2], cmd.data[3], cmd.data[4]);
                }
                break;
                
            case Path::CommandType::Arc:
                if (cmd.data.size() >= 6) {
                    bool clockwise = cmd.data[5] > 0.5f;
                    nvgArc(nvgContext_, cmd.data[0], cmd.data[1], cmd.data[2], 
                          cmd.data[3], cmd.data[4], clockwise ? NVG_CW : NVG_CCW);
                }
                break;
                
            case Path::CommandType::Rect:
                if (cmd.data.size() >= 8) {
                    float x = cmd.data[0];
                    float y = cmd.data[1];
                    float w = cmd.data[2];
                    float h = cmd.data[3];
                    CornerRadius cr(cmd.data[4], cmd.data[5], cmd.data[6], cmd.data[7]);
                    
                    if (cr.isZero()) {
                        nvgRect(nvgContext_, x, y, w, h);
                    } else if (cr.isUniform()) {
                        nvgRoundedRect(nvgContext_, x, y, w, h, cr.topLeft);
                    } else {
                        nvgRoundedRectVarying(nvgContext_, x, y, w, h,
                                            cr.topLeft, cr.topRight, cr.bottomRight, cr.bottomLeft);
                    }
                }
                break;
                
            case Path::CommandType::Circle:
                if (cmd.data.size() >= 3) {
                    nvgCircle(nvgContext_, cmd.data[0], cmd.data[1], cmd.data[2]);
                }
                break;
                
            case Path::CommandType::Ellipse:
                if (cmd.data.size() >= 4) {
                    nvgEllipse(nvgContext_, cmd.data[0], cmd.data[1], cmd.data[2], cmd.data[3]);
                }
                break;
                
            case Path::CommandType::Close:
                nvgClosePath(nvgContext_);
                break;
        }
    }
    
    // Apply current fill and stroke styles based on their types
    if (currentFillStyle_.type != FillStyle::Type::None) {
        nvgFill(nvgContext_);
    }
    if (currentStrokeStyle_.type != StrokeStyle::Type::None) {
        nvgStroke(nvgContext_);
    }
}

// ============================================================================
// DIRECT SHAPE DRAWING
// ============================================================================

void NanoVGRenderContext::drawCircle(const Point& center, float radius) {
    nvgBeginPath(nvgContext_);
    nvgCircle(nvgContext_, center.x, center.y, radius);
    
    // Apply current fill and stroke styles based on their types
    if (currentFillStyle_.type != FillStyle::Type::None) {
        nvgFill(nvgContext_);
    }
    if (currentStrokeStyle_.type != StrokeStyle::Type::None) {
        nvgStroke(nvgContext_);
    }
}

void NanoVGRenderContext::drawLine(const Point& start, const Point& end) {
    nvgBeginPath(nvgContext_);
    nvgMoveTo(nvgContext_, start.x, start.y);
    nvgLineTo(nvgContext_, end.x, end.y);
    
    // Lines only use stroke style (no fill applied)
    if (currentStrokeStyle_.type != StrokeStyle::Type::None) {
        nvgStroke(nvgContext_);
    }
}

void NanoVGRenderContext::drawRect(const Rect& rect, const CornerRadius& cornerRadius) {
    nvgBeginPath(nvgContext_);

    if (cornerRadius.isZero()) {
        nvgRect(nvgContext_, rect.x, rect.y, rect.width, rect.height);
    } else if (cornerRadius.isUniform()) {
        nvgRoundedRect(nvgContext_, rect.x, rect.y, rect.width, rect.height, cornerRadius.topLeft);
    } else {
        nvgRoundedRectVarying(nvgContext_, rect.x, rect.y, rect.width, rect.height,
                              cornerRadius.topLeft, cornerRadius.topRight, 
                              cornerRadius.bottomRight, cornerRadius.bottomLeft);
    }
    
    // Apply current fill and stroke styles based on their types
    if (currentFillStyle_.type != FillStyle::Type::None) {
        nvgFill(nvgContext_);
    }
    if (currentStrokeStyle_.type != StrokeStyle::Type::None) {
        nvgStroke(nvgContext_);
    }
}

void NanoVGRenderContext::drawEllipse(const Point& center, float radiusX, float radiusY) {
    nvgBeginPath(nvgContext_);
    nvgEllipse(nvgContext_, center.x, center.y, radiusX, radiusY);
    
    // Apply current fill and stroke styles based on their types
    if (currentFillStyle_.type != FillStyle::Type::None) {
        nvgFill(nvgContext_);
    }
    if (currentStrokeStyle_.type != StrokeStyle::Type::None) {
        nvgStroke(nvgContext_);
    }
}

void NanoVGRenderContext::drawArc(const Point& center, float radius, float startAngle, float endAngle, bool clockwise) {
    nvgBeginPath(nvgContext_);
    nvgArc(nvgContext_, center.x, center.y, radius, startAngle, endAngle, clockwise ? NVG_CW : NVG_CCW);
    
    // Apply current fill and stroke styles based on their types
    if (currentFillStyle_.type != FillStyle::Type::None) {
        nvgFill(nvgContext_);
    }
    if (currentStrokeStyle_.type != StrokeStyle::Type::None) {
        nvgStroke(nvgContext_);
    }
}

// ============================================================================
// TEXT RENDERING
// ============================================================================

void NanoVGRenderContext::setFont(const std::string& name, FontWeight weight) {
    int font = getFont(name, weight);
    nvgFontFaceId(nvgContext_, font);
}

void NanoVGRenderContext::setFontSize(float size) {
    nvgFontSize(nvgContext_, size);
}

void NanoVGRenderContext::setFontBlur(float blur) {
    nvgFontBlur(nvgContext_, blur);
}

void NanoVGRenderContext::setLetterSpacing(float spacing) {
    nvgTextLetterSpacing(nvgContext_, spacing);
}

void NanoVGRenderContext::setLineHeight(float height) {
    nvgTextLineHeight(nvgContext_, height);
}

void NanoVGRenderContext::setTextStyle(const TextStyle& style) {
    setFont(style.fontName, style.weight);
    setFontSize(style.size);
    setLetterSpacing(style.letterSpacing);
    setLineHeight(style.lineHeight);
}

void NanoVGRenderContext::drawText(const std::string& text, const Point& position, HorizontalAlignment hAlign, VerticalAlignment vAlign) {
    nvgTextAlign(nvgContext_, getNVGTextAlign(hAlign, vAlign));
    // // Apply current fill style for text rendering
    // if (currentFillStyle_.type != FillStyle::Type::None) {
    //     nvgFill(nvgContext_);
    // }
    nvgText(nvgContext_, position.x, position.y, text.c_str(), nullptr);
}

Size NanoVGRenderContext::measureText(const std::string& text, const TextStyle& style) {
    // Apply the text style temporarily for measurement
    nvgFontFaceId(nvgContext_, getFont(style.fontName, style.weight));
    nvgFontSize(nvgContext_, style.size);
    nvgTextLetterSpacing(nvgContext_, style.letterSpacing);
    nvgTextLineHeight(nvgContext_, style.lineHeight);
    
    float bounds[4];
    nvgTextBounds(nvgContext_, 0, 0, text.c_str(), nullptr, bounds);
    return Size{bounds[2] - bounds[0], bounds[3] - bounds[1]};
}

Rect NanoVGRenderContext::getTextBounds(const std::string& text, const Point& position, const TextStyle& style) {
    // Apply the text style temporarily for bounds calculation
    nvgFontFaceId(nvgContext_, getFont(style.fontName, style.weight));
    nvgFontSize(nvgContext_, style.size);
    nvgTextLetterSpacing(nvgContext_, style.letterSpacing);
    nvgTextLineHeight(nvgContext_, style.lineHeight);
    
    float bounds[4];
    nvgTextBounds(nvgContext_, position.x, position.y, text.c_str(), nullptr, bounds);
    return Rect{bounds[0], bounds[1], bounds[2] - bounds[0], bounds[3] - bounds[1]};
}

// ============================================================================
// IMAGE MANAGEMENT
// ============================================================================

int NanoVGRenderContext::createImage(const std::string& filename) {
    auto it = imageCache_.find(filename);
    if (it != imageCache_.end()) {
        return it->second;
    }

    int imageId = nvgCreateImage(nvgContext_, filename.c_str(), 0);
    if (imageId != -1) {
        imageCache_[filename] = imageId;
    }
    return imageId;
}

int NanoVGRenderContext::createImageMem(const unsigned char* data, int dataSize) {
    return nvgCreateImageMem(nvgContext_, 0, const_cast<unsigned char*>(data), dataSize);
}

int NanoVGRenderContext::createImageRGBA(int width, int height, const unsigned char* data) {
    return nvgCreateImageRGBA(nvgContext_, width, height, 0, data);
}

void NanoVGRenderContext::updateImage(int imageId, const unsigned char* data) {
    nvgUpdateImage(nvgContext_, imageId, data);
}

Size NanoVGRenderContext::getImageSize(int imageId) {
    int w, h;
    nvgImageSize(nvgContext_, imageId, &w, &h);
    return Size{static_cast<float>(w), static_cast<float>(h)};
}

void NanoVGRenderContext::deleteImage(int imageId) {
    nvgDeleteImage(nvgContext_, imageId);
}

void NanoVGRenderContext::drawImage(int imageId, const Rect& rect, ImageFit fit, 
                                const CornerRadius& cornerRadius, float alpha) {
    Size imageSize = getImageSize(imageId);
    if (imageSize.width <= 0 || imageSize.height <= 0) {
        return; // Invalid image
    }

    // Calculate the rect to use for the image pattern based on fit mode
    Rect imageRect = rect;
    
    switch (fit) {
        case ImageFit::Fill:
            // Stretch to fill - use rect as-is
            imageRect = rect;
            break;
            
        case ImageFit::Cover: {
            // Scale to cover entire area (may crop)
            float scaleX = rect.width / imageSize.width;
            float scaleY = rect.height / imageSize.height;
            float scale = std::max(scaleX, scaleY);
            
            float scaledWidth = imageSize.width * scale;
            float scaledHeight = imageSize.height * scale;
            
            float offsetX = (rect.width - scaledWidth) / 2;
            float offsetY = (rect.height - scaledHeight) / 2;
            
            imageRect = Rect{
                rect.x + offsetX,
                rect.y + offsetY,
                scaledWidth,
                scaledHeight
            };
            break;
        }
            
        case ImageFit::Contain: {
            // Scale to fit inside (may letterbox)
            float scaleX = rect.width / imageSize.width;
            float scaleY = rect.height / imageSize.height;
            float scale = std::min(scaleX, scaleY);
            
            float scaledWidth = imageSize.width * scale;
            float scaledHeight = imageSize.height * scale;
            
            float offsetX = (rect.width - scaledWidth) / 2;
            float offsetY = (rect.height - scaledHeight) / 2;
            
            imageRect = Rect{
                rect.x + offsetX,
                rect.y + offsetY,
                scaledWidth,
                scaledHeight
            };
            break;
        }
            
        case ImageFit::None:
            // Use original size, centered
            float offsetX = (rect.width - imageSize.width) / 2;
            float offsetY = (rect.height - imageSize.height) / 2;
            
            imageRect = Rect{
                rect.x + offsetX,
                rect.y + offsetY,
                imageSize.width,
                imageSize.height
            };
            break;
    }

    // Create fill style with image pattern
    FillStyle fillStyle;
    fillStyle.type = FillStyle::Type::ImagePattern;
    fillStyle.imageId = imageId;
    fillStyle.imageOrigin = imageRect.origin();
    fillStyle.imageSize = imageRect.size();
    fillStyle.imageAngle = 0.0f;
    fillStyle.imageAlpha = alpha;

    // Draw the image
    nvgBeginPath(nvgContext_);
    if (cornerRadius.isZero()) {
        nvgRect(nvgContext_, rect.x, rect.y, rect.width, rect.height);
    } else if (cornerRadius.isUniform()) {
        nvgRoundedRect(nvgContext_, rect.x, rect.y, rect.width, rect.height, cornerRadius.topLeft);
    } else {
        nvgRoundedRectVarying(nvgContext_, rect.x, rect.y, rect.width, rect.height,
                              cornerRadius.topLeft, cornerRadius.topRight,
                              cornerRadius.bottomRight, cornerRadius.bottomLeft);
    }
    nvgFillPaint(nvgContext_, toNVGFillStyle(fillStyle));
    nvgFill(nvgContext_);
}

void NanoVGRenderContext::drawImage(const std::string& path, const Rect& rect, ImageFit fit,
                                const CornerRadius& cornerRadius, float alpha) {
    int imageId = createImage(path);
    if (imageId != -1) {
        drawImage(imageId, rect, fit, cornerRadius, alpha);
    } else {
        // Draw placeholder
        nvgBeginPath(nvgContext_);
        if (cornerRadius.isZero()) {
            nvgRect(nvgContext_, rect.x, rect.y, rect.width, rect.height);
        } else if (cornerRadius.isUniform()) {
            nvgRoundedRect(nvgContext_, rect.x, rect.y, rect.width, rect.height, cornerRadius.topLeft);
        } else {
            nvgRoundedRectVarying(nvgContext_, rect.x, rect.y, rect.width, rect.height,
                                  cornerRadius.topLeft, cornerRadius.topRight,
                                  cornerRadius.bottomRight, cornerRadius.bottomLeft);
        }
        nvgFillColor(nvgContext_, nvgRGBA(200, 200, 200, 255));
        nvgFill(nvgContext_);

        nvgFontSize(nvgContext_, 12);
        nvgFillColor(nvgContext_, nvgRGBA(100, 100, 100, 255));
        nvgTextAlign(nvgContext_, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgText(nvgContext_, rect.x + rect.width/2, rect.y + rect.height/2, path.c_str(), nullptr);
    }
}

// ============================================================================
// CLIPPING
// ============================================================================

void NanoVGRenderContext::clipPath(const Path& path) {
    // NanoVG doesn't support arbitrary path clipping, only rectangular scissors
    // Use the bounding box of the path
    Rect bounds = path.getBounds();
    nvgIntersectScissor(nvgContext_, bounds.x, bounds.y, bounds.width, bounds.height);
}

void NanoVGRenderContext::resetClip() {
    nvgResetScissor(nvgContext_);
}

// ============================================================================
// UTILITIES
// ============================================================================

Point NanoVGRenderContext::transformPoint(const Point& point) {
    float matrix[6];
    nvgCurrentTransform(nvgContext_, matrix);

    float x = matrix[0] * point.x + matrix[2] * point.y + matrix[4];
    float y = matrix[1] * point.x + matrix[3] * point.y + matrix[5];

    return Point(x, y);
}

Rect NanoVGRenderContext::transformRect(const Rect& rect) {
    Point topLeft = transformPoint(rect.origin());
    Point topRight = transformPoint(Point(rect.x + rect.width, rect.y));
    Point bottomLeft = transformPoint(Point(rect.x, rect.y + rect.height));
    Point bottomRight = transformPoint(Point(rect.x + rect.width, rect.y + rect.height));

    float minX = std::min({topLeft.x, topRight.x, bottomLeft.x, bottomRight.x});
    float minY = std::min({topLeft.y, topRight.y, bottomLeft.y, bottomRight.y});
    float maxX = std::max({topLeft.x, topRight.x, bottomLeft.x, bottomRight.x});
    float maxY = std::max({topLeft.y, topRight.y, bottomLeft.y, bottomRight.y});

    return Rect(minX, minY, maxX - minX, maxY - minY);
}

float NanoVGRenderContext::degToRad(float degrees) {
    return nvgDegToRad(degrees);
}

float NanoVGRenderContext::radToDeg(float radians) {
    return nvgRadToDeg(radians);
}

// ============================================================================
// HELPER FUNCTIONS
// ============================================================================

NVGcolor NanoVGRenderContext::toNVGColor(const Color& color) const {
    return nvgRGBAf(color.r, color.g, color.b, color.a);
}

NVGpaint NanoVGRenderContext::toNVGFillStyle(const FillStyle& fillStyle) const {
    switch (fillStyle.type) {
        case FillStyle::Type::None:
            return nvgLinearGradient(nvgContext_, 0, 0, 0, 0, 
                                   toNVGColor(Colors::transparent), toNVGColor(Colors::transparent));

        case FillStyle::Type::Solid:
            return nvgLinearGradient(nvgContext_, 0, 0, 0, 0, 
                                   toNVGColor(fillStyle.color), toNVGColor(fillStyle.color));

        case FillStyle::Type::LinearGradient:
            return nvgLinearGradient(nvgContext_, fillStyle.startPoint.x, fillStyle.startPoint.y,
                                     fillStyle.endPoint.x, fillStyle.endPoint.y,
                                     toNVGColor(fillStyle.startColor), toNVGColor(fillStyle.endColor));

        case FillStyle::Type::RadialGradient:
            return nvgRadialGradient(nvgContext_, fillStyle.center.x, fillStyle.center.y,
                                     fillStyle.innerRadius, fillStyle.outerRadius,
                                     toNVGColor(fillStyle.startColor), toNVGColor(fillStyle.endColor));

        case FillStyle::Type::BoxGradient:
            return nvgBoxGradient(nvgContext_, fillStyle.bounds.x, fillStyle.bounds.y,
                                  fillStyle.bounds.width, fillStyle.bounds.height,
                                  fillStyle.cornerRadius, fillStyle.feather,
                                  toNVGColor(fillStyle.startColor), toNVGColor(fillStyle.endColor));

        case FillStyle::Type::ImagePattern:
            return nvgImagePattern(nvgContext_, fillStyle.imageOrigin.x, fillStyle.imageOrigin.y,
                                   fillStyle.imageSize.width, fillStyle.imageSize.height,
                                   fillStyle.imageAngle, fillStyle.imageId, fillStyle.imageAlpha);

        default:
            return nvgLinearGradient(nvgContext_, 0, 0, 0, 0, toNVGColor(fillStyle.color), toNVGColor(fillStyle.color));
    }
}

int NanoVGRenderContext::getFont(const std::string& fontName, FontWeight weight) {
    std::string fontKey = fontName + "_" + std::to_string(static_cast<int>(weight));

    auto it = fontCache_.find(fontKey);
    if (it != fontCache_.end()) {
        return it->second;
    }

    // Try to create a font with a fallback approach
    int font = -1;

    // Try common system fonts on different platforms
    const char* fontPaths[] = {
        // Linux fonts
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/liberation/LiberationSans-Regular.ttf",
        // macOS fonts
        "/System/Library/Fonts/Arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/SF-Pro-Display-Regular.otf",
        "/System/Library/Fonts/HelveticaNeue.ttc",
        // Windows fonts (if running on Wine or WSL)
        "C:\\Windows\\Fonts\\arial.ttf",
        nullptr
    };

    for (int i = 0; fontPaths[i] != nullptr; i++) {
        font = nvgCreateFont(nvgContext_, fontKey.c_str(), fontPaths[i]);
        if (font != -1) {
            std::cout << "[NanoVGRenderContext] Loaded font: " << fontPaths[i] << std::endl;
            break;
        }
    }

    // If all font files fail, try using the default font
    if (font == -1) {
        font = nvgCreateFont(nvgContext_, fontKey.c_str(), nullptr);
        if (font == -1) {
            std::cerr << "[NanoVGRenderContext] Failed to load any font!" << std::endl;
            font = 0;
        } else {
            std::cout << "[NanoVGRenderContext] Using default font" << std::endl;
        }
    }

    fontCache_[fontKey] = font;
    return font;
}

int NanoVGRenderContext::getNVGLineCap(LineCap cap) const {
    switch (cap) {
        case LineCap::Butt: return NVG_BUTT;
        case LineCap::Round: return NVG_ROUND;
        case LineCap::Square: return NVG_SQUARE;
        default: return NVG_BUTT;
    }
}

int NanoVGRenderContext::getNVGLineJoin(LineJoin join) const {
    switch (join) {
        case LineJoin::Miter: return NVG_MITER;
        case LineJoin::Round: return NVG_ROUND;
        case LineJoin::Bevel: return NVG_BEVEL;
        default: return NVG_MITER;
    }
}

int NanoVGRenderContext::getNVGCompositeOperation(CompositeOperation op) const {
    switch (op) {
        case CompositeOperation::SourceOver: return NVG_SOURCE_OVER;
        case CompositeOperation::SourceIn: return NVG_SOURCE_IN;
        case CompositeOperation::SourceOut: return NVG_SOURCE_OUT;
        case CompositeOperation::Atop: return NVG_ATOP;
        case CompositeOperation::DestinationOver: return NVG_DESTINATION_OVER;
        case CompositeOperation::DestinationIn: return NVG_DESTINATION_IN;
        case CompositeOperation::DestinationOut: return NVG_DESTINATION_OUT;
        case CompositeOperation::DestinationAtop: return NVG_DESTINATION_ATOP;
        case CompositeOperation::Lighter: return NVG_LIGHTER;
        case CompositeOperation::Copy: return NVG_COPY;
        case CompositeOperation::Xor: return NVG_XOR;
        default: return NVG_SOURCE_OVER;
    }
}

int NanoVGRenderContext::getNVGPathWinding(PathWinding winding) const {
    switch (winding) {
        case PathWinding::CounterClockwise: return NVG_CCW;
        case PathWinding::Clockwise: return NVG_CW;
        default: return NVG_CCW;
    }
}

int NanoVGRenderContext::getNVGTextAlign(HorizontalAlignment hAlign, VerticalAlignment vAlign) const {
    int align = 0;

    // Horizontal alignment
    switch (hAlign) {
        case HorizontalAlignment::leading:
            align |= NVG_ALIGN_LEFT;
            break;
        case HorizontalAlignment::center:
            align |= NVG_ALIGN_CENTER;
            break;
        case HorizontalAlignment::trailing:
            align |= NVG_ALIGN_RIGHT;
            break;
        case HorizontalAlignment::justify:
            align |= NVG_ALIGN_LEFT; // Justify not directly supported, fallback to left
            break;
    }

    // Vertical alignment
    switch (vAlign) {
        case VerticalAlignment::top:
            align |= NVG_ALIGN_TOP;
            break;
        case VerticalAlignment::center:
            align |= NVG_ALIGN_MIDDLE;
            break;
        case VerticalAlignment::bottom:
            align |= NVG_ALIGN_BOTTOM;
            break;
    }

    return align;
}

} // namespace flux