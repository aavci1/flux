#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/Typography.hpp>
#include <string>

namespace flux {

struct Badge {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> text = "";
    Property<Color> badgeColor = Colors::red;
    Property<Color> textColor = Colors::white;
    Property<float> fontSize = Typography::caption;
    Property<float> paddingHorizontal = 8.0f;
    Property<float> paddingVertical = 4.0f;

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        std::string content = text;
        if (content.empty()) {
            return;
        }

        EdgeInsets paddingVal = padding;
        float padH = paddingHorizontal;
        float padV = paddingVertical;

        float fs = fontSize;
        TextStyle style = makeTextStyle("default", FontWeight::bold, fs, Typography::lineHeightTight,
            Typography::trackingCaption(fs));
        ctx.setTextStyle(style);
        Size textSize = ctx.measureText(content, style);

        // Calculate badge dimensions
        float badgeWidth = textSize.width + padH * 2;
        float badgeHeight = textSize.height + padV * 2;
        float badgeX = bounds.x + paddingVal.left + (bounds.width - paddingVal.horizontal() - badgeWidth) / 2;
        float badgeY = bounds.y + paddingVal.top + (bounds.height - paddingVal.vertical() - badgeHeight) / 2;

        // Draw badge background
        Rect badgeRect = {badgeX, badgeY, badgeWidth, badgeHeight};
        ctx.setFillStyle(FillStyle::solid(badgeColor));
        ctx.setStrokeStyle(StrokeStyle::none());
        ctx.drawRect(badgeRect, CornerRadius(badgeHeight / 2));

        // Draw text
        float textX = badgeX + padH;
        float textY = badgeY + padV + textSize.height;

        ctx.setTextStyle(style);
        ctx.setFillStyle(FillStyle::solid(textColor));
        ctx.drawText(content, {textX, textY}, HorizontalAlignment::leading, VerticalAlignment::bottom);
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        EdgeInsets paddingVal = padding;
        std::string content = text;
        
        if (content.empty()) {
            return {paddingVal.horizontal(), paddingVal.vertical()};
        }

        float padH = paddingHorizontal;
        float padV = paddingVertical;

        float fs = fontSize;
        Size textSize = textMeasurer.measureText(content,
            makeTextStyle("default", FontWeight::bold, fs, Typography::lineHeightTight,
                Typography::trackingCaption(fs)));
        
        return {
            textSize.width + padH * 2 + paddingVal.horizontal(),
            textSize.height + padV * 2 + paddingVal.vertical()
        };
    }
};

} // namespace flux

