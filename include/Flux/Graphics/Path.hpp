#pragma once

#include <Flux/Core/Types.hpp>
#include <cstdint>
#include <vector>
#include <memory>

namespace flux {

enum class PathWinding {
    CounterClockwise,  // Solid shapes
    Clockwise          // Holes
};

/**
 * Path - Independent path construction class
 * 
 * Builds vector paths that can be drawn or used for clipping.
 * Similar to Skia's SkPath and QPainter's QPainterPath.
 *
 * All parameter data is stored in a single contiguous flat buffer
 * (data_) to eliminate per-command heap allocations.
 */
class Path {
public:
    Path();
    ~Path();
    
    Path(const Path& other);
    Path& operator=(const Path& other);
    Path(Path&& other) noexcept;
    Path& operator=(Path&& other) noexcept;
    
    // ============================================================================
    // PATH CONSTRUCTION
    // ============================================================================

    void setWinding(PathWinding winding);
    void moveTo(const Point& point);
    void lineTo(const Point& point);
    void quadTo(const Point& control, const Point& end);
    void bezierTo(const Point& c1, const Point& c2, const Point& end);
    void arcTo(const Point& p1, const Point& p2, float radius);
    void arc(const Point& center, float radius, float startAngle, float endAngle, bool clockwise = false);
    void rect(const Rect& rect, const CornerRadius& cornerRadius = CornerRadius());
    void circle(const Point& center, float radius);
    void ellipse(const Point& center, float radiusX, float radiusY);
    void close();
    void reset();
    
    // ============================================================================
    // QUERIES
    // ============================================================================
    
    bool isEmpty() const;
    Rect getBounds() const;
    uint64_t contentHash() const;

    // ============================================================================
    // INTERNAL TYPES (public for friend access; prefer the iteration API below)
    // ============================================================================

    enum class CommandType : uint8_t {
        SetWinding,
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
    
    struct Command {
        CommandType type;
        PathWinding winding = PathWinding::CounterClockwise;
        uint32_t dataOffset = 0;
        uint8_t  dataCount  = 0;
    };

    // ============================================================================
    // ITERATION API — used by NanoVG*, PathFlattener, etc.
    // ============================================================================

    struct CommandView {
        CommandType type;
        PathWinding winding;
        const float* data;
        uint8_t dataCount;
    };

    size_t commandCount() const { return commands_.size(); }

    CommandView command(size_t idx) const {
        const auto& c = commands_[idx];
        return {c.type, c.winding, data_.data() + c.dataOffset, c.dataCount};
    }

private:
    std::vector<Command> commands_;
    std::vector<float>   data_;
    mutable Rect cachedBounds_;
    mutable bool boundsDirty_ = true;

    void pushCommand(CommandType type, PathWinding w, std::initializer_list<float> params);
    void invalidateBounds();
    void updateBounds() const;
};

} // namespace flux
