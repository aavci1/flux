#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <string>
#include <variant>

namespace flux {

struct Text {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> value = "";
    Property<float> fontSize = 16;
    Property<FontWeight> fontWeight = FontWeight::regular;
    Property<Color> color = Colors::black;
    Property<HorizontalAlignment> horizontalAlignment = HorizontalAlignment::center;
    Property<VerticalAlignment> verticalAlignment = VerticalAlignment::center;

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        // Draw text
        EdgeInsets paddingVal = padding;

        // Measure text for alignment calculations
        Size textSize = ctx.measureText(static_cast<std::string>(value),
                                       static_cast<float>(fontSize),
                                       static_cast<FontWeight>(fontWeight));

        Point textPos = {
            bounds.x + paddingVal.left,
            bounds.y + paddingVal.top
        };

        // Adjust horizontal position based on horizontal alignment
        HorizontalAlignment hAlign = horizontalAlignment;
        switch (hAlign) {
            case HorizontalAlignment::leading:
                // Position at left edge
                textPos.x = bounds.x + paddingVal.left;
                break;
            case HorizontalAlignment::center:
                // Position at center
                textPos.x = bounds.x + bounds.width / 2;
                break;
            case HorizontalAlignment::trailing:
                // Position at right edge
                textPos.x = bounds.x + bounds.width - paddingVal.right;
                break;
            case HorizontalAlignment::justify:
                // For now, treat justify same as leading (full justification not yet implemented)
                textPos.x = bounds.x + paddingVal.left;
                break;
        }

        // Adjust vertical position based on vertical alignment
        // With the alignment system, we pass the exact position where we want the text aligned
        VerticalAlignment vAlign = verticalAlignment;
        switch (vAlign) {
            case VerticalAlignment::top:
                textPos.y = bounds.y + paddingVal.top;
                break;
            case VerticalAlignment::center:
                textPos.y = bounds.y + bounds.height / 2;
                break;
            case VerticalAlignment::bottom:
                textPos.y = bounds.y + bounds.height - paddingVal.bottom;
                break;
        }

        // Draw the text using the new method that supports both horizontal and vertical alignment
        Color textColor = color;
        ctx.drawText(
            static_cast<std::string>(value),
            textPos,
            static_cast<float>(fontSize),
            textColor,
            static_cast<FontWeight>(fontWeight),
            hAlign,
            vAlign
        );
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        std::string text = value;
        float fontSz = fontSize;
        EdgeInsets paddingVal = padding;

        // Use accurate measurement from renderer
        Size textSize = textMeasurer.measureText(text, fontSz, static_cast<FontWeight>(fontWeight));
        return {textSize.width + paddingVal.horizontal(),
                textSize.height + paddingVal.vertical()};
    }
};

} // namespace flux
