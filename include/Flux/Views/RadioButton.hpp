#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <string>
#include <memory>
#include <vector>
#include <map>

namespace flux {

// Radio group manager to handle mutual exclusion
class RadioGroup {
public:
    static RadioGroup& instance() {
        static RadioGroup instance;
        return instance;
    }

    void selectInGroup(const std::string& groupName, void* radioButton) {
        if (groupName.empty()) return;
        groups_[groupName] = radioButton;
    }

    bool isSelected(const std::string& groupName, void* radioButton) const {
        if (groupName.empty()) return false;
        auto it = groups_.find(groupName);
        return it != groups_.end() && it->second == radioButton;
    }

private:
    std::map<std::string, void*> groups_;
    RadioGroup() = default;
};

struct RadioButton {
    FLUX_VIEW_PROPERTIES;

    Property<bool> selected = false;
    Property<std::string> value = "";
    Property<std::string> group = "";
    Property<std::string> label = "";
    Property<float> size = 20.0f;
    Property<Color> selectedColor = Colors::blue;
    Property<Color> labelColor = Colors::black;
    Property<float> labelFontSize = 14.0f;

    void init() {
        focusable = true;  // Radio buttons are focusable
        
        // Handle click to select
        onClick = [this]() {
            if (!static_cast<bool>(selected)) {
                std::string groupName = group;
                if (!groupName.empty()) {
                    RadioGroup::instance().selectInGroup(groupName, (void*)this);
                }
                selected = true;
                if (onChange) {
                    onChange();
                }
            }
        };
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        bool hasFocus = ctx.isCurrentViewFocused();
        
        // Check if this radio button is selected in its group
        bool isSelected = selected;
        std::string groupName = group;
        if (!groupName.empty()) {
            isSelected = RadioGroup::instance().isSelected(groupName, (void*)this);
        }

        float radioSize = size;
        EdgeInsets paddingVal = padding;

        // Calculate radio button position
        float radioX = bounds.x + paddingVal.left + radioSize / 2;
        float radioY = bounds.y + paddingVal.top + (bounds.height - paddingVal.vertical()) / 2;

        // Draw outer circle
        Color borderColor = hasFocus ? static_cast<Color>(selectedColor).darken(0.2f) : Colors::gray;
        
        if (isSelected) {
            // Fill with color when selected
            ctx.setFillStyle(FillStyle::solid(selectedColor));
            ctx.setStrokeStyle(StrokeStyle::none());
            ctx.drawCircle({radioX, radioY}, radioSize / 2);
            
            // Draw white inner circle for contrast
            ctx.setFillStyle(FillStyle::solid(Colors::white));
            ctx.drawCircle({radioX, radioY}, radioSize / 2 - 4);
            
            // Draw colored center dot
            ctx.setFillStyle(FillStyle::solid(selectedColor));
            ctx.drawCircle({radioX, radioY}, radioSize / 2 - 7);
        } else {
            // Unfilled state with border
            ctx.setFillStyle(FillStyle::solid(Colors::white));
            ctx.setStrokeStyle(StrokeStyle::none());
            ctx.drawCircle({radioX, radioY}, radioSize / 2);

            ctx.setFillStyle(FillStyle::none());
            ctx.setStrokeStyle(StrokeStyle{
                .color = borderColor,
                .width = hasFocus ? 2.5f : 2.0f
            });
            ctx.drawCircle({radioX, radioY}, radioSize / 2);
        }

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
            if (!static_cast<bool>(selected)) {
                std::string groupName = group;
                if (!groupName.empty()) {
                    RadioGroup::instance().selectInGroup(groupName, (void*)this);
                }
                selected = true;
                if (onChange) {
                    onChange();
                }
            }
            return true;
        }
        return false;
    }
};

} // namespace flux

