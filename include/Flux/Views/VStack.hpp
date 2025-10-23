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

struct VStack {
    FLUX_VIEW_PROPERTIES;

    Property<std::vector<View>> children = {};
    Property<float> spacing = 0;
    Property<JustifyContent> justifyContent = JustifyContent::start;
    Property<AlignItems> alignItems = AlignItems::stretch;

    LayoutNode layout(RenderContext& ctx, const Rect& bounds) {
        std::vector<View> childrenVec = children;
        auto result = layoutStack<StackAxis::Vertical>(
            childrenVec, spacing, justifyContent, alignItems, padding, bounds, ctx
        );
        return LayoutNode(View(*this), bounds, std::move(result.childLayouts));
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        EdgeInsets paddingVal = padding;
        float width = 0, height = paddingVal.vertical();

        std::vector<View> childrenVec = children;
        bool hasVisibleChild = false;
        for (const auto& child : childrenVec) {
            if (!child->isVisible()) continue;

            Size childSize = child.preferredSize(textMeasurer);
            width = std::max(width, childSize.width);
            height += childSize.height;

            if (hasVisibleChild) {
                height += static_cast<float>(spacing);
            }
            hasVisibleChild = true;
        }

        return {width + paddingVal.horizontal(), height};
    }
};

} // namespace flux