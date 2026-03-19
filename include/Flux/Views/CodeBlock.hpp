#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Views/VStack.hpp>
#include <Flux/Views/HStack.hpp>
#include <Flux/Views/Text.hpp>
#include <Flux/Views/Button.hpp>
#include <Flux/Views/Spacer.hpp>
#include <string>
#include <sstream>
#include <algorithm>
#include <Flux/Core/Typography.hpp>

namespace flux {

struct CodeBlock {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;

    Property<std::string> code = std::string("");
    Property<std::string> language = std::string("");
    Property<Color> codeBgColor = Color(0.1f, 0.1f, 0.1f);
    Property<Color> codeTextColor = Color(0.85f, 0.85f, 0.85f);
    Property<Color> headerBgColor = Color(0.08f, 0.08f, 0.08f);
    Property<Color> headerTextColor = Color(0.48f, 0.48f, 0.48f);
    Property<float> headerFontSize = Typography::subheadline;
    Property<float> codeFontSize = Typography::caption;
    Property<float> codePadding = 12.0f;
    Property<float> codeCornerRadius = 6.0f;

    mutable bool copied = false;

    void render(RenderContext& ctx, const Rect& bounds) const {
        float pad = codePadding;
        float rad = codeCornerRadius;
        float fs = codeFontSize;

        ctx.setFillStyle(FillStyle::solid(codeBgColor));
        ctx.setStrokeStyle(StrokeStyle::none());
        ctx.drawRect(bounds, CornerRadius(rad));

        std::string lang = language;
        float headerH = 0;
        if (!lang.empty()) {
            headerH = 24.0f;
            Rect headerRect = {bounds.x, bounds.y, bounds.width, headerH};
            ctx.setFillStyle(FillStyle::solid(headerBgColor));
            ctx.drawRect(headerRect, CornerRadius(rad, rad, 0, 0));

            float hfs = headerFontSize;
            TextStyle headerStyle = makeTextStyle("default", FontWeight::regular, hfs, Typography::lineHeightTight,
                Typography::trackingFor(hfs, FontWeight::regular));
            ctx.setTextStyle(headerStyle);
            ctx.setFillStyle(FillStyle::solid(headerTextColor));
            ctx.drawText(lang, {bounds.x + pad, bounds.y + headerH / 2},
                HorizontalAlignment::leading, VerticalAlignment::center);

            std::string copyLabel = copied ? "Copied!" : "Copy";
            ctx.setFillStyle(FillStyle::solid(headerTextColor));
            ctx.drawText(copyLabel,
                {bounds.x + bounds.width - pad, bounds.y + headerH / 2},
                HorizontalAlignment::trailing, VerticalAlignment::center);
        }

        std::string codeStr = code;
        float lineH = fs * Typography::lineHeightBody;
        float y = bounds.y + headerH + pad + fs;
        float x = bounds.x + pad;

        TextStyle codeStyle = makeTextStyle("default", FontWeight::regular, fs, Typography::lineHeightBody,
            Typography::trackingCaption(fs));
        ctx.setTextStyle(codeStyle);
        ctx.setFillStyle(FillStyle::solid(codeTextColor));

        std::istringstream stream(codeStr);
        std::string line;
        while (std::getline(stream, line)) {
            if (y > bounds.y + bounds.height) break;
            ctx.drawText(line, {x, y},
                HorizontalAlignment::leading, VerticalAlignment::bottom);
            y += lineH;
        }
    }

    Size preferredSize(TextMeasurement& tm) const {
        float fs = codeFontSize;
        float pad = codePadding;
        std::string codeStr = code;
        std::string lang = language;

        TextStyle measureStyle = makeTextStyle("default", FontWeight::regular, fs, Typography::lineHeightBody,
            Typography::trackingCaption(fs));

        int lineCount = 0;
        float maxLineW = 0;
        std::istringstream stream(codeStr);
        std::string line;
        while (std::getline(stream, line)) {
            Size ls = tm.measureText(line, measureStyle);
            maxLineW = std::max(maxLineW, ls.width);
            lineCount++;
        }
        if (lineCount == 0) lineCount = 1;

        float headerH = lang.empty() ? 0 : 24.0f;
        return {
            maxLineW + pad * 2,
            headerH + pad * 2 + lineCount * fs * Typography::lineHeightBody
        };
    }
};

} // namespace flux
