#include <Flux/Graphics/RenderContext.hpp>
#include <nanovg.h>

// Forward declaration
struct NSVGpath;
#include <iostream>
#include <stack>
#include <unordered_map>
#include <memory>

namespace flux {

class NanoVGRenderContext : public RenderContext {
private:
    NVGcontext* nvgContext_;
    int width_;
    int height_;
    float dpiScaleX_;
    float dpiScaleY_;
    int framebufferWidth_;
    int framebufferHeight_;

    // Font cache
    std::unordered_map<std::string, int> fontCache_;

    // Image cache
    std::unordered_map<std::string, int> imageCache_;

public:
    NanoVGRenderContext(NVGcontext* nvgContext, int width, int height, float dpiScaleX = 1.0f, float dpiScaleY = 1.0f);
    ~NanoVGRenderContext() override = default;

    // ============================================================================
    // FRAME MANAGEMENT
    // ============================================================================
    void beginFrame() override;
    void clear(const Color& color = Color(1, 1, 1, 1)) override;
    void present() override;
    void resize(int width, int height) override;
    void updateDPIScale(float dpiScaleX, float dpiScaleY);

    // ============================================================================
    // STATE MANAGEMENT
    // ============================================================================
    void save() override;
    void restore() override;
    void reset() override;

    // ============================================================================
    // TRANSFORMATIONS
    // ============================================================================
    void translate(float x, float y) override;
    void rotate(float angle) override;
    void scale(float sx, float sy) override;
    void skewX(float angle) override;
    void skewY(float angle) override;
    void setTransform(float a, float b, float c, float d, float e, float f) override;
    void resetTransform() override;
    void getTransform(float* matrix) override;

    // ============================================================================
    // COMPOSITE OPERATIONS
    // ============================================================================
    void setCompositeOperation(CompositeOperation op) override;
    void setOpacity(float alpha) override;
    void setShapeAntiAlias(bool enabled) override;

    // ============================================================================
    // STROKE STYLING
    // ============================================================================
    void setStrokeColor(const Color& color) override;
    void setStrokeWidth(float width) override;
    void setLineCap(LineCap cap) override;
    void setLineJoin(LineJoin join) override;
    void setMiterLimit(float limit) override;
    void setDashPattern(const std::vector<float>& pattern, float offset = 0.0f) override;
    void setStrokeStyle(const StrokeStyle& style) override;

    // ============================================================================
    // FILL STYLING
    // ============================================================================
    void setFillStyle(const FillStyle& style) override;
    void setFillColor(const Color& color) override;
    void setPathWinding(PathWinding winding) override;

    // ============================================================================
    // PATH BUILDING
    // ============================================================================
    void beginPath() override;
    void closePath() override;
    void moveTo(const Point& point) override;
    void lineTo(const Point& point) override;
    void quadTo(const Point& control, const Point& end) override;
    void bezierTo(const Point& c1, const Point& c2, const Point& end) override;
    void arcTo(const Point& p1, const Point& p2, float radius) override;
    void arc(const Point& center, float radius, float startAngle, float endAngle, bool clockwise = false) override;

    // ============================================================================
    // PATH SHAPES
    // ============================================================================
    void addRect(const Rect& rect) override;
    void addRoundedRect(const Rect& rect, float cornerRadius) override;
    void addRoundedRectVarying(const Rect& rect, float radTopLeft, float radTopRight,
                               float radBottomRight, float radBottomLeft) override;
    void addCircle(const Point& center, float radius) override;
    void addEllipse(const Point& center, float radiusX, float radiusY) override;

    // ============================================================================
    // PATH RENDERING
    // ============================================================================
    void fill() override;
    void stroke() override;

    // ============================================================================
    // CONVENIENCE DRAWING METHODS
    // ============================================================================
    void drawRect(const Rect& rect, const Color& color) override;
    void drawRoundedRect(const Rect& rect, float cornerRadius, const Color& color) override;
    void drawRoundedRectBorder(const Rect& rect, float cornerRadius, const Color& color, float width) override;
    void drawCircle(const Point& center, float radius, const Color& color) override;
    void drawEllipse(const Point& center, float radiusX, float radiusY, const Color& color) override;
    void drawLine(const Point& start, const Point& end, const StrokeStyle& style) override;
    void drawArc(const Point& center, float radius, float startAngle, float endAngle, const StrokeStyle& style) override;

    // ============================================================================
    // TEXT RENDERING
    // ============================================================================
    void setFont(const std::string& name, FontWeight weight = FontWeight::regular) override;
    void setFontSize(float size) override;
    void setFontBlur(float blur) override;
    void setLetterSpacing(float spacing) override;
    void setLineHeight(float height) override;
    void setTextAlign(HorizontalAlignment hAlign, VerticalAlignment vAlign = VerticalAlignment::center) override;
    void setTextStyle(const TextStyle& style) override;

    void drawText(const std::string& text, const Point& position, const TextStyle& style) override;
    void drawText(const std::string& text, const Point& position, float fontSize, const Color& color,
                 FontWeight weight = FontWeight::regular,
                 HorizontalAlignment hAlign = HorizontalAlignment::leading,
                 VerticalAlignment vAlign = VerticalAlignment::center) override;
    void drawTextBox(const std::string& text, const Point& position, float breakWidth, const TextStyle& style) override;
    Size measureText(const std::string& text, const TextStyle& style) override;
    Size measureText(const std::string& text, float fontSize, FontWeight weight = FontWeight::regular) override;
    Rect getTextBounds(const std::string& text, const Point& position, const TextStyle& style) override;

    // ============================================================================
    // IMAGE MANAGEMENT
    // ============================================================================
    int createImage(const std::string& filename) override;
    int createImageMem(const unsigned char* data, int dataSize) override;
    int createImageRGBA(int width, int height, const unsigned char* data) override;
    void updateImage(int imageId, const unsigned char* data) override;
    Size getImageSize(int imageId) override;
    void deleteImage(int imageId) override;
    void drawImage(int imageId, const Rect& rect, float cornerRadius = 0, float alpha = 1.0f) override;
    void drawImage(const std::string& path, const Rect& rect, float cornerRadius = 0) override;

    // CSS-like image sizing methods
    void drawImageCover(int imageId, const Rect& rect, float cornerRadius = 0, float alpha = 1.0f) override;
    void drawImageCover(const std::string& path, const Rect& rect, float cornerRadius = 0) override;
    void drawImageContain(int imageId, const Rect& rect, float cornerRadius = 0, float alpha = 1.0f) override;
    void drawImageContain(const std::string& path, const Rect& rect, float cornerRadius = 0) override;

    // ============================================================================
    // CLIPPING
    // ============================================================================
    void clipRect(const Rect& rect) override;
    void clipRoundedRect(const Rect& rect, float cornerRadius) override;
    void intersectClipRect(const Rect& rect) override;
    void resetClip() override;

    // ============================================================================
    // SHADOWS AND EFFECTS
    // ============================================================================
    void drawShadow(const Rect& rect, float cornerRadius, const Shadow& shadow) override;

    // ============================================================================
    // UTILITIES
    // ============================================================================
    Point transformPoint(const Point& point) override;
    Rect transformRect(const Rect& rect) override;
    float degToRad(float degrees) override;
    float radToDeg(float radians) override;

    // ============================================================================
    // SVG SUPPORT
    // ============================================================================
    void renderNanoVGPath(NSVGpath* path, const Color& fillColor, const Color& strokeColor, float strokeWidth);

private:
    // Helper functions
    NVGcolor toNVGColor(const Color& color) const;
    NVGpaint toNVGFillStyle(const FillStyle& fillStyle) const;
    NVGpaint createShadowPaint(const Shadow& shadow) const;
    int getFont(const std::string& fontName, FontWeight weight);
    int getNVGLineCap(LineCap cap) const;
    int getNVGLineJoin(LineJoin join) const;
    int getNVGCompositeOperation(CompositeOperation op) const;
    int getNVGPathWinding(PathWinding winding) const;
    int getNVGTextAlign(HorizontalAlignment hAlign, VerticalAlignment vAlign) const;
};

} // namespace flux
