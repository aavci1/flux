#pragma once

#include <Flux/Core/Types.hpp>
#include <concepts>

namespace flux {

inline float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

inline Color lerp(const Color& a, const Color& b, float t) {
    return Color(
        a.r + (b.r - a.r) * t,
        a.g + (b.g - a.g) * t,
        a.b + (b.b - a.b) * t,
        a.a + (b.a - a.a) * t
    );
}

inline Point lerp(const Point& a, const Point& b, float t) {
    return Point(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t
    );
}

inline EdgeInsets lerp(const EdgeInsets& a, const EdgeInsets& b, float t) {
    return EdgeInsets(
        a.top + (b.top - a.top) * t,
        a.right + (b.right - a.right) * t,
        a.bottom + (b.bottom - a.bottom) * t,
        a.left + (b.left - a.left) * t
    );
}

inline CornerRadius lerp(const CornerRadius& a, const CornerRadius& b, float t) {
    return CornerRadius(
        a.topLeft + (b.topLeft - a.topLeft) * t,
        a.topRight + (b.topRight - a.topRight) * t,
        a.bottomRight + (b.bottomRight - a.bottomRight) * t,
        a.bottomLeft + (b.bottomLeft - a.bottomLeft) * t
    );
}

inline Rect lerp(const Rect& a, const Rect& b, float t) {
    return Rect(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.width + (b.width - a.width) * t,
        a.height + (b.height - a.height) * t
    );
}

template<typename T>
concept Animatable = requires(const T& a, const T& b, float t) {
    { flux::lerp(a, b, t) } -> std::convertible_to<T>;
};

} // namespace flux
