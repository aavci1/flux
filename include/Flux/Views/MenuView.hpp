#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Views/StackLayout.hpp>

namespace flux {

/**
 * Vertical stack styled as a floating menu panel (dropdown list, context menu, menu bar popup).
 * Delegates layout to the same engine as VStack. Present with showMenuOverlay() in
 * OverlayManager.hpp (dismiss-on-outside-click defaults).
 */
struct MenuView {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;
    FLUX_TRANSFORM_PROPERTIES;

    Property<std::vector<View>> children = {};
    Property<float> spacing = 1.0f;
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
        std::vector<View> childrenVec = children;
        return stackPreferredSize<StackAxis::Vertical>(childrenVec, spacing, padding, textMeasurer);
    }

    float heightForWidth(float width, TextMeasurement& textMeasurer) const {
        std::vector<View> childrenVec = children;
        return stackHeightForWidth<StackAxis::Vertical>(childrenVec, width, spacing, padding, textMeasurer);
    }
};

} // namespace flux
