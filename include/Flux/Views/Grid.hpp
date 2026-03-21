#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Layout/LayoutEngine.hpp>
#include <algorithm>
#include <numeric>
#include <vector>

namespace flux {

struct Grid {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;
    FLUX_TRANSFORM_PROPERTIES;

    Property<std::vector<View>> children = {};
    Property<int> columns = 1;
    Property<int> rows = 1;
    Property<float> spacing = 0;

    LayoutNode layout(RenderContext& ctx, const Rect& bounds) {
        std::vector<View> childrenVec = children;

        if (childrenVec.empty()) {
            return LayoutNode(View(*this), bounds);
        }

        std::vector<GridChildInput> inputs;
        inputs.reserve(childrenVec.size());
        for (const auto& childView : childrenVec) {
            auto lc = childView.getLayoutConstraints();
            inputs.push_back({
                lc.colspan,
                lc.rowspan,
                lc.visible
            });
        }

        EdgeInsets paddingVal = padding;
        std::vector<Rect> rects = LayoutEngine::computeGrid(
            inputs, columns, rows, spacing, paddingVal, bounds
        );

        std::vector<LayoutNode> childLayouts;
        for (size_t i = 0; i < childrenVec.size(); ++i) {
            if (!childrenVec[i]->isVisible()) continue;
            if (rects[i].width <= 0 && rects[i].height <= 0) continue;
            childLayouts.push_back(childrenVec[i].layout(ctx, rects[i]));
        }

        return LayoutNode(View(*this), bounds, std::move(childLayouts));
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        EdgeInsets paddingVal = padding;
        float width = paddingVal.horizontal();
        float height = paddingVal.vertical();

        std::vector<View> childrenVec = children;
        for (const auto& childView : childrenVec) {
            if (!childView->isVisible()) continue;
            
            Size childSize = childView.preferredSize(textMeasurer);
            width = std::max(width, childSize.width);
            height = std::max(height, childSize.height);
        }

        return {width, height};
    }
};

} // namespace flux
