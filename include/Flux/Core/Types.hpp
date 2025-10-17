#pragma once

#include <variant>
#include <cstdint>
#include <algorithm>
#include <string>

namespace flux {

// Basic geometry types
struct Point {
    float x, y;

    constexpr Point(float x = 0, float y = 0) : x(x), y(y) {}
    constexpr Point operator+(const Point& other) const { return {x + other.x, y + other.y}; }
    constexpr Point operator-(const Point& other) const { return {x - other.x, y - other.y}; }
    constexpr Point operator*(float scalar) const { return {x * scalar, y * scalar}; }
    constexpr bool operator==(const Point& other) const = default;
};

struct Size {
    float width, height;

    constexpr Size(float width = 0, float height = 0) : width(width), height(height) {}
    constexpr bool isEmpty() const { return width <= 0 || height <= 0; }
    constexpr float area() const { return width * height; }
    constexpr bool operator==(const Size& other) const = default;
};

struct Rect {
    float x, y, width, height;

    constexpr Rect(float x = 0, float y = 0, float width = 0, float height = 0)
        : x(x), y(y), width(width), height(height) {}

    constexpr Point origin() const { return {x, y}; }
    constexpr Size size() const { return {width, height}; }
    constexpr Point center() const { return {x + width/2, y + height/2}; }
    constexpr bool contains(const Point& p) const {
        return p.x >= x && p.x <= x + width && p.y >= y && p.y <= y + height;
    }
    constexpr bool operator==(const Rect& other) const = default;
};

struct EdgeInsets {
    float top, right, bottom, left;

    EdgeInsets() : top(0), right(0), bottom(0), left(0) {}
    EdgeInsets(float all) : top(all), right(all), bottom(all), left(all) {}
    EdgeInsets(float v, float h) : top(v), right(h), bottom(v), left(h) {}
    EdgeInsets(float t, float r, float b, float l) : top(t), right(r), bottom(b), left(l) {}

    // Enable: .padding = 20
    EdgeInsets& operator=(float value) {
        top = right = bottom = left = value;
        return *this;
    }

    constexpr float horizontal() const { return left + right; }
    constexpr float vertical() const { return top + bottom; }
    constexpr bool operator==(const EdgeInsets& other) const = default;
    constexpr bool operator!=(const EdgeInsets& other) const = default;
};

// Color system
struct Color {
    float r, g, b, a;

    constexpr Color(float r = 0, float g = 0, float b = 0, float a = 1.0f)
        : r(r), g(g), b(b), a(a) {}

    static constexpr Color rgb(uint8_t r, uint8_t g, uint8_t b) {
        return Color(r / 255.0f, g / 255.0f, b / 255.0f, 1.0f);
    }

    static constexpr Color hex(uint32_t hex) {
        return rgb((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF);
    }

    constexpr Color opacity(float alpha) const {
        return Color(r, g, b, alpha);
    }

    constexpr Color darken(float amount) const {
        float factor = 1.0f - amount;
        return Color(r * factor, g * factor, b * factor, a);
    }

    constexpr bool operator==(const Color& other) const = default;
    constexpr bool operator!=(const Color& other) const = default;
};

// Predefined colors namespace
namespace Colors {
    constexpr Color white = Color(1, 1, 1, 1);
    constexpr Color black = Color(0, 0, 0, 1);
    constexpr Color red = Color::hex(0xF44336);
    constexpr Color blue = Color::hex(0x2196F3);
    constexpr Color green = Color::hex(0x4CAF50);
    constexpr Color yellow = Color::hex(0xFFD700);
    constexpr Color gray = Color::hex(0x9E9E9E);
    constexpr Color transparent = Color(0, 0, 0, 0);
    constexpr Color darkGray = Color::hex(0x424242);
    constexpr Color lightGray = Color::hex(0xEEEEEE);
}

struct Shadow {
    float offsetX = 0;
    float offsetY = 0;
    float blurRadius = 0;
    float spreadRadius = 0;
    Color color = Colors::black;
    float opacity = 0.3;

    Shadow() = default;
    Shadow(float x, float y, float blur, float spread = 0)
        : offsetX(x), offsetY(y), blurRadius(blur), spreadRadius(spread) {}

    // Enable: .shadow = {2, 4, 8}  // x, y, blur
    Shadow& operator=(std::initializer_list<float> values) {
        auto it = values.begin();
        if (it != values.end()) offsetX = *it++;
        if (it != values.end()) offsetY = *it++;
        if (it != values.end()) blurRadius = *it++;
        if (it != values.end()) spreadRadius = *it++;
        return *this;
    }

    // ============================================================================
    // FACTORY METHODS
    // ============================================================================

    static Shadow drop(float offsetX, float offsetY, float blur, const Color& color = Colors::black) {
        Shadow shadow;
        shadow.offsetX = offsetX;
        shadow.offsetY = offsetY;
        shadow.blurRadius = blur;
        shadow.color = color;
        return shadow;
    }

    static Shadow inner(float offsetX, float offsetY, float blur, const Color& color = Colors::black) {
        Shadow shadow;
        shadow.offsetX = offsetX;
        shadow.offsetY = offsetY;
        shadow.blurRadius = blur;
        shadow.color = color;
        shadow.spreadRadius = -1; // Negative for inner shadows
        return shadow;
    }

    static Shadow glow(float blur, const Color& color = Colors::black) {
        Shadow shadow;
        shadow.blurRadius = blur;
        shadow.color = color;
        return shadow;
    }

    static Shadow subtle(float offsetX, float offsetY, float blur, const Color& color = Colors::black) {
        Shadow shadow;
        shadow.offsetX = offsetX;
        shadow.offsetY = offsetY;
        shadow.blurRadius = blur;
        shadow.color = color;
        shadow.opacity = 0.1f; // Very subtle
        return shadow;
    }

    bool operator==(const Shadow& other) const = default;
};

// Enums

// CSS Flexbox-style alignment for stacks (main axis)
enum class JustifyContent {
    start,          // Pack items to start (CSS: flex-start)
    center,         // Center items
    end,            // Pack items to end (CSS: flex-end)
    spaceBetween,   // Equal spacing between items (CSS: space-between)
    spaceAround,    // Equal spacing around items (CSS: space-around)
    spaceEvenly     // Equal spacing everywhere (CSS: space-evenly)
};

// CSS Flexbox-style alignment for stacks (cross axis)
enum class AlignItems {
    start,     // Align to start (CSS: flex-start)
    center,    // Center alignment
    end,       // Align to end (CSS: flex-end)
    stretch,   // Stretch to fill (CSS: stretch)
    baseline   // Align baselines (CSS: baseline)
};

// For Text and content components - horizontal alignment
enum class HorizontalAlignment {
    leading,   // Left in LTR, right in RTL
    center,    // Center alignment
    trailing,  // Right in LTR, left in RTL
    justify    // Full justification
};

// For Text and content components - vertical alignment
enum class VerticalAlignment {
    top,       // Align to top
    center,    // Center alignment
    bottom     // Align to bottom
};

enum class FontWeight {
    thin = 100, light = 300, regular = 400,
    medium = 500, semibold = 600, bold = 700,
    heavy = 900
};

enum class ButtonStyle {
    primary, secondary, outlined, text
};


// Text measurement interface - provides only text measurement capabilities
// This is a minimal interface that custom view developers can safely use
class TextMeasurement {
public:
    virtual ~TextMeasurement() = default;

    // Measure text dimensions using the current renderer's font system
    virtual Size measureText(const std::string& text, float fontSize, FontWeight weight = FontWeight::regular) = 0;
};

} // namespace flux
