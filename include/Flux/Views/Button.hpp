#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Core/Typography.hpp>
#include <functional>
#include <string>

namespace flux {

struct Button {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;

    Property<std::string> text;
    Property<Color> textColor = Colors::inherit;

    void init() {
        focusable = true;
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        bool hasFocus = ctx.isCurrentViewFocused();
        bool isHovered = ctx.isCurrentViewHovered();
        bool isPressed = ctx.isCurrentViewPressed();
        Element* el = ctx.currentElement();

        Color bgColor = static_cast<Color>(backgroundColor);
        if (isPressed) bgColor = bgColor.darken(0.15f);
        else if (isHovered) bgColor = bgColor.lighten(0.12f);

        if (el) bgColor = el->animateValue<Color>(AnimPropID::Custom0, bgColor);

        ctx.setFillStyle(FillStyle::solid(bgColor));
        ctx.setStrokeStyle(StrokeStyle::none());
        ctx.drawRect(bounds, cornerRadius);

        float bw = static_cast<float>(borderWidth);
        if (bw > 0) {
            Color bcVal = isHovered
                ? static_cast<Color>(borderColor).lighten(0.2f)
                : static_cast<Color>(borderColor);
            if (el) bcVal = el->animateValue<Color>(AnimPropID::Custom1, bcVal);
            Path border;
            border.rect(bounds, cornerRadius);
            ctx.setFillStyle(FillStyle::none());
            ctx.setStrokeStyle(StrokeStyle::solid(bcVal, bw));
            ctx.drawPath(border);
        }

        if (hasFocus && !isHovered) {
            Path focusRing;
            Rect focusRect = {bounds.x - 1, bounds.y - 1, bounds.width + 2, bounds.height + 2};
            focusRing.rect(focusRect, cornerRadius);
            ctx.setFillStyle(FillStyle::none());
            ctx.setStrokeStyle(StrokeStyle::solid(ctx.theme().focusRing, 2.0f));
            ctx.drawPath(focusRing);
        }

        float labelSize = Typography::callout;
        ctx.setTextStyle(makeTextStyle("default", FontWeight::regular, labelSize, Typography::lineHeightTight,
            Typography::trackingFor(labelSize, FontWeight::regular)));
        ctx.setFillStyle(FillStyle::solid(resolveColor(textColor, ctx.theme().onAccent)));
        ctx.drawText(static_cast<std::string>(text), bounds.center(), HorizontalAlignment::center, VerticalAlignment::center);
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        EdgeInsets paddingVal = padding;
        if (paddingVal.horizontal() == 0) {
            paddingVal = EdgeInsets(12, 24); // Default button padding
        }

        std::string buttonText = text;

        // Use accurate measurement from renderer
        float labelSize = Typography::callout;
        Size textSize = textMeasurer.measureText(buttonText,
            makeTextStyle("default", FontWeight::regular, labelSize, Typography::lineHeightTight,
                Typography::trackingFor(labelSize, FontWeight::regular)));
        return {textSize.width + paddingVal.horizontal(),
                textSize.height + paddingVal.vertical()};
    }

    // Handle keyboard activation (Enter/Space)
    bool handleKeyDown(const KeyEvent& event) const {
        if ((event.key == Key::Enter || event.key == Key::Space) && !event.isRepeat) {
            if (onClick) {
                onClick();
                return true;
            }
        }
        return false;
    }
};

} // namespace flux
