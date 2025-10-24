#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Graphics/Path.hpp>
#include <string>
#include <vector>

namespace flux {

// Enums for rendering styles
enum class LineCap {
    Butt,    // Flat ends
    Round,   // Rounded ends
    Square   // Square ends extending beyond the line
};

enum class LineJoin {
    Miter,   // Sharp corners
    Round,   // Rounded corners
    Bevel    // Cut corners
};

enum class PathWinding {
    CounterClockwise,  // Solid shapes
    Clockwise          // Holes
};

enum class CompositeOperation {
    SourceOver,           // Default: source over destination
    SourceIn,            // Source in destination
    SourceOut,           // Source out of destination
    Atop,                // Source atop destination
    DestinationOver,     // Destination over source
    DestinationIn,       // Destination in source
    DestinationOut,      // Destination out of source
    DestinationAtop,     // Destination atop source
    Lighter,             // Lighter of source and destination
    Copy,                // Copy source
    Xor                  // Exclusive or
};

enum class ImageFit {
    Fill,      // Stretch to fill rect (may distort aspect ratio)
    Contain,   // Scale to fit inside rect (maintains aspect, may letterbox)
    Cover,     // Scale to cover entire rect (maintains aspect, may crop)
    None       // Use original size, no scaling
};
// Unified fill style configuration
struct FillStyle {
    enum class Type {
        None,               // No fill
        Solid,              // Solid color fill
        LinearGradient,     // Linear gradient fill
        RadialGradient,     // Radial gradient fill
        BoxGradient,        // Box gradient fill (rounded rectangle)
        ImagePattern        // Image/texture pattern fill
    };
    
    Type type = Type::Solid;
    PathWinding winding = PathWinding::CounterClockwise;

    // ============================================================================
    // SOLID FILL PROPERTIES
    // ============================================================================
    Color color = Colors::black;

    // ============================================================================
    // GRADIENT PROPERTIES (shared by Linear, Radial, Box)
    // ============================================================================
    Color startColor = Colors::black;
    Color endColor = Colors::white;

    // Linear gradient specific
    Point startPoint = {0, 0};
    Point endPoint = {100, 0};

    // Radial gradient specific
    Point center = {50, 50};
    float innerRadius = 0.0f;
    float outerRadius = 100.0f;

    // Box gradient specific
    Rect bounds = {0, 0, 100, 100};
    float cornerRadius = 0.0f;
    float feather = 0.0f;

    // ============================================================================
    // IMAGE PATTERN PROPERTIES
    // ============================================================================
    int imageId = -1;
    Point imageOrigin = {0, 0};
    Size imageSize = {100, 100};
    float imageAngle = 0.0f;
    float imageAlpha = 1.0f;

    // ============================================================================
    // FACTORY METHODS
    // ============================================================================

    static FillStyle none() {
        FillStyle style;
        style.type = Type::None;
        return style;
    }

    static FillStyle solid(const Color& color) {
        FillStyle style;
        style.type = Type::Solid;
        style.color = color;
        return style;
    }

    static FillStyle linearGradient(const Point& start, const Point& end,
                                    const Color& startColor, const Color& endColor) {
        FillStyle style;
        style.type = Type::LinearGradient;
        style.startPoint = start;
        style.endPoint = end;
        style.startColor = startColor;
        style.endColor = endColor;
        return style;
    }

    static FillStyle radialGradient(const Point& center, float innerRadius, float outerRadius,
                                    const Color& startColor, const Color& endColor) {
        FillStyle style;
        style.type = Type::RadialGradient;
        style.center = center;
        style.innerRadius = innerRadius;
        style.outerRadius = outerRadius;
        style.startColor = startColor;
        style.endColor = endColor;
        return style;
    }

    static FillStyle boxGradient(const Rect& bounds, float cornerRadius, float feather,
                                 const Color& innerColor, const Color& outerColor) {
        FillStyle style;
        style.type = Type::BoxGradient;
        style.bounds = bounds;
        style.cornerRadius = cornerRadius;
        style.feather = feather;
        style.startColor = innerColor;
        style.endColor = outerColor;
        return style;
    }

    static FillStyle imagePattern(int imageId, const Point& origin, const Size& size,
                                  float angle = 0.0f, float alpha = 1.0f) {
        FillStyle style;
        style.type = Type::ImagePattern;
        style.imageId = imageId;
        style.imageOrigin = origin;
        style.imageSize = size;
        style.imageAngle = angle;
        style.imageAlpha = alpha;
        return style;
    }
};

// Stroke style configuration
struct StrokeStyle {
    enum class Type {
        None,           // No stroke
        Solid,          // Solid stroke
        Dashed,         // Dashed stroke
        Rounded,        // Rounded caps and joins
        Square          // Square caps
    };
    
    Type type = Type::Solid;
    Color color = Colors::black;
    float width = 1.0f;
    LineCap cap = LineCap::Butt;
    LineJoin join = LineJoin::Miter;
    float miterLimit = 4.0f;
    std::vector<float> dashPattern;  // Empty = solid line
    float dashOffset = 0.0f;

    // ============================================================================
    // FACTORY METHODS
    // ============================================================================

    static StrokeStyle none() {
        StrokeStyle style;
        style.type = Type::None;
        return style;
    }

    static StrokeStyle solid(const Color& color, float width = 1.0f) {
        StrokeStyle style;
        style.type = Type::Solid;
        style.color = color;
        style.width = width;
        return style;
    }

    static StrokeStyle dashed(const Color& color, float width, const std::vector<float>& pattern, float dashOffset = 0.0f) {
        StrokeStyle style;
        style.type = Type::Dashed;
        style.color = color;
        style.width = width;
        style.dashPattern = pattern;
        style.dashOffset = dashOffset;
        return style;
    }

    static StrokeStyle rounded(const Color& color, float width) {
        StrokeStyle style;
        style.type = Type::Rounded;
        style.color = color;
        style.width = width;
        style.cap = LineCap::Round;
        style.join = LineJoin::Round;
        return style;
    }

    static StrokeStyle square(const Color& color, float width) {
        StrokeStyle style;
        style.type = Type::Square;
        style.color = color;
        style.width = width;
        style.cap = LineCap::Square;
        style.join = LineJoin::Miter;
        return style;
    }
};

// Text style configuration
struct TextStyle {
    std::string fontName = "default";
    FontWeight weight = FontWeight::regular;
    float size = 16.0f;
    float letterSpacing = 0.0f;
    float lineHeight = 1.0f;

    // ============================================================================
    // FACTORY METHODS
    // ============================================================================

    static TextStyle regular(const std::string& fontName, float size) {
        TextStyle style;
        style.fontName = fontName;
        style.size = size;
        style.weight = FontWeight::regular;
        return style;
    }

    static TextStyle bold(const std::string& fontName, float size) {
        TextStyle style;
        style.fontName = fontName;
        style.size = size;
        style.weight = FontWeight::bold;
        return style;
    }

    static TextStyle light(const std::string& fontName, float size) {
        TextStyle style;
        style.fontName = fontName;
        style.size = size;
        style.weight = FontWeight::light;
        return style;
    }
};

class RenderContext : public TextMeasurement {
public:
    virtual ~RenderContext() = default;

    // ============================================================================
    // FRAME MANAGEMENT
    // ============================================================================

    virtual void beginFrame() = 0;
    virtual void clear(const Color& color = Color(1, 1, 1, 1)) = 0;
    virtual void present() = 0;
    virtual void resize(int width, int height) {}

    // ============================================================================
    // STATE MANAGEMENT
    // ============================================================================

    virtual void save() = 0;
    virtual void restore() = 0;
    virtual void reset() = 0;

    // ============================================================================
    // TRANSFORMATIONS
    // ============================================================================

    virtual void translate(float x, float y) = 0;
    virtual void rotate(float angle) = 0;
    virtual void scale(float sx, float sy) = 0;
    virtual void skewX(float angle) = 0;
    virtual void skewY(float angle) = 0;
    virtual void setTransform(float a, float b, float c, float d, float e, float f) = 0;
    virtual void resetTransform() = 0;
    virtual void getTransform(float* matrix) = 0;  // Returns 6-element matrix

    // ============================================================================
    // COMPOSITE OPERATIONS
    // ============================================================================

    virtual void setCompositeOperation(CompositeOperation op) = 0;
    virtual void setOpacity(float alpha) = 0;
    virtual void setShapeAntiAlias(bool enabled) = 0;

    // ============================================================================
    // STROKE STYLING
    // ============================================================================

    virtual void setStrokeColor(const Color& color) = 0;
    virtual void setStrokeWidth(float width) = 0;
    virtual void setLineCap(LineCap cap) = 0;
    virtual void setLineJoin(LineJoin join) = 0;
    virtual void setMiterLimit(float limit) = 0;
    virtual void setDashPattern(const std::vector<float>& pattern, float offset = 0.0f) = 0;
    virtual void setStrokeStyle(const StrokeStyle& style) = 0;

    // ============================================================================
    // FILL STYLING
    // ============================================================================

    virtual void setFillColor(const Color& color) = 0;
    virtual void setPathWinding(PathWinding winding) = 0;
    virtual void setFillStyle(const FillStyle& style) = 0;


    // ============================================================================
    // PATH RENDERING
    // ============================================================================

    /**
     * Draw a path to the screen using current fill and stroke styles
     * @param path The path to draw
     */
    virtual void drawPath(const Path& path) = 0;

    // ============================================================================
    // DIRECT SHAPE DRAWING (using current styles)
    // ============================================================================

    /**
     * Draw a circle using current fill and stroke styles
     * @param center Center point of the circle
     * @param radius Radius of the circle
     */
    virtual void drawCircle(const Point& center, float radius) = 0;

    /**
     * Draw a line using current stroke style (no fill applied)
     * @param start Start point of the line
     * @param end End point of the line
     */
    virtual void drawLine(const Point& start, const Point& end) = 0;

    /**
     * Draw a rectangle using current fill and stroke styles
     * @param rect Rectangle bounds
     * @param cornerRadius Corner radius for rounded rectangles (default: no rounding)
     */
    virtual void drawRect(const Rect& rect, const CornerRadius& cornerRadius = CornerRadius()) = 0;

    /**
     * Draw an ellipse using current fill and stroke styles
     * @param center Center point of the ellipse
     * @param radiusX Horizontal radius
     * @param radiusY Vertical radius
     */
    virtual void drawEllipse(const Point& center, float radiusX, float radiusY) = 0;

    /**
     * Draw an arc using current fill and stroke styles
     * @param center Center point of the arc
     * @param radius Radius of the arc
     * @param startAngle Start angle in radians
     * @param endAngle End angle in radians
     * @param clockwise On whether to draw clockwise (default: false)
     */
    virtual void drawArc(const Point& center, float radius, float startAngle, float endAngle, bool clockwise = false) = 0;

    // ============================================================================
    // TEXT RENDERING
    // ============================================================================

    virtual void setFont(const std::string& name, FontWeight weight = FontWeight::regular) = 0;
    virtual void setFontSize(float size) = 0;
    virtual void setFontBlur(float blur) = 0;
    virtual void setLetterSpacing(float spacing) = 0;
    virtual void setLineHeight(float height) = 0;
    virtual void setTextStyle(const TextStyle& style) = 0;

    virtual void drawText(const std::string& text, const Point& position, HorizontalAlignment hAlign, VerticalAlignment vAlign) = 0;
    virtual Size measureText(const std::string& text, const TextStyle& style) = 0;
    virtual Rect getTextBounds(const std::string& text, const Point& position, const TextStyle& style) = 0;

    // ============================================================================
    // IMAGE MANAGEMENT
    // ============================================================================

    virtual int createImage(const std::string& filename) = 0;
    virtual int createImageMem(const unsigned char* data, int dataSize) = 0;
    virtual int createImageRGBA(int width, int height, const unsigned char* data) = 0;
    virtual void updateImage(int imageId, const unsigned char* data) = 0;
    virtual Size getImageSize(int imageId) = 0;
    virtual void deleteImage(int imageId) = 0;
    
    // Unified image rendering with ImageFit enum and CornerRadius
    virtual void drawImage(int imageId, const Rect& rect, 
                          ImageFit fit = ImageFit::Fill,
                          const CornerRadius& cornerRadius = CornerRadius(),
                          float alpha = 1.0f) = 0;
    virtual void drawImage(const std::string& path, const Rect& rect,
                          ImageFit fit = ImageFit::Fill,
                          const CornerRadius& cornerRadius = CornerRadius(),
                          float alpha = 1.0f) = 0;

    // ============================================================================
    // CLIPPING
    // ============================================================================

    /**
     * Clip rendering to the given path
     * @param path The path defining the clipping region
     */
    virtual void clipPath(const Path& path) = 0;
    
    /**
     * Reset clipping to the full render area
     */
    virtual void resetClip() = 0;

    // ============================================================================
    // UTILITIES
    // ============================================================================

    virtual Point transformPoint(const Point& point) = 0;
    virtual Rect transformRect(const Rect& rect) = 0;
    virtual float degToRad(float degrees) = 0;
    virtual float radToDeg(float radians) = 0;

    // ============================================================================
    // FOCUS STATE (for rendering focus indicators)
    // ============================================================================
    
    /**
     * Set the focus key for the currently rendering view
     * This allows views to check if they have focus during rendering
     */
    virtual void setCurrentFocusKey(const std::string& focusKey) = 0;
    
    /**
     * Get the focus key that should be highlighted
     * Returns empty string if no view has focus
     */
    virtual std::string getFocusedKey() const = 0;
    
    /**
     * Check if the currently rendering view has focus
     */
    virtual bool isCurrentViewFocused() const = 0;

    // ============================================================================
    // LEGACY COMPATIBILITY (deprecated but kept for transition)
    // ============================================================================

};

} // namespace flux
