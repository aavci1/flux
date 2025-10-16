#pragma once

#include <Flux/Core/Types.hpp>
#include <string>

namespace flux {

class RenderContext : public TextMeasurement {
public:
    virtual ~RenderContext() = default;

    // Basic shapes
    virtual void drawRect(const Rect& rect, const Color& color) = 0;
    virtual void drawRoundedRect(const Rect& rect, float cornerRadius, const Color& color) = 0;
    virtual void drawRoundedRectBorder(const Rect& rect, float cornerRadius,
                                       const Color& color, float width) = 0;
    virtual void drawCircle(const Point& center, float radius, const Color& color) = 0;
    virtual void drawArc(const Point& center, float radius, float startAngle, float endAngle, const Color& color, float strokeWidth) = 0;
    virtual void drawLine(const Point& start, const Point& end, float width, const Color& color) = 0;

    virtual void drawBezier(const Point& c1, const Point& c2, const Point& end, const Color& fillColor, const Color& strokeColor, float strokeWidth) = 0;
    virtual void drawQuad(const Point& control, const Point& end, const Color& fillColor, const Color& strokeColor, float strokeWidth) = 0;

    // Path building for complex shapes
    virtual void beginPath() = 0;
    virtual void moveTo(const Point& point) = 0;
    virtual void lineTo(const Point& point) = 0;
    virtual void arcTo(const Point& center, float radius, float startAngle, float endAngle) = 0;
    virtual void quadTo(const Point& control, const Point& end) = 0;
    virtual void bezierTo(const Point& c1, const Point& c2, const Point& end) = 0;
    virtual void closePath() = 0;
    virtual void fillPath(const Color& fillColor) = 0;
    virtual void strokePath(const Color& strokeColor, float strokeWidth) = 0;

    // Shadows
    virtual void drawShadow(const Rect& rect, float cornerRadius, const Shadow& shadow) = 0;

    // Text
    virtual void drawText(const std::string& text, const Point& position,
                         float fontSize, const Color& color,
                         FontWeight weight = FontWeight::regular,
                         HorizontalAlignment align = HorizontalAlignment::leading) = 0;

    virtual void drawText(const std::string& text, const Point& position,
                         float fontSize, const Color& color,
                         FontWeight weight,
                         HorizontalAlignment hAlign,
                         VerticalAlignment vAlign) = 0;

    virtual Size measureText(const std::string& text, float fontSize,
                            FontWeight weight = FontWeight::regular) = 0;

    // Images
    virtual void drawImage(const std::string& path, const Rect& rect, float cornerRadius = 0) = 0;

    // Transforms
    virtual void save() = 0;
    virtual void restore() = 0;
    virtual void translate(float x, float y) = 0;
    virtual void rotate(float angle) = 0;
    virtual void scale(float sx, float sy) = 0;
    virtual void setOpacity(float opacity) = 0;

    // Clipping
    virtual void clipRect(const Rect& rect) = 0;
    virtual void clipRoundedRect(const Rect& rect, float cornerRadius) = 0;

    // Frame management
    virtual void beginFrame() = 0;
    virtual void clear(const Color& color = Color(1, 1, 1, 1)) = 0;
    virtual void present() = 0;

    // Window management
    virtual void resize(int width, int height) {}
};

} // namespace flux
