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
        
        // Calculate dynamic spacing for space distribution modes
        float dynamicSpacing = baseSpacing;
        if (visibleCount > 1) {
            float totalUsedHeight = std::accumulate(visibleChildren.begin(), visibleChildren.end(), 0.0f, 
                [](float sum, const ChildInfo& info) { return sum + info.baseHeight; });
            float availableSpace = availableHeight - totalUsedHeight;
            
            if (justifyContent == JustifyContent::spaceBetween) {
                dynamicSpacing = availableSpace / (visibleCount - 1);
            } else if (justifyContent == JustifyContent::spaceAround) {
                // spaceAround: x/2 at edges, x between items
                // Total spacing = 2 * (x/2) + (visibleCount-1) * x = x + (visibleCount-1) * x = visibleCount * x
                // So: visibleCount * x = availableSpace, therefore x = availableSpace / visibleCount
                dynamicSpacing = availableSpace / visibleCount;
            } else if (justifyContent == JustifyContent::spaceEvenly) {
                dynamicSpacing = availableSpace / (visibleCount + 1);
            }
        }

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
        float totalUsedHeight = std::accumulate(finalHeights.begin(), finalHeights.end(), 0.0f);
        float totalAvailableSpace = availableHeight - totalUsedHeight;
        
        if (justifyContent == JustifyContent::center) {
            startY += totalAvailableSpace / 2.0f;
        } else if (justifyContent == JustifyContent::end) {
            startY += totalAvailableSpace;
        } else if (justifyContent == JustifyContent::spaceBetween) {
            // No additional offset needed - children will be positioned with calculated spacing
        } else if (justifyContent == JustifyContent::spaceAround) {
            // No additional offset needed - children will be positioned with calculated spacing
        } else if (justifyContent == JustifyContent::spaceEvenly) {
            // No additional offset needed - children will be positioned with calculated spacing
        }

        // Layout children
        std::vector<LayoutNode> childLayouts;
        float y = startY;
        int childIndex = 0;

        // Add initial spacing for spaceAround and spaceEvenly
        if (justifyContent == JustifyContent::spaceAround) {
            y += dynamicSpacing / 2.0f;  // x/2 space at the beginning
        } else if (justifyContent == JustifyContent::spaceEvenly) {
            y += dynamicSpacing;  // x space at the beginning
        }

        for (const auto& info : visibleChildren) {
            float childHeight = finalHeights[childIndex];
            float childWidth;

            // Determine child width based on alignItems
            if (alignItems == AlignItems::stretch) {
                childWidth = availableWidth;
            } else {
                // Use preferred width for non-stretch alignments
                Size childPreferredSize = info.child->preferredSize(static_cast<TextMeasurement&>(ctx));
                childWidth = childPreferredSize.width;
            }

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
                // Use dynamic spacing for space distribution modes, otherwise use base spacing
                if (justifyContent == JustifyContent::spaceBetween || 
                    justifyContent == JustifyContent::spaceAround || 
                    justifyContent == JustifyContent::spaceEvenly) {
                    y += dynamicSpacing;
                } else {
                    y += baseSpacing;
                }
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