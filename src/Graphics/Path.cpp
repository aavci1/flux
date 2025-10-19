#include <Flux/Graphics/Path.hpp>
#include <algorithm>
#include <cmath>
#include <limits>

namespace flux {

Path::Path() = default;

Path::~Path() = default;

Path::Path(const Path& other)
    : commands_(other.commands_)
    , cachedBounds_(other.cachedBounds_)
    , boundsDirty_(other.boundsDirty_) {
}

Path& Path::operator=(const Path& other) {
    if (this != &other) {
        commands_ = other.commands_;
        cachedBounds_ = other.cachedBounds_;
        boundsDirty_ = other.boundsDirty_;
    }
    return *this;
}

Path::Path(Path&& other) noexcept
    : commands_(std::move(other.commands_))
    , cachedBounds_(other.cachedBounds_)
    , boundsDirty_(other.boundsDirty_) {
}

Path& Path::operator=(Path&& other) noexcept {
    if (this != &other) {
        commands_ = std::move(other.commands_);
        cachedBounds_ = other.cachedBounds_;
        boundsDirty_ = other.boundsDirty_;
    }
    return *this;
}

void Path::moveTo(const Point& point) {
    Command cmd(CommandType::MoveTo);
    cmd.data = {point.x, point.y};
    commands_.push_back(std::move(cmd));
    invalidateBounds();
}

void Path::lineTo(const Point& point) {
    Command cmd(CommandType::LineTo);
    cmd.data = {point.x, point.y};
    commands_.push_back(std::move(cmd));
    invalidateBounds();
}

void Path::quadTo(const Point& control, const Point& end) {
    Command cmd(CommandType::QuadTo);
    cmd.data = {control.x, control.y, end.x, end.y};
    commands_.push_back(std::move(cmd));
    invalidateBounds();
}

void Path::bezierTo(const Point& c1, const Point& c2, const Point& end) {
    Command cmd(CommandType::BezierTo);
    cmd.data = {c1.x, c1.y, c2.x, c2.y, end.x, end.y};
    commands_.push_back(std::move(cmd));
    invalidateBounds();
}

void Path::arcTo(const Point& p1, const Point& p2, float radius) {
    Command cmd(CommandType::ArcTo);
    cmd.data = {p1.x, p1.y, p2.x, p2.y, radius};
    commands_.push_back(std::move(cmd));
    invalidateBounds();
}

void Path::arc(const Point& center, float radius, float startAngle, float endAngle, bool clockwise) {
    Command cmd(CommandType::Arc);
    cmd.data = {center.x, center.y, radius, startAngle, endAngle, clockwise ? 1.0f : 0.0f};
    commands_.push_back(std::move(cmd));
    invalidateBounds();
}

void Path::rect(const Rect& rect, const CornerRadius& cornerRadius) {
    Command cmd(CommandType::Rect);
    cmd.data = {
        rect.x, rect.y, rect.width, rect.height,
        cornerRadius.topLeft, cornerRadius.topRight,
        cornerRadius.bottomRight, cornerRadius.bottomLeft
    };
    commands_.push_back(std::move(cmd));
    invalidateBounds();
}

void Path::circle(const Point& center, float radius) {
    Command cmd(CommandType::Circle);
    cmd.data = {center.x, center.y, radius};
    commands_.push_back(std::move(cmd));
    invalidateBounds();
}

void Path::ellipse(const Point& center, float radiusX, float radiusY) {
    Command cmd(CommandType::Ellipse);
    cmd.data = {center.x, center.y, radiusX, radiusY};
    commands_.push_back(std::move(cmd));
    invalidateBounds();
}

void Path::close() {
    Command cmd(CommandType::Close);
    commands_.push_back(std::move(cmd));
}

void Path::reset() {
    commands_.clear();
    invalidateBounds();
}

bool Path::isEmpty() const {
    return commands_.empty();
}

Rect Path::getBounds() const {
    if (boundsDirty_) {
        updateBounds();
    }
    return cachedBounds_;
}

void Path::invalidateBounds() {
    boundsDirty_ = true;
}

void Path::updateBounds() const {
    if (commands_.empty()) {
        cachedBounds_ = Rect{0, 0, 0, 0};
        boundsDirty_ = false;
        return;
    }
    
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float maxX = std::numeric_limits<float>::lowest();
    float maxY = std::numeric_limits<float>::lowest();
    
    auto updateMinMax = [&](float x, float y) {
        minX = std::min(minX, x);
        minY = std::min(minY, y);
        maxX = std::max(maxX, x);
        maxY = std::max(maxY, y);
    };
    
    for (const auto& cmd : commands_) {
        switch (cmd.type) {
            case CommandType::MoveTo:
            case CommandType::LineTo:
                if (cmd.data.size() >= 2) {
                    updateMinMax(cmd.data[0], cmd.data[1]);
                }
                break;
                
            case CommandType::QuadTo:
                if (cmd.data.size() >= 4) {
                    updateMinMax(cmd.data[0], cmd.data[1]);
                    updateMinMax(cmd.data[2], cmd.data[3]);
                }
                break;
                
            case CommandType::BezierTo:
                if (cmd.data.size() >= 6) {
                    updateMinMax(cmd.data[0], cmd.data[1]);
                    updateMinMax(cmd.data[2], cmd.data[3]);
                    updateMinMax(cmd.data[4], cmd.data[5]);
                }
                break;
                
            case CommandType::ArcTo:
                if (cmd.data.size() >= 5) {
                    updateMinMax(cmd.data[0], cmd.data[1]);
                    updateMinMax(cmd.data[2], cmd.data[3]);
                }
                break;
                
            case CommandType::Arc:
                if (cmd.data.size() >= 3) {
                    float cx = cmd.data[0];
                    float cy = cmd.data[1];
                    float r = cmd.data[2];
                    updateMinMax(cx - r, cy - r);
                    updateMinMax(cx + r, cy + r);
                }
                break;
                
            case CommandType::Rect:
                if (cmd.data.size() >= 4) {
                    float x = cmd.data[0];
                    float y = cmd.data[1];
                    float w = cmd.data[2];
                    float h = cmd.data[3];
                    updateMinMax(x, y);
                    updateMinMax(x + w, y + h);
                }
                break;
                
            case CommandType::Circle:
                if (cmd.data.size() >= 3) {
                    float cx = cmd.data[0];
                    float cy = cmd.data[1];
                    float r = cmd.data[2];
                    updateMinMax(cx - r, cy - r);
                    updateMinMax(cx + r, cy + r);
                }
                break;
                
            case CommandType::Ellipse:
                if (cmd.data.size() >= 4) {
                    float cx = cmd.data[0];
                    float cy = cmd.data[1];
                    float rx = cmd.data[2];
                    float ry = cmd.data[3];
                    updateMinMax(cx - rx, cy - ry);
                    updateMinMax(cx + rx, cy + ry);
                }
                break;
                
            case CommandType::Close:
                // No bounds impact
                break;
        }
    }
    
    cachedBounds_ = Rect{minX, minY, maxX - minX, maxY - minY};
    boundsDirty_ = false;
}

} // namespace flux

