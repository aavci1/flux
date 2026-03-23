#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/Typography.hpp>
#include <string>
#include <variant>
#include <vector>

namespace flux {

namespace TextDetail {

inline std::vector<size_t> utf8CodepointStarts(const std::string& s) {
    std::vector<size_t> starts;
    for (size_t i = 0; i < s.size();) {
        starts.push_back(i);
        uint8_t c = static_cast<uint8_t>(s[i]);
        if (c < 0x80) {
            i += 1;
        } else if ((c & 0xE0) == 0xC0 && i + 1 < s.size()) {
            i += 2;
        } else if ((c & 0xF0) == 0xE0 && i + 2 < s.size()) {
            i += 3;
        } else if ((c & 0xF8) == 0xF0 && i + 3 < s.size()) {
            i += 4;
        } else {
            i += 1;
        }
    }
    return starts;
}

/** Single-line tail ellipsis; `maxWidth` is the horizontal space for text glyphs (padding excluded). */
inline std::string ellipsizeTail(TextMeasurement& tm, const std::string& text, const TextStyle& style, float maxWidth) {
    static const std::string kEll = "\xE2\x80\xA6";
    if (maxWidth <= 0.0f) {
        return "";
    }
    if (tm.measureText(text, style).width <= maxWidth) {
        return text;
    }
    float ellW = tm.measureText(kEll, style).width;
    if (ellW >= maxWidth) {
        return "";
    }
    float budget = maxWidth - ellW;
    std::vector<size_t> starts = utf8CodepointStarts(text);
    if (starts.empty()) {
        return kEll;
    }

    auto endByte = [&](int k) -> size_t {
        if (k <= 0) {
            return 0;
        }
        if (k >= static_cast<int>(starts.size())) {
            return text.size();
        }
        return starts[static_cast<size_t>(k)];
    };

    int lo = 0;
    int hi = static_cast<int>(starts.size());
    while (lo < hi) {
        int mid = (lo + hi + 1) / 2;
        std::string prefix = text.substr(0, endByte(mid));
        if (tm.measureText(prefix, style).width <= budget) {
            lo = mid;
        } else {
            hi = mid - 1;
        }
    }
    if (lo == 0) {
        return kEll;
    }
    return text.substr(0, endByte(lo)) + kEll;
}

} // namespace TextDetail

struct Text {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> value = "";
    /** macOS body default (~17pt). */
    Property<float> fontSize = Typography::body;
    Property<FontWeight> fontWeight = FontWeight::regular;
    Property<Color> color = Colors::inherit;
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
    /**
     * When true, draw a single line and truncate overflowing text with a trailing ellipsis (…).
     * Prefer pairing with expansionBias so the label can shrink inside toolbars and dropdown rows.
     */
    Property<bool> truncateTail = false;

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
        ctx.setFillStyle(FillStyle::solid(resolveColor(color, ctx.theme().foreground)));

        float contentWidth = bounds.width - paddingVal.horizontal();

        if (static_cast<bool>(truncateTail) && contentWidth > 0.0f) {
            std::string out = TextDetail::ellipsizeTail(static_cast<TextMeasurement&>(ctx), text, style, contentWidth);
            Point textPos = {bounds.x + paddingVal.left, bounds.y + paddingVal.top};
            HorizontalAlignment hAlign = horizontalAlignment;
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
            ctx.drawText(out, textPos, hAlign, vAlign);
            return;
        }

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
        EdgeInsets paddingVal = padding;

        TextStyle style = resolvedStyle(false);
        if (static_cast<bool>(truncateTail)) {
            Size textSize = textMeasurer.measureText(text, style);
            return {paddingVal.horizontal(), textSize.height + paddingVal.vertical()};
        }
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
        if (static_cast<bool>(truncateTail)) {
            Size singleLine = textMeasurer.measureText(text, tight);
            return singleLine.height + paddingVal.vertical();
        }

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
