#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <iostream>

namespace flux {

// ViewHelpers namespace for view rendering utilities
namespace ViewHelpers {

// Forward declarations for background image helper functions
inline void drawBackgroundImageWithSizing(RenderContext& ctx, const BackgroundImage& bgImage, const Rect& bounds);
inline void drawBackgroundImageAtPosition(RenderContext& ctx, const std::string& imagePath, const Rect& bounds,
                                        BackgroundPosition position, const Point& customPosition);
inline void drawBackgroundImageScaledToCover(RenderContext& ctx, const std::string& imagePath, const Rect& bounds,
                                           BackgroundPosition position, const Point& customPosition);
inline void drawBackgroundImageScaledToContain(RenderContext& ctx, const std::string& imagePath, const Rect& bounds,
                                             BackgroundPosition position, const Point& customPosition);

// Unified render function that handles decorations only
// Framework handles children rendering through layout tree
template<typename ViewType>
inline void renderView(const ViewType& view, RenderContext& ctx, const Rect& bounds) {
    ctx.save();

    // Apply transforms
    Point offsetPt = view.offset;
    ctx.translate(offsetPt.x, offsetPt.y);

    float rot = view.rotation;
    if (rot != 0) {
        ctx.rotate(rot);
    }

    float sx = view.scaleX, sy = view.scaleY;
    if (sx != 1.0f || sy != 1.0f) {
        ctx.scale(sx, sy);
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
        ctx.drawRect(bounds, CornerRadius(static_cast<float>(view.cornerRadius)));
    }

    // // Draw background image if present
    BackgroundImage bgImage = view.backgroundImage;
    if (bgImage.isValid()) {
        // Simple background image rendering for now
        ctx.save();
        if (bgImage.opacity < 1.0f) {
            ctx.setOpacity(bgImage.opacity);
        }

        // Create clipping path for rounded corners if needed
        if (static_cast<float>(view.cornerRadius) > 0) {
            Path clipPath;
            clipPath.rect(bounds, static_cast<float>(view.cornerRadius));
            ctx.clipPath(clipPath);
        }

        // Draw the background image with proper sizing
        drawBackgroundImageWithSizing(ctx, bgImage, bounds);

        ctx.restore();
    }

    // Draw border
    float borderWidth = view.borderWidth;
    Color borderColor = view.borderColor;
    if (borderWidth > 0 && borderColor.a > 0) {
        ctx.setFillStyle(FillStyle::none());
        ctx.setStrokeStyle(StrokeStyle::solid(borderColor, borderWidth));
        ctx.drawRect(bounds, CornerRadius(static_cast<float>(view.cornerRadius)));
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
                                        BackgroundPosition position, const Point& customPosition) {
    // For Auto size, we need to get the actual image dimensions
    // For now, we'll assume the image is loaded and use a placeholder approach
    // In a real implementation, you would load the image and get its dimensions

    // For now, just draw centered at original size
    // This is a simplified implementation - in practice you'd need image loading
    ctx.drawImage(imagePath, bounds, ImageFit::None);
}

// Helper function to draw background image scaled to cover
inline void drawBackgroundImageScaledToCover(RenderContext& ctx, const std::string& imagePath, const Rect& bounds,
                                           BackgroundPosition position, const Point& customPosition) {
    // Use the CSS-like Cover method that maintains aspect ratio
    ctx.drawImage(imagePath, bounds, ImageFit::Cover);
}

// Helper function to draw background image scaled to contain
inline void drawBackgroundImageScaledToContain(RenderContext& ctx, const std::string& imagePath, const Rect& bounds,
                                             BackgroundPosition position, const Point& customPosition) {
    // Use the CSS-like Contain method that maintains aspect ratio
    ctx.drawImage(imagePath, bounds, ImageFit::Contain);
}

} // namespace ViewHelpers

} // namespace flux