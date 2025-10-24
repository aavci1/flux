#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <string>

namespace flux {

struct Checkbox {
    FLUX_VIEW_PROPERTIES;

    Property<bool> checked = false;
    Property<std::string> label = "";
    Property<float> size = 20.0f;
    Property<Color> checkColor = Colors::blue;
    Property<Color> labelColor = Colors::black;
    Property<float> labelFontSize = 14.0f;

    void init() {
        cursor = CursorType::Pointer;
        focusable = true;
        onClick = [this]() {
            checked = !static_cast<bool>(checked);
            if (onChange) {
                onChange();
            }
        };
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        bool hasFocus = ctx.isCurrentViewFocused();
        bool isChecked = checked;
        float boxSize = size;
        EdgeInsets paddingVal = padding;

        // Calculate checkbox position
        float checkboxX = bounds.x + paddingVal.left;
        float checkboxY = bounds.y + paddingVal.top + (bounds.height - paddingVal.vertical() - boxSize) / 2;

        // Draw checkbox box
        Rect checkboxRect = {checkboxX, checkboxY, boxSize, boxSize};
        
        // Draw background - fill with color when checked for better visibility
        if (isChecked) {
            ctx.setFillStyle(FillStyle::solid(checkColor));
            ctx.setStrokeStyle(StrokeStyle::none());
            ctx.drawRect(checkboxRect, CornerRadius(3.0f));
        } else {
            ctx.setFillStyle(FillStyle::solid(Colors::white));
            ctx.setStrokeStyle(StrokeStyle::none());
            ctx.drawRect(checkboxRect, CornerRadius(3.0f));
            
            // Draw border
            Color borderColor = hasFocus ? static_cast<Color>(checkColor).darken(0.2f) : Colors::gray;
            ctx.setFillStyle(FillStyle::none());
            ctx.setStrokeStyle(StrokeStyle{
                .color = borderColor,
                .width = hasFocus ? 2.5f : 2.0f
            });
            ctx.drawRect(checkboxRect, CornerRadius(3.0f));
        }

        // Draw checkmark if checked (white on colored background)
        if (isChecked) {
            ctx.setFillStyle(FillStyle::none());
            ctx.setStrokeStyle(StrokeStyle{
                .color = Colors::white,  // White checkmark on colored background
                .width = 2.5f,
                .cap = LineCap::Round,
                .join = LineJoin::Round
            });

            // Draw checkmark path
            float centerX = checkboxX + boxSize / 2;
            float centerY = checkboxY + boxSize / 2;
            float scale = boxSize / 20.0f;

            Point p1 = {centerX - 4 * scale, centerY};
            Point p2 = {centerX - 1 * scale, centerY + 3 * scale};
            Point p3 = {centerX + 5 * scale, centerY - 4 * scale};

            ctx.drawLine(p1, p2);
            ctx.drawLine(p2, p3);
        }

        // Draw label if provided
        std::string labelText = label;
        if (!labelText.empty()) {
            float labelX = checkboxX + boxSize + 8.0f;
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
        float boxSize = size;
        float width = boxSize + paddingVal.horizontal();
        float height = boxSize + paddingVal.vertical();

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
            // Toggle checkbox
            checked = !static_cast<bool>(checked);
            if (onChange) {
                onChange();
            }
            return true;
        }
        return false;
    }
};

} // namespace flux

