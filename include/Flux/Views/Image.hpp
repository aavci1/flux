#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <string>

namespace flux {

// Image view component for displaying content images
struct Image {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> source = "";
    Property<BackgroundSize> contentMode = BackgroundSize::Cover;
    Property<BackgroundPosition> contentPosition = BackgroundPosition::Center;
    Property<bool> preserveAspectRatio = true;
    Property<float> imageOpacity = 1.0f;

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        std::string imagePath = source;
        if (imagePath.empty()) {
            return;
        }

        EdgeInsets paddingVal = padding;
        Rect contentBounds = {
            bounds.x + paddingVal.left,
            bounds.y + paddingVal.top,
            bounds.width - paddingVal.horizontal(),
            bounds.height - paddingVal.vertical()
        };

        if (contentBounds.width <= 0 || contentBounds.height <= 0) {
            return;
        }

        // Apply image opacity
        float finalOpacity = static_cast<float>(imageOpacity) * static_cast<float>(opacity);
        if (finalOpacity < 1.0f) {
            ctx.save();
            ctx.setOpacity(finalOpacity);
        }

        // Draw image based on content mode
        drawImageWithContentMode(ctx, imagePath, contentBounds);

        if (finalOpacity < 1.0f) {
            ctx.restore();
        }
    }

    Size preferredSize(TextMeasurement& /* textMeasurer */) const {
        EdgeInsets paddingVal = padding;

        // For now, return a default size
        // In a real implementation, you might want to load the image
        // and return its actual dimensions
        return {200 + paddingVal.horizontal(), 200 + paddingVal.vertical()};
    }

private:
    void drawImageWithContentMode(RenderContext& ctx, const std::string& imagePath, const Rect& bounds) const {
        BackgroundSize mode = contentMode;
        BackgroundPosition pos = contentPosition;

        switch (mode) {
            case BackgroundSize::Auto:
                // Draw at original size, positioned according to contentPosition
                drawImageAtPosition(ctx, imagePath, bounds, false);
                break;

            case BackgroundSize::Cover:
                // Scale to cover entire bounds, may crop
                drawImageScaledToCover(ctx, imagePath, bounds);
                break;

            case BackgroundSize::Contain:
                // Scale to fit entirely within bounds, may leave empty space
                drawImageScaledToContain(ctx, imagePath, bounds);
                break;

            case BackgroundSize::Stretch:
                // Stretch to fill exact bounds, may distort
                ctx.drawImage(imagePath, bounds);
                break;
        }
    }

    void drawImageAtPosition(RenderContext& ctx, const std::string& imagePath, const Rect& bounds, bool scaleToFit) const {
        // For now, just draw the image centered
        // In a real implementation, you would:
        // 1. Load the image to get its dimensions
        // 2. Calculate the position based on contentPosition
        // 3. Draw at the calculated position

        ctx.drawImage(imagePath, bounds);
    }

    void drawImageScaledToCover(RenderContext& ctx, const std::string& imagePath, const Rect& bounds) const {
        // For now, just draw the image to fill the bounds
        // In a real implementation, you would:
        // 1. Load the image to get its dimensions
        // 2. Calculate the scale factor to cover the entire area
        // 3. Calculate the crop area to center the image
        // 4. Draw the cropped and scaled image

        ctx.drawImage(imagePath, bounds);
    }

    void drawImageScaledToContain(RenderContext& ctx, const std::string& imagePath, const Rect& bounds) const {
        // For now, just draw the image to fit within bounds
        // In a real implementation, you would:
        // 1. Load the image to get its dimensions
        // 2. Calculate the scale factor to fit entirely within bounds
        // 3. Calculate the centered position
        // 4. Draw the scaled image at the calculated position

        ctx.drawImage(imagePath, bounds);
    }
};

} // namespace flux
