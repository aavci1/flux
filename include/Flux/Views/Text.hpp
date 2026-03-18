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
    Property<float> fontSize = 13;
    Property<FontWeight> fontWeight = FontWeight::regular;
    Property<Color> color = Colors::black;
    Property<HorizontalAlignment> horizontalAlignment = HorizontalAlignment::center;
    Property<VerticalAlignment> verticalAlignment = VerticalAlignment::center;

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        EdgeInsets paddingVal = padding;
        float fontSz = fontSize;
        std::string text = value;
        HorizontalAlignment hAlign = horizontalAlignment;

        ctx.setTextStyle(TextStyle::regular("default", fontSz));
        ctx.setFillStyle(FillStyle::solid(color));

        float contentWidth = bounds.width - paddingVal.horizontal();

        Size singleLineSize = ctx.measureText(text, TextStyle::regular("default", fontSz));
        bool needsWrap = singleLineSize.width > contentWidth && contentWidth > 0;

        if (needsWrap) {
            Point textPos = { bounds.x + paddingVal.left, bounds.y + paddingVal.top };
            ctx.drawTextBox(text, textPos, contentWidth, hAlign);
        } else {
            Point textPos = { bounds.x + paddingVal.left, bounds.y + paddingVal.top };

            switch (hAlign) {
                case HorizontalAlignment::leading:
                    textPos.x = bounds.x + paddingVal.left;
                    break;
                case HorizontalAlignment::center:
                    textPos.x = bounds.x + bounds.width / 2;
                    break;
                case HorizontalAlignment::trailing:
                    textPos.x = bounds.x + bounds.width - paddingVal.right;
                    break;
                case HorizontalAlignment::justify:
                    textPos.x = bounds.x + paddingVal.left;
                    break;
            }

            VerticalAlignment vAlign = verticalAlignment;
            switch (vAlign) {
                case VerticalAlignment::top:
                    textPos.y = bounds.y + paddingVal.top;
                    break;
                case VerticalAlignment::center:
                    textPos.y = bounds.y + bounds.height / 2 + 0.075f * fontSz;
                    break;
                case VerticalAlignment::bottom:
                    textPos.y = bounds.y + bounds.height - paddingVal.bottom;
                    break;
            }

            ctx.drawText(text, textPos, hAlign, vAlign);
        }
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        std::string text = value;
        float fontSz = fontSize;
        EdgeInsets paddingVal = padding;

        Size textSize = textMeasurer.measureText(text, TextStyle::regular("default", fontSz));
        return {textSize.width + paddingVal.horizontal(),
                textSize.height + paddingVal.vertical()};
    }

    float heightForWidth(float width, TextMeasurement& textMeasurer) const {
        std::string text = value;
        float fontSz = fontSize;
        EdgeInsets paddingVal = padding;

        float contentWidth = width - paddingVal.horizontal();
        if (contentWidth <= 0) {
            return paddingVal.vertical();
        }

        Size singleLine = textMeasurer.measureText(text, TextStyle::regular("default", fontSz));
        if (singleLine.width <= contentWidth) {
            return singleLine.height + paddingVal.vertical();
        }

        Size boxSize = textMeasurer.measureTextBox(text, TextStyle::regular("default", fontSz), contentWidth);
        return boxSize.height + paddingVal.vertical();
    }
};

} // namespace flux
