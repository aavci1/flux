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

// TODO: make size of the accessory calculated based on the font size of the label
struct RadioButtonAccessory {
    FLUX_VIEW_PROPERTIES;

    Property<bool> selected = false;

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        auto ux = bounds.width * 0.5f;
        auto uy = bounds.height * 0.5f;
        auto ur = (ux + uy) * 0.5f;
    
        if (!selected) {
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

    Size preferredSize(TextMeasurement& /* textMeasurer */) const {
        EdgeInsets paddingVal = padding;
        return {20.0f + paddingVal.horizontal(), 20.0f + paddingVal.vertical()};
    }
};

struct RadioButton {
    FLUX_VIEW_PROPERTIES;

    Property<bool> selected = false;
    Property<std::string> value = "";
    Property<std::string> label = "";
    Property<float> size = 20.0f;
    Property<Color> labelColor = Colors::black;
    Property<float> labelFontSize = 14.0f;
    Property<LabelPosition> labelPosition = LabelPosition::trailing;
    Property<JustifyContent> justifyContent = JustifyContent::start;
    Property<float> spacing = 8.0f;

    void init() {
        focusable = true;
        cursor = CursorType::Pointer;
        onClick = [this]() {
            if (!static_cast<bool>(selected) && onChange) {
                onChange();
            }
        };
    }

    View body() const {
        std::string labelText = label;
        
        // If no label, just render the radio button accessory
        if (labelText.empty()) {
            return RadioButtonAccessory {
                .selected = selected
            };
        }

        std::vector<View> children = {
            RadioButtonAccessory {
                .selected = selected
            },
            Text {
                .value = labelText,
                .fontSize = labelFontSize,
                .color = labelColor
            }
        };

        if (labelPosition == LabelPosition::leading) {
            std::reverse(children.begin(), children.end());
        }

        return HStack {
            .spacing = spacing,
            .justifyContent = justifyContent,
            .alignItems = AlignItems::center,
            .padding = padding,
            .children = children
        };
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        EdgeInsets paddingVal = padding;
        float radioSize = size;
        
        std::string labelText = label;
        if (labelText.empty()) {
            return {radioSize + paddingVal.horizontal(), radioSize + paddingVal.vertical()};
        }

        Size textSize = textMeasurer.measureText(labelText, TextStyle::regular("default", labelFontSize));
        float totalWidth = radioSize + static_cast<float>(spacing) + textSize.width + paddingVal.horizontal();
        float totalHeight = std::max(radioSize, textSize.height) + paddingVal.vertical();

        return {totalWidth, totalHeight};
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

