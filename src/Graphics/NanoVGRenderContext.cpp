#include <Flux/Graphics/NanoVGRenderContext.hpp>
#include <iostream>
#include <cmath>
#include <sstream>
#include <nanosvg.h>

namespace flux {

NanoVGRenderContext::NanoVGRenderContext(NVGcontext* nvgContext, int width, int height, float dpiScaleX, float dpiScaleY)
    : nvgContext_(nvgContext), width_(width), height_(height), dpiScaleX_(dpiScaleX), dpiScaleY_(dpiScaleY) {

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

void NanoVGRenderContext::drawRect(const Rect& rect, const Color& color) {
    nvgBeginPath(nvgContext_);
    nvgRect(nvgContext_, rect.x, rect.y, rect.width, rect.height);
    nvgFillColor(nvgContext_, toNVGColor(color));
    nvgFill(nvgContext_);
}

void NanoVGRenderContext::drawRoundedRect(const Rect& rect, float cornerRadius, const Color& color) {
    nvgBeginPath(nvgContext_);
    nvgRoundedRect(nvgContext_, rect.x, rect.y, rect.width, rect.height, cornerRadius);
    nvgFillColor(nvgContext_, toNVGColor(color));
    nvgFill(nvgContext_);
}

void NanoVGRenderContext::drawRoundedRectBorder(const Rect& rect, float cornerRadius, const Color& color, float width) {
    nvgBeginPath(nvgContext_);
    nvgRoundedRect(nvgContext_, rect.x, rect.y, rect.width, rect.height, cornerRadius);
    nvgStrokeColor(nvgContext_, toNVGColor(color));
    nvgStrokeWidth(nvgContext_, width);
    nvgStroke(nvgContext_);
}

void NanoVGRenderContext::drawCircle(const Point& center, float radius, const Color& color) {
    nvgBeginPath(nvgContext_);
    nvgCircle(nvgContext_, center.x, center.y, radius);
    nvgFillColor(nvgContext_, toNVGColor(color));
    nvgFill(nvgContext_);
}

void NanoVGRenderContext::drawArc(const Point& center, float radius, float startAngle, float endAngle, const Color& color, float strokeWidth) {
    nvgBeginPath(nvgContext_);
    nvgArc(nvgContext_, center.x, center.y, radius, startAngle, endAngle, NVG_CW);
    nvgStrokeColor(nvgContext_, toNVGColor(color));
    nvgStrokeWidth(nvgContext_, strokeWidth);
    nvgStroke(nvgContext_);
}

void NanoVGRenderContext::drawLine(const Point& start, const Point& end, float width, const Color& color) {
    nvgBeginPath(nvgContext_);
    nvgMoveTo(nvgContext_, start.x, start.y);
    nvgLineTo(nvgContext_, end.x, end.y);
    nvgStrokeColor(nvgContext_, toNVGColor(color));
    nvgStrokeWidth(nvgContext_, width);
    nvgStroke(nvgContext_);
}

void NanoVGRenderContext::drawBezier(const Point& c1, const Point& c2, const Point& end, const Color& fillColor, const Color& strokeColor, float strokeWidth) {
    nvgBeginPath(nvgContext_);

    // Move to origin (0,0) as starting point
    nvgMoveTo(nvgContext_, 0, 0);

    // Draw cubic bezier curve
    nvgBezierTo(nvgContext_, c1.x, c1.y, c2.x, c2.y, end.x, end.y);

    // Fill the path if fill color is specified
    if (fillColor.a > 0.0f) {
        nvgFillColor(nvgContext_, toNVGColor(fillColor));
        nvgFill(nvgContext_);
    }

    // Stroke the path if stroke color is specified
    if (strokeColor.a > 0.0f && strokeWidth > 0.0f) {
        nvgStrokeColor(nvgContext_, toNVGColor(strokeColor));
        nvgStrokeWidth(nvgContext_, strokeWidth);
        nvgStroke(nvgContext_);
    }
}

void NanoVGRenderContext::drawQuad(const Point& control, const Point& end, const Color& fillColor, const Color& strokeColor, float strokeWidth) {
    nvgBeginPath(nvgContext_);

    // Move to origin (0,0) as starting point
    nvgMoveTo(nvgContext_, 0, 0);

    // Draw quadratic bezier curve
    nvgQuadTo(nvgContext_, control.x, control.y, end.x, end.y);

    // Fill the path if fill color is specified
    if (fillColor.a > 0.0f) {
        nvgFillColor(nvgContext_, toNVGColor(fillColor));
        nvgFill(nvgContext_);
    }

    // Stroke the path if stroke color is specified
    if (strokeColor.a > 0.0f && strokeWidth > 0.0f) {
        nvgStrokeColor(nvgContext_, toNVGColor(strokeColor));
        nvgStrokeWidth(nvgContext_, strokeWidth);
        nvgStroke(nvgContext_);
    }
}

// Path building methods for complex shapes
void NanoVGRenderContext::beginPath() {
    nvgBeginPath(nvgContext_);
}

void NanoVGRenderContext::moveTo(const Point& point) {
    nvgMoveTo(nvgContext_, point.x, point.y);
}

void NanoVGRenderContext::lineTo(const Point& point) {
    nvgLineTo(nvgContext_, point.x, point.y);
}

void NanoVGRenderContext::arcTo(const Point& center, float radius, float startAngle, float endAngle) {
    nvgArcTo(nvgContext_, center.x, center.y, radius, startAngle, endAngle);
}

void NanoVGRenderContext::quadTo(const Point& control, const Point& end) {
    nvgQuadTo(nvgContext_, control.x, control.y, end.x, end.y);
}

void NanoVGRenderContext::bezierTo(const Point& c1, const Point& c2, const Point& end) {
    std::cout << "[NanoVGRenderContext] Drawing bezier to " << c1.x << ", " << c1.y << " " << c2.x << ", " << c2.y << " " << end.x << ", " << end.y << std::endl;
    nvgBezierTo(nvgContext_, c1.x, c1.y, c2.x, c2.y, end.x, end.y);
}

void NanoVGRenderContext::closePath() {
    nvgClosePath(nvgContext_);
}

void NanoVGRenderContext::fillPath(const Color& fillColor) {
    if (fillColor.a > 0.0f) {
        nvgFillColor(nvgContext_, toNVGColor(fillColor));
        nvgFill(nvgContext_);
    }
}

void NanoVGRenderContext::strokePath(const Color& strokeColor, float strokeWidth) {
    if (strokeColor.a > 0.0f && strokeWidth > 0.0f) {
        nvgStrokeColor(nvgContext_, toNVGColor(strokeColor));
        nvgStrokeWidth(nvgContext_, strokeWidth);
        nvgStroke(nvgContext_);
    }
}

void NanoVGRenderContext::renderNanoVGPath(NSVGpath* path, const Color& fillColor, const Color& strokeColor, float strokeWidth) {
    if (!path || path->npts < 2) return;

    nvgBeginPath(nvgContext_);

    // Convert NanoSVG path to NanoVG path
    float* pts = path->pts;
    int npts = path->npts;

    // Move to first point
    nvgMoveTo(nvgContext_, pts[0], pts[1]);

    // Draw cubic bezier curves
    for (int i = 1; i < npts - 1; i += 3) {
        if (i + 5 < npts * 2) {
            nvgBezierTo(nvgContext_, pts[i*2], pts[i*2+1], pts[(i+1)*2], pts[(i+1)*2+1], pts[(i+2)*2], pts[(i+2)*2+1]);
        }
    }

    // Close path if it's closed
    if (path->closed) {
        nvgClosePath(nvgContext_);
    }

    // Fill the path if fill color is specified
    if (fillColor.a > 0.0f) {
        nvgFillColor(nvgContext_, toNVGColor(fillColor));
        nvgFill(nvgContext_);
    }

    // Stroke the path if stroke color is specified
    if (strokeColor.a > 0.0f && strokeWidth > 0.0f) {
        nvgStrokeColor(nvgContext_, toNVGColor(strokeColor));
        nvgStrokeWidth(nvgContext_, strokeWidth);
        nvgStroke(nvgContext_);
    }
}

void NanoVGRenderContext::drawShadow(const Rect& rect, float cornerRadius, const Shadow& shadow) {
    // Create shadow paint
    NVGpaint shadowPaint = createShadowPaint(shadow);

    nvgBeginPath(nvgContext_);
    nvgRoundedRect(nvgContext_, rect.x + shadow.offsetX, rect.y + shadow.offsetY,
                   rect.width, rect.height, cornerRadius);
    nvgFillPaint(nvgContext_, shadowPaint);
    nvgFill(nvgContext_);
}

void NanoVGRenderContext::drawText(const std::string& text, const Point& position, float fontSize, const Color& color,
                                  FontWeight weight, HorizontalAlignment align) {
    int font = getFont("default", weight);
    nvgFontSize(nvgContext_, fontSize);
    nvgFontFaceId(nvgContext_, font);
    nvgFillColor(nvgContext_, toNVGColor(color));

    // Set horizontal alignment using NanoVG's alignment system
    int nvgAlign = NVG_ALIGN_BASELINE; // Default vertical alignment
    switch (align) {
        case HorizontalAlignment::leading:
            nvgAlign |= NVG_ALIGN_LEFT;
            break;
        case HorizontalAlignment::center:
            nvgAlign |= NVG_ALIGN_CENTER;
            break;
        case HorizontalAlignment::trailing:
            nvgAlign |= NVG_ALIGN_RIGHT;
            break;
        case HorizontalAlignment::justify:
            nvgAlign |= NVG_ALIGN_LEFT; // Justify not directly supported, fallback to left
            break;
    }

    nvgTextAlign(nvgContext_, nvgAlign);
    nvgText(nvgContext_, position.x, position.y, text.c_str(), nullptr);
}

void NanoVGRenderContext::drawText(const std::string& text, const Point& position, float fontSize, const Color& color,
                                  FontWeight weight, HorizontalAlignment hAlign, VerticalAlignment vAlign) {
    int font = getFont("default", weight);
    nvgFontSize(nvgContext_, fontSize);
    nvgFontFaceId(nvgContext_, font);
    nvgFillColor(nvgContext_, toNVGColor(color));

    // Set alignment using NanoVG's alignment system
    int nvgAlign = 0;

    // Horizontal alignment
    switch (hAlign) {
        case HorizontalAlignment::leading:
            nvgAlign |= NVG_ALIGN_LEFT;
            break;
        case HorizontalAlignment::center:
            nvgAlign |= NVG_ALIGN_CENTER;
            break;
        case HorizontalAlignment::trailing:
            nvgAlign |= NVG_ALIGN_RIGHT;
            break;
        case HorizontalAlignment::justify:
            nvgAlign |= NVG_ALIGN_LEFT; // Justify not directly supported, fallback to left
            break;
    }

    // Vertical alignment - use baseline for better control
    float adjustedY = position.y;
    switch (vAlign) {
        case VerticalAlignment::top:
            nvgAlign |= NVG_ALIGN_TOP;
            break;
        case VerticalAlignment::center:
            // For center alignment, use NVG_ALIGN_MIDDLE but adjust for better visual centering
            // The issue is that fonts have different amounts of space above and below characters
            // We'll use NVG_ALIGN_MIDDLE but with a small adjustment
            nvgAlign |= NVG_ALIGN_MIDDLE;
            // Small adjustment to account for font metrics - move text slightly up
            adjustedY = position.y + fontSize * 0.08f;
            break;
        case VerticalAlignment::bottom:
            nvgAlign |= NVG_ALIGN_BOTTOM;
            break;
    }

    nvgTextAlign(nvgContext_, nvgAlign);
    nvgText(nvgContext_, position.x, adjustedY, text.c_str(), nullptr);
}

Size NanoVGRenderContext::measureText(const std::string& text, float fontSize, FontWeight weight) {
    int font = getFont("default", weight);
    nvgFontSize(nvgContext_, fontSize);
    nvgFontFaceId(nvgContext_, font);

    float bounds[4];
    nvgTextBounds(nvgContext_, 0, 0, text.c_str(), nullptr, bounds);

    return Size{bounds[2] - bounds[0], bounds[3] - bounds[1]};
}

void NanoVGRenderContext::drawImage(const std::string& path, const Rect& rect, float cornerRadius) {
    // For now, draw a placeholder rectangle
    // In a real implementation, you would load and cache the image
    nvgBeginPath(nvgContext_);
    if (cornerRadius > 0) {
        nvgRoundedRect(nvgContext_, rect.x, rect.y, rect.width, rect.height, cornerRadius);
    } else {
        nvgRect(nvgContext_, rect.x, rect.y, rect.width, rect.height);
    }
    nvgFillColor(nvgContext_, nvgRGBA(200, 200, 200, 255)); // Gray placeholder
    nvgFill(nvgContext_);

    // Draw image path as text
    nvgFontSize(nvgContext_, 12);
    nvgFillColor(nvgContext_, nvgRGBA(100, 100, 100, 255));
    nvgTextAlign(nvgContext_, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
    nvgText(nvgContext_, rect.x + rect.width/2, rect.y + rect.height/2, path.c_str(), nullptr);
}

void NanoVGRenderContext::save() {
    nvgSave(nvgContext_);
}

void NanoVGRenderContext::restore() {
    nvgRestore(nvgContext_);
}

void NanoVGRenderContext::translate(float x, float y) {
    nvgTranslate(nvgContext_, x, y);
}

void NanoVGRenderContext::rotate(float angle) {
    nvgRotate(nvgContext_, angle);
}

void NanoVGRenderContext::scale(float sx, float sy) {
    nvgScale(nvgContext_, sx, sy);
}

void NanoVGRenderContext::setOpacity(float opacity) {
    nvgGlobalAlpha(nvgContext_, opacity);
}

void NanoVGRenderContext::clipRect(const Rect& rect) {
    nvgScissor(nvgContext_, rect.x, rect.y, rect.width, rect.height);
}

void NanoVGRenderContext::clipRoundedRect(const Rect& rect, float cornerRadius) {
    nvgBeginPath(nvgContext_);
    nvgRoundedRect(nvgContext_, rect.x, rect.y, rect.width, rect.height, cornerRadius);
    nvgScissor(nvgContext_, rect.x, rect.y, rect.width, rect.height);
}

void NanoVGRenderContext::beginFrame() {
    // Begin the NanoVG frame with logical dimensions and device pixel ratio
    // NanoVG will handle the scaling internally
    nvgBeginFrame(nvgContext_, width_, height_, dpiScaleX_);
}

void NanoVGRenderContext::clear(const Color& color) {
    // Clear by drawing a filled rectangle covering the entire area
    nvgBeginPath(nvgContext_);
    nvgRect(nvgContext_, 0, 0, width_, height_);
    nvgFillColor(nvgContext_, toNVGColor(color));
    nvgFill(nvgContext_);
}

void NanoVGRenderContext::present() {
    // End the NanoVG frame
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

NVGcolor NanoVGRenderContext::toNVGColor(const Color& color) const {
    return nvgRGBAf(color.r, color.g, color.b, color.a);
}

NVGpaint NanoVGRenderContext::createShadowPaint(const Shadow& shadow) const {
    Color shadowColor = shadow.color;
    shadowColor.a *= shadow.opacity; // Apply shadow opacity

    NVGcolor nvgShadowColor = nvgRGBAf(shadowColor.r, shadowColor.g, shadowColor.b, shadowColor.a);
    return nvgBoxGradient(nvgContext_, 0, 0, 0, 0, shadow.blurRadius, shadow.blurRadius, nvgShadowColor, nvgRGBA(0, 0, 0, 0));
}

int NanoVGRenderContext::getFont(const std::string& fontName, FontWeight weight) {
    std::string fontKey = fontName + "_" + std::to_string(static_cast<int>(weight));

    auto it = fontCache_.find(fontKey);
    if (it != fontCache_.end()) {
        return it->second;
    }

    // Try to create a font with a fallback approach
    int font = -1;

    // Try common system fonts on macOS
    const char* fontPaths[] = {
        "/System/Library/Fonts/Arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/SF-Pro-Display-Regular.otf",
        "/System/Library/Fonts/HelveticaNeue.ttc",
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
            // Return a default font ID
            font = 0;
        } else {
            std::cout << "[NanoVGRenderContext] Using default font" << std::endl;
        }
    }

    fontCache_[fontKey] = font;
    return font;
}

} // namespace flux
