#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
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
        EdgeInsets paddingVal = padding;
        float availableWidth = bounds.width - paddingVal.horizontal();
        float availableHeight = bounds.height - paddingVal.vertical();

        std::vector<View> childrenVec = children;

        // Collect visible children with their base sizes and flex properties
        struct ChildInfo {
            View* child;
            float baseHeight;
            float expansionBias;
            float compressionBias;
        };
        std::vector<ChildInfo> visibleChildren;
        float totalBaseHeight = 0;

        for (auto& child : childrenVec) {
            if (!child->isVisible()) continue;

            Size childSize = child.preferredSize(static_cast<TextMeasurement&>(ctx)); // Use accurate measurement
            ChildInfo info = {
                &child,
                childSize.height,
                child.getExpansionBias(),
                child.getCompressionBias()
            };
            visibleChildren.push_back(info);
            totalBaseHeight += childSize.height;
        }

        int visibleCount = visibleChildren.size();
        if (visibleCount == 0) {
            return LayoutNode(View(*this), bounds);
        }

        // Calculate spacing and available space for content
        float baseSpacing = static_cast<float>(spacing);
        float totalSpacing = baseSpacing * (visibleCount - 1);
        float availableContentHeight = availableHeight - totalSpacing;

        // Distribute space using flexbox algorithm
        std::vector<float> finalHeights(visibleCount);
        float remainingSpace = availableContentHeight - totalBaseHeight;

        if (remainingSpace > 0) {
            // Expansion phase: distribute extra space
            float totalExpansionBias = 0;
            for (const auto& info : visibleChildren) {
                totalExpansionBias += info.expansionBias;
            }

            if (totalExpansionBias > 0) {
                for (size_t i = 0; i < visibleCount; ++i) {
                    const auto& info = visibleChildren[i];
                    float expansionRatio = info.expansionBias / totalExpansionBias;
                    finalHeights[i] = info.baseHeight + (remainingSpace * expansionRatio);
                }
            } else {
                for (size_t i = 0; i < visibleCount; ++i) {
                    finalHeights[i] = visibleChildren[i].baseHeight;
                }
            }
        } else if (remainingSpace < 0) {
            // Compression phase: reduce space proportionally
            float totalCompressionBias = 0;
            for (const auto& info : visibleChildren) {
                totalCompressionBias += info.compressionBias;
            }

            if (totalCompressionBias > 0) {
                for (size_t i = 0; i < visibleCount; ++i) {
                    const auto& info = visibleChildren[i];
                    float compressionRatio = info.compressionBias / totalCompressionBias;
                    float compressionAmount = std::abs(remainingSpace) * compressionRatio;
                    finalHeights[i] = std::max(0.0f, info.baseHeight - compressionAmount);
                }
            } else {
                for (size_t i = 0; i < visibleCount; ++i) {
                    finalHeights[i] = visibleChildren[i].baseHeight;
                }
            }
        } else {
            // No space change needed
            for (size_t i = 0; i < visibleCount; ++i) {
                finalHeights[i] = visibleChildren[i].baseHeight;
            }
        }

        // Apply justifyContent
        float startY = bounds.y + paddingVal.top;
        if (justifyContent == JustifyContent::center) {
            float totalUsedHeight = std::accumulate(finalHeights.begin(), finalHeights.end(), 0.0f);
            totalUsedHeight += totalSpacing;
            startY += (availableHeight - totalUsedHeight) / 2.0f;
        } else if (justifyContent == JustifyContent::end) {
            float totalUsedHeight = std::accumulate(finalHeights.begin(), finalHeights.end(), 0.0f);
            totalUsedHeight += totalSpacing;
            startY += availableHeight - totalUsedHeight;
        }

        // Layout children
        std::vector<LayoutNode> childLayouts;
        float y = startY;
        int childIndex = 0;

        for (const auto& info : visibleChildren) {
            float childHeight = finalHeights[childIndex];
            float childWidth = availableWidth;

            // Apply alignItems
            float childX = bounds.x + paddingVal.left;
            if (alignItems == AlignItems::center) {
                childX += (availableWidth - childWidth) / 2.0f;
            } else if (alignItems == AlignItems::end) {
                childX += availableWidth - childWidth;
            }

            Rect childRect = {childX, y, childWidth, childHeight};
            LayoutNode childLayout = info.child->layout(ctx, childRect);
            childLayouts.push_back(childLayout);

            // Move to next position
            y += childHeight;
            if (childIndex < visibleCount - 1) {
                y += baseSpacing;
            }
            childIndex++;
        }

        return LayoutNode(View(*this), bounds, std::move(childLayouts));
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