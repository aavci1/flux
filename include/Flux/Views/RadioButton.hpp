#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Views/LabeledControl.hpp>
#include <Flux/Core/Typography.hpp>
#include <string>

namespace flux {

// TODO: make size of the accessory calculated based on the font size of the label
struct RadioButtonAccessory {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;

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
    FLUX_INTERACTIVE_PROPERTIES;

    Property<bool> selected = false;
    Property<std::string> value = "";
    Property<std::string> label = "";
    Property<float> size = 20.0f;
    Property<Color> labelColor = Colors::black;
    Property<float> labelFontSize = Typography::callout;
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
        return LabeledControl::build(
            View(RadioButtonAccessory{.selected = selected}),
            label, labelPosition, justifyContent,
            spacing, padding, labelFontSize, labelColor);
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        float radioSize = size;
        return LabeledControl::measure(
            radioSize, radioSize, label, labelFontSize, spacing, padding, textMeasurer);
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

