#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
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
        EdgeInsets paddingVal = padding;
        float availableWidth = bounds.width - paddingVal.horizontal();
        float availableHeight = bounds.height - paddingVal.vertical();

        std::vector<View> childrenVec = children;

        // Collect visible children with their base sizes and flex properties
        struct ChildInfo {
            View* child;
            float baseWidth;
            float expansionBias;
            float compressionBias;
        };
        std::vector<ChildInfo> visibleChildren;
        float totalBaseWidth = 0;

        for (auto& child : childrenVec) {
            if (!child->isVisible()) continue;

            Size childSize = child.preferredSize(static_cast<TextMeasurement&>(ctx)); // Use accurate measurement
            ChildInfo info = {
                &child,
                childSize.width,
                child.getExpansionBias(),
                child.getCompressionBias()
            };
            visibleChildren.push_back(info);
            totalBaseWidth += childSize.width;
        }

        int visibleCount = visibleChildren.size();
        if (visibleCount == 0) {
            return LayoutNode(View(*this), bounds);
        }

        // Calculate spacing and available space for content
        float baseSpacing = static_cast<float>(spacing);
        float totalSpacing = baseSpacing * (visibleCount - 1);
        float availableContentWidth = availableWidth - totalSpacing;

        // Distribute space using flexbox algorithm
        std::vector<float> finalWidths(visibleCount);
        float remainingSpace = availableContentWidth - totalBaseWidth;

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
                    finalWidths[i] = info.baseWidth + (remainingSpace * expansionRatio);
                }
            } else {
                for (size_t i = 0; i < visibleCount; ++i) {
                    finalWidths[i] = visibleChildren[i].baseWidth;
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
                    finalWidths[i] = std::max(0.0f, info.baseWidth - compressionAmount);
                }
            } else {
                for (size_t i = 0; i < visibleCount; ++i) {
                    finalWidths[i] = visibleChildren[i].baseWidth;
                }
            }
        } else {
            // No space change needed
            for (size_t i = 0; i < visibleCount; ++i) {
                finalWidths[i] = visibleChildren[i].baseWidth;
            }
        }

        // Apply justifyContent
        float startX = bounds.x + paddingVal.left;
        if (justifyContent == JustifyContent::center) {
            float totalUsedWidth = std::accumulate(finalWidths.begin(), finalWidths.end(), 0.0f);
            totalUsedWidth += totalSpacing;
            startX += (availableWidth - totalUsedWidth) / 2.0f;
        } else if (justifyContent == JustifyContent::end) {
            float totalUsedWidth = std::accumulate(finalWidths.begin(), finalWidths.end(), 0.0f);
            totalUsedWidth += totalSpacing;
            startX += availableWidth - totalUsedWidth;
        }

        // Layout children
        std::vector<LayoutNode> childLayouts;
        float x = startX;
        int childIndex = 0;

        for (const auto& info : visibleChildren) {
            float childWidth = finalWidths[childIndex];
            float childHeight = availableHeight;

            // Apply alignItems
            float childY = bounds.y + paddingVal.top;
            if (alignItems == AlignItems::center) {
                childY += (availableHeight - childHeight) / 2.0f;
            } else if (alignItems == AlignItems::end) {
                childY += availableHeight - childHeight;
            }

            Rect childRect = {x, childY, childWidth, childHeight};
            LayoutNode childLayout = info.child->layout(ctx, childRect);
            childLayouts.push_back(childLayout);

            // Move to next position
            x += childWidth;
            if (childIndex < visibleCount - 1) {
                x += baseSpacing;
            }
            childIndex++;
        }

        return LayoutNode(View(*this), bounds, std::move(childLayouts));
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