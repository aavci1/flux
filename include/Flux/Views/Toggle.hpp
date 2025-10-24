#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/KeyEvent.hpp>

namespace flux {

struct Toggle {
    FLUX_VIEW_PROPERTIES;

    Property<bool> isOn = false;
    Property<float> width = 50.0f;
    Property<float> height = 26.0f;
    Property<Color> onColor = Colors::green;
    Property<Color> offColor = Colors::gray;

    void init() {
        focusable = true;  // Toggles are focusable
        
        // Handle click to toggle
        onClick = [this]() {
            isOn = !static_cast<bool>(isOn);
            if (onChange) {
                onChange();
            }
        };
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        bool hasFocus = ctx.isCurrentViewFocused();
        bool on = isOn;
        EdgeInsets paddingVal = padding;

        float toggleWidth = width;
        float toggleHeight = height;
        float toggleX = bounds.x + paddingVal.left;
        float toggleY = bounds.y + paddingVal.top + (bounds.height - paddingVal.vertical() - toggleHeight) / 2;

        // Draw track (background)
        Rect trackRect = {toggleX, toggleY, toggleWidth, toggleHeight};
        Color trackColor = on ? static_cast<Color>(onColor) : static_cast<Color>(offColor);
        
        ctx.setFillStyle(FillStyle::solid(trackColor));
        ctx.setStrokeStyle(StrokeStyle::none());
        ctx.drawRect(trackRect, CornerRadius(toggleHeight / 2));

        // Draw focus ring if focused
        if (hasFocus) {
            ctx.setFillStyle(FillStyle::none());
            ctx.setStrokeStyle(StrokeStyle{
                .color = trackColor.darken(0.2f),
                .width = 2.0f
            });
            Rect focusRect = {toggleX - 2, toggleY - 2, toggleWidth + 4, toggleHeight + 4};
            ctx.drawRect(focusRect, CornerRadius((toggleHeight + 4) / 2));
        }

        // Draw thumb (circle)
        float thumbRadius = (toggleHeight - 4) / 2;
        float thumbX = on ? 
            (toggleX + toggleWidth - thumbRadius - 2) : 
            (toggleX + thumbRadius + 2);
        float thumbY = toggleY + toggleHeight / 2;

        ctx.setFillStyle(FillStyle::solid(Colors::white));
        ctx.setStrokeStyle(StrokeStyle::none());
        ctx.drawCircle({thumbX, thumbY}, thumbRadius);

        // Add subtle shadow to thumb
        ctx.setFillStyle(FillStyle::solid(Color(0, 0, 0, 0.2f)));
        ctx.drawCircle({thumbX, thumbY + 1}, thumbRadius);
        ctx.setFillStyle(FillStyle::solid(Colors::white));
        ctx.drawCircle({thumbX, thumbY}, thumbRadius);
    }

    Size preferredSize(TextMeasurement& /* textMeasurer */) const {
        EdgeInsets paddingVal = padding;
        return {
            static_cast<float>(width) + paddingVal.horizontal(),
            static_cast<float>(height) + paddingVal.vertical()
        };
    }

    bool handleKeyDown(const KeyEvent& event) {
        if (event.key == Key::Space || event.key == Key::Enter) {
            isOn = !static_cast<bool>(isOn);
            if (onChange) {
                onChange();
            }
            return true;
        }
        return false;
    }
};

} // namespace flux

