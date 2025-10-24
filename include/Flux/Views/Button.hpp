#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <functional>
#include <string>

namespace flux {

struct Button {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> text;
    
    // Focus state is now tracked by Window, no need for local state
    
    // Initialize default properties
    void init() {
        backgroundColor = Colors::blue;
        focusable = true;  // Buttons are focusable by default
        
        // Set up focus callbacks to track focus state
        onFocus = [this]() {
            // Use a safer approach - don't modify member variables in callbacks
            // The focus state will be tracked by the Window
        };
        onBlur = [this]() {
            // Use a safer approach - don't modify member variables in callbacks
            // The focus state will be tracked by the Window
        };
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        // Check if this button has focus
        bool hasFocus = ctx.isCurrentViewFocused();
        
        // Render background with focus indicator
        if (hasFocus) {
            // Draw a lighter background when focused (add 0.2 to RGB values)
            Color bgColor = static_cast<Color>(backgroundColor);
            Color focusColor(
                std::min(bgColor.r + 0.2f, 1.0f),
                std::min(bgColor.g + 0.2f, 1.0f),
                std::min(bgColor.b + 0.2f, 1.0f),
                bgColor.a
            );
            ctx.setFillStyle(FillStyle::solid(focusColor));
            ctx.drawRect(bounds, cornerRadius);
            
            // Draw focus ring
            Path focusRing;
            focusRing.rect(bounds, cornerRadius);
            ctx.setStrokeStyle(StrokeStyle::solid(Colors::white, 3.0f));
            ctx.drawPath(focusRing);
        } else {
            // Normal rendering
            ViewHelpers::renderView(*this, ctx, bounds);
        }

        // Render button text
        ctx.setFillStyle(FillStyle::solid(Colors::white));
        ctx.drawText(static_cast<std::string>(text), bounds.center(), HorizontalAlignment::center, VerticalAlignment::center);
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        EdgeInsets paddingVal = padding;
        if (paddingVal.horizontal() == 0) {
            paddingVal = EdgeInsets(12, 24); // Default button padding
        }

        std::string buttonText = text;

        // Use accurate measurement from renderer
        Size textSize = textMeasurer.measureText(buttonText, TextStyle::regular("default", 16));
        return {textSize.width + paddingVal.horizontal(),
                textSize.height + paddingVal.vertical()};
    }
    
    // Handle keyboard activation (Enter/Space)
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
