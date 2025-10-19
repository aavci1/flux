#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <functional>
#include <string>

namespace flux {

struct Button {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> text;
    Property<ButtonStyle> style = ButtonStyle::primary;
    Property<bool> disabled = false;
    Property<Color> color = Colors::blue;
    std::function<void()> onClick;

    mutable bool isHovered = false;
    mutable bool isPressed = false;

    void render(RenderContext& ctx, const Rect& bounds) const {
        const Rect& layout = bounds;
        // Calculate opacity including disabled state
        float opacityVal = opacity;
        bool isDisabled = disabled;
        if (isDisabled) {
            opacityVal *= 0.5f;
        }

        // Apply view decorations
        ctx.save();

        // Apply transforms
        Point offsetPt = offset;
        ctx.translate(offsetPt.x, offsetPt.y);

        if (static_cast<float>(rotation) != 0) {
            ctx.rotate(static_cast<float>(rotation));
        }

        float sx = scaleX, sy = scaleY;
        if (sx != 1.0f || sy != 1.0f) {
            ctx.scale(sx, sy);
        }

        if (opacityVal < 1.0f) {
            ctx.setOpacity(opacityVal);
        }

        // Determine colors based on style and state
        ButtonStyle buttonStyle = style;
        Color buttonColor = color;
        Color bgColor, borderColor, textColor;

        bool hovered = isHovered && !isDisabled;
        bool pressed = isPressed && !isDisabled;

        switch (buttonStyle) {
            case ButtonStyle::primary:
                bgColor = pressed ? buttonColor.darken(0.2) :
                         (hovered ? buttonColor.darken(0.1) : buttonColor);
                borderColor = bgColor;
                textColor = Color(1, 1, 1, 1);
                break;

            case ButtonStyle::secondary:
                bgColor = Color(0.9, 0.9, 0.9, 1);
                if (pressed) bgColor = bgColor.darken(0.1);
                else if (hovered) bgColor = bgColor.darken(0.05);
                borderColor = Color(0.7, 0.7, 0.7, 1);
                textColor = buttonColor;
                break;

            case ButtonStyle::outlined:
                bgColor = pressed ? Color(0.95, 0.95, 0.95, 1) :
                          (hovered ? Color(0.98, 0.98, 0.98, 1) : Color(1, 1, 1, 0));
                borderColor = buttonColor;
                textColor = buttonColor;
                break;

            case ButtonStyle::text:
                bgColor = pressed ? Color(0.9, 0.9, 0.9, 0.2) :
                          (hovered ? Color(0.9, 0.9, 0.9, 0.1) : Color(1, 1, 1, 0));
                borderColor = Color(1, 1, 1, 0);
                textColor = buttonColor;
                break;
        }

        // Draw button background
        float radius = static_cast<float>(cornerRadius);
        Path bgPath;
        bgPath.rect(layout, radius);
        ctx.setFillColor(bgColor);
        ctx.drawPath(bgPath, true, false);

        // Draw border for outlined style
        if (buttonStyle == ButtonStyle::outlined ||
            (buttonStyle == ButtonStyle::secondary && borderColor.a > 0)) {
            Path borderPath;
            borderPath.rect(layout, radius);
            ctx.setStrokeColor(borderColor);
            ctx.setStrokeWidth(1);
            ctx.drawPath(borderPath, false, true);
        }

        // Draw button text
        EdgeInsets paddingVal = padding;
        if (paddingVal.horizontal() == 0) {
            paddingVal = EdgeInsets(12, 24); // Default button padding
        }

        std::string buttonText = text;
        Size textSize = ctx.measureText(buttonText, 16, FontWeight::medium);

        Point textPos = {
            layout.x + (layout.width - textSize.width) / 2,
            layout.y + (layout.height + 16) / 2 - 4
        };

        ctx.drawText(buttonText, textPos, 16, textColor, FontWeight::medium);

        ctx.restore();
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        EdgeInsets paddingVal = padding;
        if (paddingVal.horizontal() == 0) {
            paddingVal = EdgeInsets(12, 24); // Default button padding
        }

        std::string buttonText = text;

        // Use accurate measurement from renderer
        Size textSize = textMeasurer.measureText(buttonText, 16, FontWeight::medium);
        return {textSize.width + paddingVal.horizontal(),
                textSize.height + paddingVal.vertical()};
    }
};

} // namespace flux
