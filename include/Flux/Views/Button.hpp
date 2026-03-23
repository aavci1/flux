#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Core/Typography.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <functional>
#include <string>

namespace flux {

struct Button {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;

    Property<std::string> text;
    /** 0 uses `theme.buttonFontSize`. */
    Property<float> fontSize = 0.0f;
    Property<Color> textColor = Colors::inherit;

    void init() {
        focusable = true;
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        bool hasFocus = ctx.isCurrentViewFocused();
        bool isHovered = ctx.isCurrentViewHovered();
        bool isPressed = ctx.isCurrentViewPressed();

        const auto& th = ctx.theme();

        Color bgColor = resolveColor(backgroundColor, th.buttonBackground);

        if (isPressed) {
            bgColor = bgColor.darken(0.15f);
        } else if (isHovered) {
            bgColor = bgColor.lighten(0.12f);
        }

        CornerRadius cr = static_cast<CornerRadius>(cornerRadius);
        if (cr.isZero()) {
            cr = th.buttonCornerRadius;
        }

        ctx.setFillStyle(FillStyle::solid(bgColor));
        ctx.setStrokeStyle(StrokeStyle::none());
        ctx.drawRect(bounds, cr);

        float bw = static_cast<float>(borderWidth);
        if (bw <= 0.0f) {
            bw = th.buttonBorderWidth;
        }

        if (bw > 0.0f) {
            Color bcVal = resolveColor(borderColor, th.buttonBorderColor);
            Path border;
            border.rect(bounds, cr);
            ctx.setFillStyle(FillStyle::none());
            ctx.setStrokeStyle(StrokeStyle::solid(isHovered ? bcVal.lighten(0.2f) : bcVal, bw));
            ctx.drawPath(border);
        }

        if (hasFocus && !isHovered && bw > 0.0f) {
            Path focusRing;
            Rect focusRect = {bounds.x - 1, bounds.y - 1, bounds.width + 2, bounds.height + 2};
            focusRing.rect(focusRect, cr);
            ctx.setFillStyle(FillStyle::none());
            ctx.setStrokeStyle(StrokeStyle::solid(th.focusRing, th.focusRingWidth));
            ctx.drawPath(focusRing);
        }

        EdgeInsets paddingVal = padding;
        if (paddingVal.horizontal() == 0.0f && paddingVal.vertical() == 0.0f) {
            paddingVal = th.buttonPadding;
        }

        Rect content = {bounds.x + paddingVal.left,
                        bounds.y + paddingVal.top,
                        bounds.width - paddingVal.horizontal(),
                        bounds.height - paddingVal.vertical()};

        float labelSize = static_cast<float>(fontSize) > 0.0f ? static_cast<float>(fontSize) : th.buttonFontSize;
        const std::string& font = th.uiFontFamily;

        ctx.setTextStyle(makeTextStyle(font, FontWeight::regular, labelSize, Typography::lineHeightTight,
            Typography::trackingFor(labelSize, FontWeight::regular)));
        ctx.setFillStyle(FillStyle::solid(resolveColor(textColor, th.buttonForeground)));
        ctx.drawText(static_cast<std::string>(text), content.center(), HorizontalAlignment::center,
            VerticalAlignment::center);
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        const Theme& th = static_cast<RenderContext&>(textMeasurer).theme();

        EdgeInsets paddingVal = padding;
        if (paddingVal.horizontal() == 0.0f && paddingVal.vertical() == 0.0f) {
            paddingVal = th.buttonPadding;
        }

        float labelSize = static_cast<float>(fontSize) > 0.0f ? static_cast<float>(fontSize) : th.buttonFontSize;
        const std::string& font = th.uiFontFamily;
        std::string buttonText = text;

        Size textSize = textMeasurer.measureText(buttonText,
            makeTextStyle(font, FontWeight::regular, labelSize, Typography::lineHeightTight,
                Typography::trackingFor(labelSize, FontWeight::regular)));
        return {textSize.width + paddingVal.horizontal(), textSize.height + paddingVal.vertical()};
    }

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
