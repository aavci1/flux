#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Views/HStack.hpp>
#include <Flux/Views/Text.hpp>
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

// CheckboxAccessory - Just the checkbox box part
struct CheckboxAccessory {
    FLUX_VIEW_PROPERTIES;

    Property<bool> checked = false;
    Property<float> size = 20.0f;

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        bool isChecked = checked;
        float boxSize = size;
        EdgeInsets paddingVal = padding;

        // Calculate checkbox position
        float checkboxX = bounds.x + paddingVal.left;
        float checkboxY = bounds.y + paddingVal.top + (bounds.height - paddingVal.vertical() - boxSize) / 2;

        // Draw checkbox box
        Rect checkboxRect = {checkboxX, checkboxY, boxSize, boxSize};
        drawCheckbox(ctx, checkboxRect, isChecked);
    }

    Size preferredSize(TextMeasurement& /* textMeasurer */) const {
        EdgeInsets paddingVal = padding;
        float boxSize = size;
        return {boxSize + paddingVal.horizontal(), boxSize + paddingVal.vertical()};
    }
};

struct Checkbox {
    FLUX_VIEW_PROPERTIES;

    Property<bool> checked = false;
    Property<std::string> label = "";
    Property<float> size = 20.0f;
    Property<Color> labelColor = Colors::black;
    Property<float> labelFontSize = 14.0f;
    Property<LabelPosition> labelPosition = LabelPosition::trailing;
    Property<JustifyContent> justifyContent = JustifyContent::start;
    Property<float> spacing = 8.0f;

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

    View body() const {
        std::string labelText = label;
        
        // If no label, just render the checkbox accessory
        if (labelText.empty()) {
            return View(CheckboxAccessory {
                .checked = checked,
                .size = size
            });
        }
        
        // Create label
        Text labelView {
            .value = labelText,
            .fontSize = labelFontSize,
            .color = labelColor,
            .verticalAlignment = VerticalAlignment::center,
            .horizontalAlignment = HorizontalAlignment::leading
        };
        
        // Create accessory
        CheckboxAccessory accessory {
            .checked = checked,
            .size = size
        };
        
        // Create HStack with appropriate order
        LabelPosition pos = labelPosition;
        return View(HStack {
            .spacing = spacing,
            .justifyContent = justifyContent,
            .alignItems = AlignItems::center,
            .padding = padding,
            .children = pos == LabelPosition::leading 
                ? std::vector<View>{View(labelView), View(accessory)}
                : std::vector<View>{View(accessory), View(labelView)}
        });
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        EdgeInsets paddingVal = padding;
        float boxSize = size;
        
        std::string labelText = label;
        if (labelText.empty()) {
            return {boxSize + paddingVal.horizontal(), boxSize + paddingVal.vertical()};
        }

        Size textSize = textMeasurer.measureText(labelText, TextStyle::regular("default", labelFontSize));
        float totalWidth = boxSize + static_cast<float>(spacing) + textSize.width + paddingVal.horizontal();
        float totalHeight = std::max(boxSize, textSize.height) + paddingVal.vertical();

        return {totalWidth, totalHeight};
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

