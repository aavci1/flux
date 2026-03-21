#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Views/StackLayout.hpp>

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
        std::vector<View> childrenVec = children;
        return stackPreferredSize<StackAxis::Horizontal>(childrenVec, spacing, padding, textMeasurer);
    }

    float heightForWidth(float width, TextMeasurement& textMeasurer) const {
        std::vector<View> childrenVec = children;
        return stackHeightForWidth<StackAxis::Horizontal>(childrenVec, width, spacing, padding, textMeasurer);
    }
};

} // namespace flux
