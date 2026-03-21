#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Typography.hpp>
#include <Flux/Views/HStack.hpp>
#include <Flux/Views/Text.hpp>
#include <string>
#include <vector>
#include <algorithm>

namespace flux {

namespace LabeledControl {

inline View build(View accessory, const std::string& labelText,
    LabelPosition labelPosition, JustifyContent justifyContent,
    float spacing, EdgeInsets padding, float labelFontSize, Color labelColor) {
    if (labelText.empty()) {
        return accessory;
    }

    Text labelView {
        .value = labelText,
        .fontSize = labelFontSize,
        .color = labelColor,
        .verticalAlignment = VerticalAlignment::center,
        .horizontalAlignment = HorizontalAlignment::leading
    };

    std::vector<View> children = {
        std::move(accessory),
        View(labelView)
    };

    if (labelPosition == LabelPosition::leading) {
        std::reverse(children.begin(), children.end());
    }

    return HStack {
        .spacing = spacing,
        .justifyContent = justifyContent,
        .alignItems = AlignItems::center,
        .padding = padding,
        .children = std::move(children)
    };
}

inline Size measure(float accessoryWidth, float accessoryHeight,
    const std::string& labelText, float labelFontSize, float spacing,
    EdgeInsets padding, TextMeasurement& textMeasurer) {
    if (labelText.empty()) {
        return {accessoryWidth + padding.horizontal(), accessoryHeight + padding.vertical()};
    }

    Size textSize = textMeasurer.measureText(labelText,
        makeTextStyle("default", FontWeight::regular, labelFontSize,
            Typography::lineHeightTight,
            Typography::trackingFor(labelFontSize, FontWeight::regular)));
    float totalWidth = accessoryWidth + spacing + textSize.width + padding.horizontal();
    float totalHeight = std::max(accessoryHeight, textSize.height) + padding.vertical();
    return {totalWidth, totalHeight};
}

} // namespace LabeledControl

} // namespace flux
