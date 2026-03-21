#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Views/StackLayout.hpp>
#include <algorithm>
#include <numeric>

namespace flux {

struct HStack {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;
    FLUX_TRANSFORM_PROPERTIES;

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

    float heightForWidth(float width, TextMeasurement& textMeasurer) const {
        EdgeInsets paddingVal = padding;
        float availableWidth = width - paddingVal.horizontal();

        std::vector<View> childrenVec = children;
        std::vector<const View*> visibleChildren;
        float totalPreferredWidth = 0;
        float totalExpansion = 0;
        int visibleCount = 0;

        for (const auto& child : childrenVec) {
            auto lc = child.getLayoutConstraints();
            if (!lc.visible) continue;
            visibleChildren.push_back(&child);
            Size childSize = child.preferredSize(textMeasurer);
            totalPreferredWidth += childSize.width;
            totalExpansion += lc.expansionBias;
            visibleCount++;
        }

        float totalSpacing = visibleCount > 1 ? static_cast<float>(spacing) * (visibleCount - 1) : 0;
        float contentWidth = availableWidth - totalSpacing;
        float remainingSpace = contentWidth - totalPreferredWidth;

        float maxHeight = 0;
        for (const auto* child : visibleChildren) {
            auto lc = child->getLayoutConstraints();
            Size childSize = child->preferredSize(textMeasurer);
            float childWidth = childSize.width;
            if (remainingSpace > 0 && totalExpansion > 0) {
                childWidth += remainingSpace * lc.expansionBias / totalExpansion;
            }
            if (lc.maxWidth.has_value()) childWidth = std::min(childWidth, lc.maxWidth.value());

            float childH = child->heightForWidth(childWidth, textMeasurer);
            maxHeight = std::max(maxHeight, childH);
        }

        return maxHeight + paddingVal.vertical();
    }
};

} // namespace flux