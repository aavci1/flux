#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <cmath>
#include <chrono>

namespace flux {

enum class ProgressBarMode {
    Determinate,    // Shows specific progress (0-100%)
    Indeterminate   // Shows loading animation
};

struct ProgressBar {
    FLUX_VIEW_PROPERTIES;

    Property<float> value = 0.0f;  // 0.0 to 1.0 for determinate mode
    Property<ProgressBarMode> mode = ProgressBarMode::Determinate;
    Property<float> height = 8.0f;
    Property<Color> fillColor = Colors::blue;
    Property<Color> trackColor = Colors::lightGray;
    Property<bool> showLabel = false;
    Property<float> labelFontSize = 12.0f;

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        EdgeInsets paddingVal = padding;
        float barHeight = height;
        float barY = bounds.y + paddingVal.top;
        
        // If showing label, reserve space for it
        bool showLbl = showLabel;
        float labelHeight = showLbl ? static_cast<float>(labelFontSize) + 4.0f : 0;
        if (showLbl) {
            barY += labelHeight;
        }

        float barWidth = bounds.width - paddingVal.horizontal();
        float barX = bounds.x + paddingVal.left;

        // Draw track (background)
        Rect trackRect = {barX, barY, barWidth, barHeight};
        ctx.setFillStyle(FillStyle::solid(trackColor));
        ctx.setStrokeStyle(StrokeStyle::none());
        ctx.drawRect(trackRect, CornerRadius(barHeight / 2));

        // Draw progress fill
        ProgressBarMode modeVal = mode;
        if (modeVal == ProgressBarMode::Determinate) {
            float progress = std::clamp(static_cast<float>(value), 0.0f, 1.0f);
            float fillWidth = barWidth * progress;
            
            if (fillWidth > 0) {
                Rect fillRect = {barX, barY, fillWidth, barHeight};
                ctx.setFillStyle(FillStyle::solid(fillColor));
                ctx.drawRect(fillRect, CornerRadius(barHeight / 2));
            }

            // Draw percentage label if enabled
            if (showLbl) {
                int percentage = static_cast<int>(progress * 100);
                std::string labelText = std::to_string(percentage) + "%";
                
                ctx.setTextStyle(TextStyle::regular("default", labelFontSize));
                ctx.setFillStyle(FillStyle::solid(Colors::black));
                
                Size textSize = ctx.measureText(labelText, TextStyle::regular("default", labelFontSize));
                float labelX = barX + barWidth / 2 - textSize.width / 2;
                float labelY = bounds.y + paddingVal.top + textSize.height;
                
                ctx.drawText(labelText, {labelX, labelY}, HorizontalAlignment::leading, VerticalAlignment::bottom);
            }
        } else {
            // Indeterminate mode - animated sliding bar
            // Use time-based animation
            auto now = std::chrono::steady_clock::now();
            auto duration = now.time_since_epoch();
            float time = std::chrono::duration<float>(duration).count();
            
            // Create sliding effect
            float cycleTime = 2.0f;  // 2 second cycle
            float phase = std::fmod(time, cycleTime) / cycleTime;
            
            float indeterminateWidth = barWidth * 0.3f;
            float indeterminateX = barX + (barWidth - indeterminateWidth) * phase;
            
            Rect fillRect = {indeterminateX, barY, indeterminateWidth, barHeight};
            ctx.setFillStyle(FillStyle::solid(fillColor));
            ctx.drawRect(fillRect, CornerRadius(barHeight / 2));
        }
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        EdgeInsets paddingVal = padding;
        float h = static_cast<float>(height) + paddingVal.vertical();
        
        if (showLabel) {
            h += static_cast<float>(labelFontSize) + 4.0f;
        }
        
        return {200.0f + paddingVal.horizontal(), h};
    }
};

} // namespace flux

