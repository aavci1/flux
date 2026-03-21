#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Layout/LayoutEngine.hpp>
#include <algorithm>
#include <numeric>

namespace flux {

template<StackAxis Axis>
struct StackLayoutResult {
    std::vector<LayoutNode> childLayouts;
};

template<StackAxis Axis>
StackLayoutResult<Axis> layoutStack(
    const std::vector<View>& children,
    float spacing,
    JustifyContent justifyContent,
    AlignItems alignItems,
    const EdgeInsets& padding,
    const Rect& bounds,
    RenderContext& ctx
) {
    StackLayoutResult<Axis> result;

    std::vector<StackChildInput> inputs;
    inputs.reserve(children.size());
    for (const auto& child : children) {
        auto lc = child.getLayoutConstraints();
        Size intrinsic = lc.visible ? child.preferredSize(static_cast<TextMeasurement&>(ctx)) : Size{};
        inputs.push_back({
            intrinsic,
            lc.expansionBias,
            lc.compressionBias,
            lc.minWidth,
            lc.maxWidth,
            lc.minHeight,
            lc.maxHeight,
            lc.visible
        });
    }

    std::vector<Rect> rects = LayoutEngine::computeStack(
        Axis, inputs, spacing, justifyContent, alignItems, padding, bounds
    );

    result.childLayouts.reserve(children.size());
    for (size_t i = 0; i < children.size(); ++i) {
        if (!children[i]->isVisible()) continue;
        LayoutNode childLayout = children[i].layout(ctx, rects[i]);
        result.childLayouts.push_back(std::move(childLayout));
    }

    return result;
}

} // namespace flux
