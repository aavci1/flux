#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/Typography.hpp>
#include <string>
#include <variant>

namespace flux {

struct Text {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> value = "";
    /** macOS body default (~17pt). */
    Property<float> fontSize = Typography::body;
    Property<FontWeight> fontWeight = FontWeight::regular;
    Property<Color> color = Colors::black;
    Property<HorizontalAlignment> horizontalAlignment = HorizontalAlignment::center;
    Property<VerticalAlignment> verticalAlignment = VerticalAlignment::center;
    /**
     * Line-height multiplier when text wraps (multi-line). Single-line uses Typography::lineHeightTight.
     * Use Typography::lineHeightBody (~1.5) for long reading text.
     */
    Property<float> lineHeightMultiplier = Typography::lineHeightReadable;
    /** When true, letter spacing follows Typography::trackingFor(fontSize, weight). */
    Property<bool> automaticLetterSpacing = true;
    /** Used when automaticLetterSpacing is false. */
    Property<float> letterSpacing = 0.0f;

    TextStyle resolvedStyle(bool multiline) const {
        float fontSz = fontSize;
        FontWeight w = fontWeight;
        float lh = multiline ? static_cast<float>(lineHeightMultiplier) : Typography::lineHeightTight;
        float tr = static_cast<bool>(automaticLetterSpacing)
            ? Typography::trackingFor(fontSz, w)
            : static_cast<float>(letterSpacing);
        return makeTextStyle("default", w, fontSz, lh, tr);
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        EdgeInsets paddingVal = padding;
        float fontSz = fontSize;
        std::string text = value;
        HorizontalAlignment hAlign = horizontalAlignment;

        TextStyle style = resolvedStyle(false);
        ctx.setTextStyle(style);
        ctx.setFillStyle(FillStyle::solid(color));

        float contentWidth = bounds.width - paddingVal.horizontal();

        Size singleLineSize = ctx.measureText(text, style);
        bool needsWrap = singleLineSize.width > contentWidth && contentWidth > 0;

        if (needsWrap) {
            style = resolvedStyle(true);
            ctx.setTextStyle(style);
        }

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

        TextStyle style = resolvedStyle(false);
        Size textSize = textMeasurer.measureText(text, style);
        return {textSize.width + paddingVal.horizontal(),
                textSize.height + paddingVal.vertical()};
    }

    float heightForWidth(float width, TextMeasurement& textMeasurer) const {
        std::string text = value;
        EdgeInsets paddingVal = padding;

        float contentWidth = width - paddingVal.horizontal();
        if (contentWidth <= 0) {
            return paddingVal.vertical();
        }

        TextStyle tight = resolvedStyle(false);
        Size singleLine = textMeasurer.measureText(text, tight);
        if (singleLine.width <= contentWidth) {
            return singleLine.height + paddingVal.vertical();
        }

        TextStyle wrapped = resolvedStyle(true);
        Size boxSize = textMeasurer.measureTextBox(text, wrapped, contentWidth);
        return boxSize.height + paddingVal.vertical();
    }
};

} // namespace flux
