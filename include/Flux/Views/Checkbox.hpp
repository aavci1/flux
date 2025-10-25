#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <string>

namespace flux {

// size 20px
// font 14px
void drawCheckbox(RenderContext& ctx, const Rect& bounds, bool isChecked) {
    auto ux = bounds.width * 0.05f;
    auto uy = bounds.height * 0.05f;
    auto ur = (ux + uy) * 0.5f;

    if (!isChecked) {
        ctx.setFillStyle(FillStyle::solid(Colors::white));
        ctx.setStrokeStyle(StrokeStyle::solid(Colors::lightGray, ur * 2));
        ctx.drawRect(bounds, CornerRadius(ur * 4.0f));
    }
    else {
        ctx.setFillStyle(FillStyle::solid(Colors::blue));
        ctx.setStrokeStyle(StrokeStyle::none());
        ctx.drawRect(bounds, CornerRadius(ur * 4.0f));

        // Draw checkmark path
        auto center = bounds.center();
        auto size = bounds.size();
        Path path;
        // path.moveTo({bounds.x + 5, bounds.y + bounds.height * 0.5f});
        // path.lineTo({bounds.x + 9, bounds.y + bounds.height * 0.5f + 4});
        // path.lineTo({bounds.x + 15, bounds.y + bounds.height * 0.5f - 4});
        path.moveTo({center.x - 5 * ux, center.y});
        path.lineTo({center.x - ux, center.y + 4 * uy});
        path.lineTo({center.x + 5 * ux, center.y - 4 * uy});

        ctx.setFillStyle(FillStyle::none());
        ctx.setStrokeStyle(StrokeStyle{
            .color = Colors::white,
            .width = ur * 2.0f,
            .cap = LineCap::Round,
            .join = LineJoin::Round
        });

        ctx.drawPath(path);
    }
}

struct Checkbox {
    FLUX_VIEW_PROPERTIES;

    Property<bool> checked = false;
    Property<std::string> label = "";
    Property<float> size = 20.0f;
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
        drawCheckbox(ctx, checkboxRect, isChecked);

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

