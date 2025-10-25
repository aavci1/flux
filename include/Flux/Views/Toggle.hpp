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

void drawToggle(RenderContext& ctx, const Rect& bounds, bool isOn) {
    ctx.setFillStyle(FillStyle::solid(isOn ? Colors::blue : Colors::gray));
    ctx.setStrokeStyle(StrokeStyle::none());
    ctx.drawRect(bounds, CornerRadius(bounds.height / 2));

    // Draw thumb (circle)
    float thumbRadius = (bounds.height - 4) / 2;
    float thumbX = isOn ? 
        (bounds.x + bounds.width - thumbRadius - 2) : 
        (bounds.x + thumbRadius + 2);
    float thumbY = bounds.y + bounds.height / 2;

    ctx.setFillStyle(FillStyle::solid(Colors::white));
    ctx.setStrokeStyle(StrokeStyle::none());
    ctx.drawCircle({thumbX, thumbY}, thumbRadius);

    // Add subtle shadow to thumb
    ctx.setFillStyle(FillStyle::solid(Color(0, 0, 0, 0.2f)));
    ctx.drawCircle({thumbX, thumbY + 1}, thumbRadius);
    ctx.setFillStyle(FillStyle::solid(Colors::white));
    ctx.drawCircle({thumbX, thumbY}, thumbRadius);
}

// ToggleAccessory - Just the switch part
struct ToggleAccessory {
    FLUX_VIEW_PROPERTIES;

    Property<bool> isOn = false;
    Property<float> width = 36.0f;
    Property<float> height = 20.0f;
    Property<Color> onColor = Colors::green;
    Property<Color> offColor = Colors::gray;

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        bool on = isOn;
        EdgeInsets paddingVal = padding;

        float toggleWidth = width;
        float toggleHeight = height;
        float toggleX = bounds.x + paddingVal.left;
        float toggleY = bounds.y + paddingVal.top + (bounds.height - paddingVal.vertical() - toggleHeight) / 2;

        // Draw track (background)
        Rect trackRect = {toggleX, toggleY, toggleWidth, toggleHeight};
        drawToggle(ctx, trackRect, on);
    }

    Size preferredSize(TextMeasurement& /* textMeasurer */) const {
        EdgeInsets paddingVal = padding;
        return {
            static_cast<float>(width) + paddingVal.horizontal(),
            static_cast<float>(height) + paddingVal.vertical()
        };
    }
};

struct Toggle {
    FLUX_VIEW_PROPERTIES;

    Property<bool> isOn = false;
    Property<std::string> label = "";
    Property<float> width = 36.0f;
    Property<float> height = 20.0f;
    Property<Color> onColor = Colors::green;
    Property<Color> offColor = Colors::gray;
    Property<Color> labelColor = Colors::black;
    Property<float> labelFontSize = 14.0f;
    Property<LabelPosition> labelPosition = LabelPosition::trailing;
    Property<JustifyContent> justifyContent = JustifyContent::start;
    Property<float> spacing = 8.0f;

    void init() {
        focusable = true;  // Toggles are focusable
        cursor = CursorType::Pointer;
        
        // Handle click to toggle
        onClick = [this]() {
            isOn = !static_cast<bool>(isOn);
            if (onChange) {
                onChange();
            }
        };
    }

    View body() const {
        std::string labelText = label;
        
        // If no label, just render the toggle accessory
        if (labelText.empty()) {
            return View(ToggleAccessory {
                .isOn = isOn,
                .width = width,
                .height = height,
                .onColor = onColor,
                .offColor = offColor
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
        ToggleAccessory accessory {
            .isOn = isOn,
            .width = width,
            .height = height,
            .onColor = onColor,
            .offColor = offColor
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
        std::string labelText = label;
        EdgeInsets paddingVal = padding;
        
        // If no label, just return toggle size
        if (labelText.empty()) {
            return {
                static_cast<float>(width) + paddingVal.horizontal(),
                static_cast<float>(height) + paddingVal.vertical()
            };
        }
        
        // Calculate combined size
        float toggleWidth = width;
        float toggleHeight = height;
        
        Size textSize = textMeasurer.measureText(labelText, TextStyle::regular("default", labelFontSize));
        float totalWidth = toggleWidth + static_cast<float>(spacing) + textSize.width + paddingVal.horizontal();
        float totalHeight = std::max(toggleHeight, textSize.height) + paddingVal.vertical();
        
        return {totalWidth, totalHeight};
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

