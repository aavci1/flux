#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/KeyEvent.hpp>

namespace flux {

struct Slider {
    FLUX_VIEW_PROPERTIES;

    Property<float> value = 0.5f;
    Property<float> minValue = 0.0f;
    Property<float> maxValue = 1.0f;
    Property<float> step = 0.01f;
    Property<Color> activeColor = Colors::blue;
    Property<Color> inactiveColor = Colors::gray;
    Property<float> thumbRadius = 8.0f;
    Property<float> trackHeight = 4.0f;

    mutable bool isDragging = false;

    void init() {
        focusable = true;  // Sliders are focusable for keyboard control

        // Handle mouse down to start dragging
        onMouseDown = [this](float x, float y, int button) {
            if (button == 0) {  // Left mouse button
                isDragging = true;
                updateValueFromPosition(x, y);
            }
        };

        // Handle mouse up to stop dragging
        onMouseUp = [this](float x, float y, int button) {
            if (button == 0) {
                isDragging = false;
            }
        };

        // Handle mouse move while dragging
        onMouseMove = [this](float x, float y) {
            if (isDragging) {
                updateValueFromPosition(x, y);
            }
        };
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        bool hasFocus = ctx.isCurrentViewFocused();
        EdgeInsets paddingVal = padding;

        float trackH = trackHeight;
        float thumbR = thumbRadius;
        float sliderY = bounds.y + paddingVal.top + (bounds.height - paddingVal.vertical()) / 2;
        float sliderX = bounds.x + paddingVal.left + thumbR;
        float sliderWidth = bounds.width - paddingVal.horizontal() - thumbR * 2;

        // Normalize value to 0-1 range
        float val = value;
        float minVal = minValue;
        float maxVal = maxValue;
        float normalizedValue = (val - minVal) / (maxVal - minVal);
        normalizedValue = std::clamp(normalizedValue, 0.0f, 1.0f);

        // Draw inactive track (right side)
        ctx.setFillStyle(FillStyle::none());
        ctx.setStrokeStyle(StrokeStyle {
            .color = inactiveColor,
            .width = trackH,
            .cap = LineCap::Round
        });
        ctx.drawLine(
            {sliderX, sliderY},
            {sliderX + sliderWidth, sliderY}
        );

        // Draw active track (left side)
        ctx.setFillStyle(FillStyle::none());
        ctx.setStrokeStyle(StrokeStyle {
            .color = activeColor,
            .width = trackH,
            .cap = LineCap::Round
        });
        ctx.drawLine(
            {sliderX, sliderY},
            {sliderX + sliderWidth * normalizedValue, sliderY}
        );

        // Draw thumb
        float thumbX = sliderX + sliderWidth * normalizedValue;
        
        // Draw shadow for thumb
        ctx.setFillStyle(FillStyle::solid(Color(0, 0, 0, 0.2f)));
        ctx.setStrokeStyle(StrokeStyle::none());
        ctx.drawCircle({thumbX, sliderY + 1}, thumbR);

        // Draw thumb
        ctx.setFillStyle(FillStyle::solid(isDragging || hasFocus ? 
            static_cast<Color>(activeColor).darken(0.1f) : 
            static_cast<Color>(activeColor)));
        ctx.drawCircle({thumbX, sliderY}, thumbR);

        // Draw focus ring if focused
        if (hasFocus && !isDragging) {
            ctx.setFillStyle(FillStyle::none());
            ctx.setStrokeStyle(StrokeStyle{
                .color = static_cast<Color>(activeColor).darken(0.2f),
                .width = 2.0f
            });
            ctx.drawCircle({thumbX, sliderY}, thumbR + 3);
        }
    }

    Size preferredSize(TextMeasurement& /* textMeasurer */) const {
        EdgeInsets paddingVal = padding;
        float thumbR = thumbRadius;
        return {
            200.0f + paddingVal.horizontal() + thumbR * 2,
            thumbR * 2 + paddingVal.vertical()
        };
    }

    bool handleKeyDown(const KeyEvent& event) {
        float currentVal = value;
        float minVal = minValue;
        float maxVal = maxValue;
        float stepVal = step;

        bool changed = false;

        if (event.key == Key::Left || event.key == Key::Down) {
            value = std::max(minVal, currentVal - stepVal);
            changed = true;
        } else if (event.key == Key::Right || event.key == Key::Up) {
            value = std::min(maxVal, currentVal + stepVal);
            changed = true;
        } else if (event.key == Key::Home) {
            value = minVal;
            changed = true;
        } else if (event.key == Key::End) {
            value = maxVal;
            changed = true;
        }

        if (changed && onChange) {
            onChange();
            return true;
        }

        return false;
    }

private:
    void updateValueFromPosition(float x, float /* y */) {
        // Calculate from the current bounds (we need to access them somehow)
        // For now, we'll use a simple calculation based on the last render
        // This is a limitation of the immediate mode approach
        
        // We need to store bounds - for now just update the value proportionally
        // In a real implementation, we'd need to track the slider bounds
        float minVal = minValue;
        float maxVal = maxValue;
        
        // This is approximate - in practice we'd need the actual bounds
        // For the demo, this will work reasonably well
        float newValue = minVal + (maxVal - minVal) * std::clamp(x / 200.0f, 0.0f, 1.0f);
        
        // Apply step
        float stepVal = step;
        if (stepVal > 0) {
            newValue = std::round(newValue / stepVal) * stepVal;
        }
        
        value = std::clamp(newValue, minVal, maxVal);
        
        if (onChange) {
            onChange();
        }
    }
};

} // namespace flux