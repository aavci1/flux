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
        
        // Calculate dynamic spacing for space distribution modes
        float dynamicSpacing = baseSpacing;
        if (visibleCount > 1) {
            float totalUsedWidth = std::accumulate(visibleChildren.begin(), visibleChildren.end(), 0.0f, 
                [](float sum, const ChildInfo& info) { return sum + info.baseWidth; });
            float availableSpace = availableWidth - totalUsedWidth;
            
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
        float totalUsedWidth = std::accumulate(finalWidths.begin(), finalWidths.end(), 0.0f);
        float totalAvailableSpace = availableWidth - totalUsedWidth;
        
        if (justifyContent == JustifyContent::center) {
            startX += totalAvailableSpace / 2.0f;
        } else if (justifyContent == JustifyContent::end) {
            startX += totalAvailableSpace;
        } else if (justifyContent == JustifyContent::spaceBetween) {
            // No additional offset needed - children will be positioned with calculated spacing
        } else if (justifyContent == JustifyContent::spaceAround) {
            // No additional offset needed - children will be positioned with calculated spacing
        } else if (justifyContent == JustifyContent::spaceEvenly) {
            // No additional offset needed - children will be positioned with calculated spacing
        }

        // Layout children
        std::vector<LayoutNode> childLayouts;
        float x = startX;
        int childIndex = 0;

        // Add initial spacing for spaceAround and spaceEvenly
        if (justifyContent == JustifyContent::spaceAround) {
            x += dynamicSpacing / 2.0f;  // x/2 space at the beginning
        } else if (justifyContent == JustifyContent::spaceEvenly) {
            x += dynamicSpacing;  // x space at the beginning
        }

        for (const auto& info : visibleChildren) {
            float childWidth = finalWidths[childIndex];
            float childHeight;

            // Determine child height based on alignItems
            if (alignItems == AlignItems::stretch) {
                childHeight = availableHeight;
            } else {
                // Use preferred height for non-stretch alignments
                Size childPreferredSize = info.child->preferredSize(static_cast<TextMeasurement&>(ctx));
                childHeight = childPreferredSize.height;
            }

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
                // Use dynamic spacing for space distribution modes, otherwise use base spacing
                if (justifyContent == JustifyContent::spaceBetween || 
                    justifyContent == JustifyContent::spaceAround || 
                    justifyContent == JustifyContent::spaceEvenly) {
                    x += dynamicSpacing;
                } else {
                    x += baseSpacing;
                }
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