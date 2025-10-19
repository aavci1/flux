#pragma once

#include <Flux/Core/Types.hpp>
#include <vector>
#include <memory>

namespace flux {

// Forward declarations
class NanoVGRenderContext;

/**
 * Path - Independent path construction class
 * 
 * Builds vector paths that can be drawn or used for clipping.
 * Similar to Skia's SkPath and QPainter's QPainterPath.
 */
class Path {
public:
    Path();
    ~Path();
    
    // Copy and move
    Path(const Path& other);
    Path& operator=(const Path& other);
    Path(Path&& other) noexcept;
    Path& operator=(Path&& other) noexcept;
    
    // ============================================================================
    // PATH CONSTRUCTION
    // ============================================================================
    
    /**
     * Move the current point to a new position without drawing
     */
    void moveTo(const Point& point);
    
    /**
     * Draw a line from current point to the specified point
     */
    void lineTo(const Point& point);
    
    /**
     * Draw a quadratic Bezier curve
     */
    void quadTo(const Point& control, const Point& end);
    
    /**
     * Draw a cubic Bezier curve
     */
    void bezierTo(const Point& c1, const Point& c2, const Point& end);
    
    /**
     * Draw an arc between two points with a given radius
     */
    void arcTo(const Point& p1, const Point& p2, float radius);
    
    /**
     * Draw an arc centered at a point
     */
    void arc(const Point& center, float radius, float startAngle, float endAngle, bool clockwise = false);
    
    /**
     * Add a rectangle to the path
     */
    void rect(const Rect& rect, const CornerRadius& cornerRadius = CornerRadius());
    
    /**
     * Add a circle to the path
     */
    void circle(const Point& center, float radius);
    
    /**
     * Add an ellipse to the path
     */
    void ellipse(const Point& center, float radiusX, float radiusY);
    
    /**
     * Close the current path by drawing a line to the starting point
     */
    void close();
    
    /**
     * Clear all path data
     */
    void reset();
    
    // ============================================================================
    // QUERIES
    // ============================================================================
    
    /**
     * Check if the path has no elements
     */
    bool isEmpty() const;
    
    /**
     * Get the bounding box of the path
     */
    Rect getBounds() const;
    
private:
    // Path command types
    enum class CommandType {
        MoveTo,
        LineTo,
        QuadTo,
        BezierTo,
        ArcTo,
        Arc,
        Rect,
        Circle,
        Ellipse,
        Close
    };
    
    // Generic path command
    struct Command {
        CommandType type;
        std::vector<float> data;  // Points and parameters flattened
        
        Command(CommandType t) : type(t) {}
    };
    
    std::vector<Command> commands_;
    mutable Rect cachedBounds_;
    mutable bool boundsDirty_ = true;
    
    void invalidateBounds();
    void updateBounds() const;
    
    friend class NanoVGRenderContext;
};

} // namespace flux

