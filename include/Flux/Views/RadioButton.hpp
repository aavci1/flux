#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <string>

namespace flux {

void drawRadioButton(RenderContext& ctx, const Rect& bounds, bool isSelected) {
    auto ux = bounds.width * 0.5f;
    auto uy = bounds.height * 0.5f;
    auto ur = (ux + uy) * 0.5f;

    if (!isSelected) {
        ctx.setFillStyle(FillStyle::solid(Colors::white));
        ctx.setStrokeStyle(StrokeStyle::solid(Colors::lightGray, ur * 0.2f));
        ctx.drawCircle(bounds.center(), ur);
    }
    else {
        ctx.setFillStyle(FillStyle::solid(Colors::blue));
        ctx.setStrokeStyle(StrokeStyle::none());
        ctx.drawCircle(bounds.center(), ur);

        ctx.setFillStyle(FillStyle::solid(Colors::white));
        ctx.setStrokeStyle(StrokeStyle::none());
        ctx.drawCircle(bounds.center(), ur * 0.4f);
    }
}

struct RadioButton {
    FLUX_VIEW_PROPERTIES;

    Property<bool> selected = false;
    Property<std::string> value = "";  // The value this radio represents (optional, for user reference)
    Property<std::string> label = "";
    Property<float> size = 20.0f;
    Property<Color> labelColor = Colors::black;
    Property<float> labelFontSize = 14.0f;

    void init() {
        focusable = true;  // Radio buttons are focusable
        
        // Handle click to select
        // User should set onChange to update their shared Property
        onClick = [this]() {
            if (!static_cast<bool>(selected) && onChange) {
                onChange();
            }
        };
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        bool hasFocus = ctx.isCurrentViewFocused();
        bool isSelected = selected;  // Simply use the selected property
        float radioSize = size;
        EdgeInsets paddingVal = padding;

        // Calculate radio button position
        float radioX = bounds.x + paddingVal.left;
        float radioY = bounds.y + paddingVal.top;

        // Draw radio button
        Rect radioRect = {radioX, radioY, radioSize, radioSize};
        drawRadioButton(ctx, radioRect, isSelected);

        // Draw label if provided
        std::string labelText = label;
        if (!labelText.empty()) {
            float labelX = bounds.x + paddingVal.left + radioSize + 8.0f;
            float labelY = bounds.y + paddingVal.top;
            float labelHeight = bounds.height - paddingVal.vertical();

            ctx.setTextStyle(TextStyle::regular("default", labelFontSize));
            ctx.setFillStyle(FillStyle::solid(labelColor));

            Size textSize = ctx.measureText(labelText, TextStyle::regular("default", labelFontSize));
            float textY = labelY + (labelHeight - textSize.height) / 2 + textSize.height;

            ctx.drawText(labelText, {labelX, textY}, HorizontalAlignment::leading, VerticalAlignment::bottom);
        }
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        EdgeInsets paddingVal = padding;
        float radioSize = size;
        float width = radioSize + paddingVal.horizontal();
        float height = radioSize + paddingVal.vertical();

        std::string labelText = label;
        if (!labelText.empty()) {
            Size textSize = textMeasurer.measureText(labelText, TextStyle::regular("default", labelFontSize));
            width += 8.0f + textSize.width;
            height = std::max(height, textSize.height + paddingVal.vertical());
        }

        return {width, height};
    }

    bool handleKeyDown(const KeyEvent& event) {
        if (event.key == Key::Space || event.key == Key::Enter) {
            if (!static_cast<bool>(selected) && onChange) {
                onChange();
            }
            return true;
        }
        return false;
    }
};

} // namespace flux

