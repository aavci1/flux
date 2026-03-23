#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Graphics/RenderContext.hpp>

namespace flux {

inline Color resolveColor(Color c, Color themeDefault) {
    return c.isInherit() ? themeDefault : c;
}

namespace ViewHelpers {

inline void drawBackgroundImageWithSizing(RenderContext& ctx, const BackgroundImage& bgImage, const Rect& bounds);
inline void drawBackgroundImageAtPosition(RenderContext& ctx, const std::string& imagePath, const Rect& bounds,
                                        BackgroundPosition position, const Point& customPosition);
inline void drawBackgroundImageScaledToCover(RenderContext& ctx, const std::string& imagePath, const Rect& bounds,
                                           BackgroundPosition position, const Point& customPosition);
inline void drawBackgroundImageScaledToContain(RenderContext& ctx, const std::string& imagePath, const Rect& bounds,
                                             BackgroundPosition position, const Point& customPosition);

namespace detail {
template<typename V> auto hasOffset(int) -> decltype(V::offset, std::true_type{});
template<typename V> auto hasOffset(...) -> std::false_type;
template<typename V> auto hasRotation(int) -> decltype(V::rotation, std::true_type{});
template<typename V> auto hasRotation(...) -> std::false_type;
template<typename V> auto hasScaleX(int) -> decltype(V::scaleX, std::true_type{});
template<typename V> auto hasScaleX(...) -> std::false_type;
template<typename V> auto hasScaleY(int) -> decltype(V::scaleY, std::true_type{});
template<typename V> auto hasScaleY(...) -> std::false_type;
}

template<typename ViewType>
inline void renderView(const ViewType& view, RenderContext& ctx, const Rect& bounds) {
    ctx.save();

    if constexpr (decltype(detail::hasOffset<ViewType>(0))::value) {
        Point offsetPt = view.offset;
        ctx.translate(offsetPt.x, offsetPt.y);
    }

    if constexpr (decltype(detail::hasRotation<ViewType>(0))::value) {
        float rot = view.rotation;
        if (rot != 0) ctx.rotate(rot);
    }

    if constexpr (decltype(detail::hasScaleX<ViewType>(0))::value) {
        float sx = view.scaleX, sy = view.scaleY;
        if (sx != 1.0f || sy != 1.0f) ctx.scale(sx, sy);
    }

    float opacityVal = view.opacity;
    if (opacityVal < 1.0f) {
        ctx.setOpacity(opacityVal);
    }

    // Draw background
    Color bgColor = view.backgroundColor;
    if (bgColor.a > 0) {
        ctx.setFillStyle(FillStyle::solid(bgColor));
        ctx.setStrokeStyle(StrokeStyle::none());
        ctx.drawRect(bounds, view.cornerRadius);
    }

    // // Draw background image if present
    BackgroundImage bgImage = view.backgroundImage;
    if (bgImage.isValid()) {
        // Draw the background image with proper sizing
        drawBackgroundImageWithSizing(ctx, bgImage, bounds);
    }

    // Draw border
    float borderWidth = view.borderWidth;
    Color borderColor = view.borderColor;
    if (borderWidth > 0 && borderColor.a > 0) {
        ctx.setFillStyle(FillStyle::none());
        ctx.setStrokeStyle(StrokeStyle::solid(borderColor, borderWidth));
        ctx.drawRect(bounds, view.cornerRadius);
    }

    ctx.restore();
}

// Helper function to draw background image with proper CSS-like sizing
inline void drawBackgroundImageWithSizing(RenderContext& ctx, const BackgroundImage& bgImage, const Rect& bounds) {
    const std::string& imagePath = bgImage.imagePath;

    switch (bgImage.size) {
        case BackgroundSize::Auto:
            // Draw at original size, positioned according to backgroundPosition
            drawBackgroundImageAtPosition(ctx, imagePath, bounds, bgImage.position, bgImage.customPosition);
            break;

        case BackgroundSize::Cover:
            // Scale to cover entire bounds, may crop
            drawBackgroundImageScaledToCover(ctx, imagePath, bounds, bgImage.position, bgImage.customPosition);
            break;

        case BackgroundSize::Contain:
            // Scale to fit entirely within bounds, may leave empty space
            drawBackgroundImageScaledToContain(ctx, imagePath, bounds, bgImage.position, bgImage.customPosition);
            break;

        case BackgroundSize::Stretch:
            // Stretch to fill exact bounds, may distort
            ctx.drawImage(imagePath, bounds, ImageFit::Fill);
            break;
    }
}

// Helper function to draw background image at a specific position (Auto size)
inline void drawBackgroundImageAtPosition(RenderContext& ctx, const std::string& imagePath, const Rect& bounds,
                                        BackgroundPosition /*position*/, const Point& /*customPosition*/) {
    // For Auto size, we need to get the actual image dimensions
    // For now, we'll assume the image is loaded and use a placeholder approach
    // In a real implementation, you would load the image and get its dimensions

    // For now, just draw centered at original size
    // This is a simplified implementation - in practice you'd need image loading
    ctx.drawImage(imagePath, bounds, ImageFit::None);
}

// Helper function to draw background image scaled to cover
inline void drawBackgroundImageScaledToCover(RenderContext& ctx, const std::string& imagePath, const Rect& bounds,
                                           BackgroundPosition /*position*/, const Point& /*customPosition*/) {
    // Use the CSS-like Cover method that maintains aspect ratio
    ctx.drawImage(imagePath, bounds, ImageFit::Cover);
}

// Helper function to draw background image scaled to contain
inline void drawBackgroundImageScaledToContain(RenderContext& ctx, const std::string& imagePath, const Rect& bounds,
                                             BackgroundPosition /*position*/, const Point& /*customPosition*/) {
    // Use the CSS-like Contain method that maintains aspect ratio
    ctx.drawImage(imagePath, bounds, ImageFit::Contain);
}

/**
 * Input chrome: fill + optional outline. If `borderWidth` is 0, only the fill is drawn and no
 * hover/focus stroke is shown (no focus ring behavior on the border).
 */
inline void drawInputFieldChrome(RenderContext& ctx, const Rect& bounds,
                                 Color bgColor, Color borderCol, Color focusBorderColor,
                                 float cornerRadius, float borderWidth, float focusRingWidth) {
    ctx.setFillStyle(FillStyle::solid(bgColor));
    ctx.setStrokeStyle(StrokeStyle::none());
    ctx.drawRect(bounds, CornerRadius(cornerRadius));

    if (borderWidth <= 0.0f) {
        return;
    }

    bool isFocused = ctx.isCurrentViewFocused();
    bool isHovered = ctx.isCurrentViewHovered();

    Color bc = isFocused ? focusBorderColor
             : isHovered ? borderCol.lighten(0.3f)
             : borderCol;
    float bw = isFocused ? focusRingWidth : borderWidth;
    Path border;
    border.rect(bounds, CornerRadius(cornerRadius));
    ctx.setFillStyle(FillStyle::none());
    ctx.setStrokeStyle(StrokeStyle::solid(bc, bw));
    ctx.drawPath(border);
}

} // namespace ViewHelpers

} // namespace flux