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
    float contentWidth = bounds.width - padding.horizontal();

    std::vector<StackChildInput> inputs;
    inputs.reserve(children.size());
    for (const auto& child : children) {
        auto lc = child.getLayoutConstraints();
        Size intrinsic = lc.visible ? child.preferredSize(static_cast<TextMeasurement&>(ctx)) : Size{};
        if constexpr (Axis == StackAxis::Vertical) {
            if (lc.visible) {
                float effectiveW = lc.maxWidth.has_value()
                    ? std::min(contentWidth, lc.maxWidth.value()) : contentWidth;
                intrinsic.height = child.heightForWidth(effectiveW, static_cast<TextMeasurement&>(ctx));
            }
        }
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

template<StackAxis Axis>
Size stackPreferredSize(
    const std::vector<View>& children,
    float spacing,
    const EdgeInsets& padding,
    TextMeasurement& textMeasurer
) {
    float main = 0, cross = 0;
    bool hasVisible = false;
    for (const auto& child : children) {
        if (!child->isVisible()) continue;
        Size cs = child.preferredSize(textMeasurer);
        if constexpr (Axis == StackAxis::Vertical) {
            main += cs.height;
            cross = std::max(cross, cs.width);
        } else {
            main += cs.width;
            cross = std::max(cross, cs.height);
        }
        if (hasVisible) main += spacing;
        hasVisible = true;
    }
    if constexpr (Axis == StackAxis::Vertical) {
        return {cross + padding.horizontal(), main + padding.vertical()};
    } else {
        return {main + padding.horizontal(), cross + padding.vertical()};
    }
}

template<StackAxis Axis>
float stackHeightForWidth(
    const std::vector<View>& children,
    float width,
    float spacing,
    const EdgeInsets& padding,
    TextMeasurement& textMeasurer
) {
    float childWidth = width - padding.horizontal();

    if constexpr (Axis == StackAxis::Vertical) {
        float height = padding.vertical();
        bool hasVisible = false;
        for (const auto& child : children) {
            auto lc = child.getLayoutConstraints();
            if (!lc.visible) continue;
            float effectiveW = lc.maxWidth.has_value()
                ? std::min(childWidth, lc.maxWidth.value()) : childWidth;
            height += child.heightForWidth(effectiveW, textMeasurer);
            if (hasVisible) height += spacing;
            hasVisible = true;
        }
        return height;
    } else {
        std::vector<const View*> visible;
        float totalPrefW = 0, totalExp = 0;
        for (const auto& child : children) {
            auto lc = child.getLayoutConstraints();
            if (!lc.visible) continue;
            visible.push_back(&child);
            totalPrefW += child.preferredSize(textMeasurer).width;
            totalExp += lc.expansionBias;
        }
        float totalSpacing = visible.size() > 1 ? spacing * (visible.size() - 1) : 0;
        float remaining = childWidth - totalSpacing - totalPrefW;
        float maxH = 0;
        for (const auto* c : visible) {
            auto lc = c->getLayoutConstraints();
            float cw = c->preferredSize(textMeasurer).width;
            if (remaining > 0 && totalExp > 0)
                cw += remaining * lc.expansionBias / totalExp;
            if (lc.maxWidth.has_value()) cw = std::min(cw, lc.maxWidth.value());
            maxH = std::max(maxH, c->heightForWidth(cw, textMeasurer));
        }
        return maxH + padding.vertical();
    }
}

} // namespace flux
