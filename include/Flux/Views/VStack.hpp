#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Views/StackLayout.hpp>
#include <algorithm>
#include <numeric>

namespace flux {

struct VStack {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;
    FLUX_TRANSFORM_PROPERTIES;

    Property<std::vector<View>> children = {};
    Property<float> spacing = 0;
    Property<JustifyContent> justifyContent = JustifyContent::start;
    Property<AlignItems> alignItems = AlignItems::stretch;

    LayoutNode layout(RenderContext& ctx, const Rect& bounds) {
        std::vector<View> childrenVec = children;
        EdgeInsets paddingVal = padding;
        float contentWidth = bounds.width - paddingVal.horizontal();

        std::vector<StackChildInput> inputs;
        inputs.reserve(childrenVec.size());
        for (const auto& child : childrenVec) {
            bool vis = child->isVisible();
            Size intrinsic = vis ? child.preferredSize(static_cast<TextMeasurement&>(ctx)) : Size{};
            if (vis) {
                auto childMaxW = child.getMaxWidth();
                float effectiveW = childMaxW.has_value()
                    ? std::min(contentWidth, childMaxW.value()) : contentWidth;
                intrinsic.height = child.heightForWidth(effectiveW, static_cast<TextMeasurement&>(ctx));
            }
            inputs.push_back({
                intrinsic,
                child.getExpansionBias(),
                child.getCompressionBias(),
                child.getMinWidth(),
                child.getMaxWidth(),
                child.getMinHeight(),
                child.getMaxHeight(),
                vis
            });
        }

        std::vector<Rect> rects = LayoutEngine::computeStack(
            StackAxis::Vertical, inputs, spacing, justifyContent, alignItems, paddingVal, bounds
        );

        StackLayoutResult<StackAxis::Vertical> result;
        result.childLayouts.reserve(childrenVec.size());
        for (size_t i = 0; i < childrenVec.size(); ++i) {
            if (!childrenVec[i]->isVisible()) continue;
            LayoutNode childLayout = childrenVec[i].layout(ctx, rects[i]);
            result.childLayouts.push_back(std::move(childLayout));
        }

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

    float heightForWidth(float width, TextMeasurement& textMeasurer) const {
        EdgeInsets paddingVal = padding;
        float childWidth = width - paddingVal.horizontal();
        float height = paddingVal.vertical();

        std::vector<View> childrenVec = children;
        bool hasVisibleChild = false;
        for (const auto& child : childrenVec) {
            if (!child->isVisible()) continue;

            auto childMaxW = child.getMaxWidth();
            float effectiveWidth = childMaxW.has_value()
                ? std::min(childWidth, childMaxW.value()) : childWidth;

            float childH = child.heightForWidth(effectiveWidth, textMeasurer);
            height += childH;

            if (hasVisibleChild) {
                height += static_cast<float>(spacing);
            }
            hasVisibleChild = true;
        }

        return height;
    }
};

} // namespace flux