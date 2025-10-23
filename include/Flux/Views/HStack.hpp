#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Views/StackLayout.hpp>
#include <iostream>
#include <algorithm>
#include <numeric>

namespace flux {

struct HStack {
    FLUX_VIEW_PROPERTIES;

    Property<std::vector<View>> children = {};
    Property<float> spacing = 0;
    Property<JustifyContent> justifyContent = JustifyContent::start;
    Property<AlignItems> alignItems = AlignItems::stretch;

    LayoutNode layout(RenderContext& ctx, const Rect& bounds) {
        std::vector<View> childrenVec = children;
        auto result = layoutStack<StackAxis::Horizontal>(
            childrenVec, spacing, justifyContent, alignItems, padding, bounds, ctx
        );
        return LayoutNode(View(*this), bounds, std::move(result.childLayouts));
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        EdgeInsets paddingVal = padding;
        float width = paddingVal.horizontal(), height = 0;

        std::vector<View> childrenVec = children;
        bool hasVisibleChild = false;
        for (const auto& child : childrenVec) {
            if (!child->isVisible()) continue;

            Size childSize = child.preferredSize(textMeasurer);
            height = std::max(height, childSize.height);
            width += childSize.width;

            if (hasVisibleChild) {
                width += static_cast<float>(spacing);
            }
            hasVisibleChild = true;
        }

        return {width, height + paddingVal.vertical()};
    }
};

} // namespace flux