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

    // Transform stack
    std::stack<float> transformStack_;

    // Font cache
    std::unordered_map<std::string, int> fontCache_;

public:
    NanoVGRenderContext(NVGcontext* nvgContext, int width, int height, float dpiScaleX = 1.0f, float dpiScaleY = 1.0f);
    ~NanoVGRenderContext() override = default;

    // Basic shapes
    void drawRect(const Rect& rect, const Color& color) override;
    void drawRoundedRect(const Rect& rect, float cornerRadius, const Color& color) override;
    void drawRoundedRectBorder(const Rect& rect, float cornerRadius, const Color& color, float width) override;
    void drawCircle(const Point& center, float radius, const Color& color) override;
    void drawArc(const Point& center, float radius, float startAngle, float endAngle, const Color& color, float strokeWidth) override;
    void drawLine(const Point& start, const Point& end, float width, const Color& color) override;
    void drawBezier(const Point& c1, const Point& c2, const Point& end, const Color& fillColor, const Color& strokeColor, float strokeWidth) override;
    void drawQuad(const Point& control, const Point& end, const Color& fillColor, const Color& strokeColor, float strokeWidth) override;

    // Path building for complex shapes
    void beginPath() override;
    void moveTo(const Point& point) override;
    void lineTo(const Point& point) override;
    void arcTo(const Point& center, float radius, float startAngle, float endAngle) override;
    void quadTo(const Point& control, const Point& end) override;
    void bezierTo(const Point& c1, const Point& c2, const Point& end) override;
    void closePath() override;
    void fillPath(const Color& fillColor) override;
    void strokePath(const Color& strokeColor, float strokeWidth) override;

    // Direct NanoVG path rendering for SVG
    void renderNanoVGPath(NSVGpath* path, const Color& fillColor, const Color& strokeColor, float strokeWidth);
    void drawShadow(const Rect& rect, float cornerRadius, const Shadow& shadow) override;

    // Text rendering
    void drawText(const std::string& text, const Point& position, float fontSize, const Color& color,
                 FontWeight weight = FontWeight::regular,
                 HorizontalAlignment align = HorizontalAlignment::leading) override;
    void drawText(const std::string& text, const Point& position, float fontSize, const Color& color,
                 FontWeight weight,
                 HorizontalAlignment hAlign,
                 VerticalAlignment vAlign) override;
    Size measureText(const std::string& text, float fontSize, FontWeight weight = FontWeight::regular) override;

    // Image rendering
    void drawImage(const std::string& path, const Rect& rect, float cornerRadius = 0) override;

    // Transform operations
    void save() override;
    void restore() override;
    void translate(float x, float y) override;
    void rotate(float angle) override;
    void scale(float sx, float sy) override;
    void setOpacity(float opacity) override;

    // Clipping
    void clipRect(const Rect& rect) override;
    void clipRoundedRect(const Rect& rect, float cornerRadius) override;

    // Frame management
    void beginFrame() override;
    void clear(const Color& color = Color(1, 1, 1, 1)) override;
    void present() override;
    void resize(int width, int height) override;
    void updateDPIScale(float dpiScaleX, float dpiScaleY);

private:
    // Helper functions
    NVGcolor toNVGColor(const Color& color) const;
    NVGpaint createShadowPaint(const Shadow& shadow) const;
    int getFont(const std::string& fontName, FontWeight weight);
};

} // namespace flux
