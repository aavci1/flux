#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <algorithm>
#include <numeric>

namespace flux {

enum class StackAxis {
    Horizontal,
    Vertical
};

template<StackAxis Axis>
struct StackChildInfo {
    const View* child;
    float baseSize;
    float expansionBias;
    float compressionBias;
    Size preferredSize;  // Cache the preferred size to avoid recalculation
};

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
    
    EdgeInsets paddingVal = padding;
    float availableMainSize = (Axis == StackAxis::Horizontal) ? 
        (bounds.width - paddingVal.horizontal()) : 
        (bounds.height - paddingVal.vertical());
    float availableCrossSize = (Axis == StackAxis::Horizontal) ? 
        (bounds.height - paddingVal.vertical()) : 
        (bounds.width - paddingVal.horizontal());

    // Collect visible children with their base sizes and flex properties
    std::vector<StackChildInfo<Axis>> visibleChildren;
    visibleChildren.reserve(children.size()); // Reserve space to avoid reallocations
    float totalBaseSize = 0;
    float totalExpansionBias = 0;
    float totalCompressionBias = 0;

    for (const auto& child : children) {
        if (!child->isVisible()) continue;

        Size childSize = child.preferredSize(static_cast<TextMeasurement&>(ctx));
        float preferredSize = (Axis == StackAxis::Horizontal) ? childSize.width : childSize.height;
        
        // Apply min/max constraints to base size before distribution
        std::optional<float> minSize, maxSize;
        if constexpr (Axis == StackAxis::Horizontal) {
            minSize = child.getMinWidth();
            maxSize = child.getMaxWidth();
        } else {
            minSize = child.getMinHeight();
            maxSize = child.getMaxHeight();
        }
        
        float baseSize = preferredSize;
        if (minSize && baseSize < *minSize) {
            baseSize = *minSize;
        }
        if (maxSize && baseSize > *maxSize) {
            baseSize = *maxSize;
        }
        
        StackChildInfo<Axis> info = {
            &child,
            baseSize,
            child.getExpansionBias(),
            child.getCompressionBias(),
            childSize  // Cache the preferred size for later use
        };
        visibleChildren.push_back(info);
        totalBaseSize += baseSize;
        totalExpansionBias += info.expansionBias;
        totalCompressionBias += info.compressionBias;
    }

    size_t visibleCount = visibleChildren.size();
    if (visibleCount == 0) {
        return result;
    }

    // Calculate spacing and available space for content
    // Spacing is treated as MINIMUM spacing - it should never be less than specified
    float baseSpacing = static_cast<float>(spacing);
    float totalSpacing = baseSpacing * (visibleCount - 1);
    
    // Calculate available space for children after reserving space for spacing
    float availableContentSize = availableMainSize - totalSpacing;
    float remainingSpace = availableContentSize - totalBaseSize;
    
    // Calculate dynamic spacing for space distribution modes
    float dynamicSpacing = baseSpacing;
    
    if (visibleCount > 1 && remainingSpace >= 0) {
        // Enough space - calculate dynamic spacing for space distribution
        float availableSpace = availableMainSize - totalBaseSize;
        
        if (justifyContent == JustifyContent::spaceBetween) {
            dynamicSpacing = std::max(baseSpacing, availableSpace / (visibleCount - 1));
        } else if (justifyContent == JustifyContent::spaceAround) {
            // spaceAround: x/2 at edges, x between items
            // Total spacing = 2 * (x/2) + (visibleCount-1) * x = x + (visibleCount-1) * x = visibleCount * x
            // So: visibleCount * x = availableSpace, therefore x = availableSpace / visibleCount
            dynamicSpacing = std::max(baseSpacing, availableSpace / visibleCount);
        } else if (justifyContent == JustifyContent::spaceEvenly) {
            dynamicSpacing = std::max(baseSpacing, availableSpace / (visibleCount + 1));
        }
    }

    // Distribute space using flexbox algorithm
    std::vector<float> finalSizes;
    finalSizes.reserve(visibleCount); // Reserve space to avoid reallocations

    if (remainingSpace > 0) {
        // Expansion phase: distribute extra space
        if (totalExpansionBias > 0) {
            for (const auto& info : visibleChildren) {
                float expansionRatio = info.expansionBias / totalExpansionBias;
                finalSizes.push_back(info.baseSize + (remainingSpace * expansionRatio));
            }
        } else {
            for (const auto& info : visibleChildren) {
                finalSizes.push_back(info.baseSize);
            }
        }
    } else if (remainingSpace < 0) {
        // Compression phase: children must shrink to maintain minimum spacing
        // All available space (after spacing) must be distributed among children
        if (totalCompressionBias > 0) {
            for (const auto& info : visibleChildren) {
                float compressionRatio = info.compressionBias / totalCompressionBias;
                float compressionAmount = std::abs(remainingSpace) * compressionRatio;
                finalSizes.push_back(std::max(0.0f, info.baseSize - compressionAmount));
            }
        } else {
            // No compression bias - compress all children uniformly
            if (availableContentSize > 0) {
                float uniformCompressionRatio = availableContentSize / totalBaseSize;
                for (const auto& info : visibleChildren) {
                    finalSizes.push_back(std::max(0.0f, info.baseSize * uniformCompressionRatio));
                }
            } else {
                // Not enough space even for spacing - set all children to 0
                for (const auto& info : visibleChildren) {
                    finalSizes.push_back(0.0f);
                }
            }
        }
    } else {
        // No space change needed
        for (const auto& info : visibleChildren) {
            finalSizes.push_back(info.baseSize);
        }
    }
    
    // Apply min/max size constraints (these may differ from baseSize if distribution changed them)
    for (size_t i = 0; i < visibleCount; ++i) {
        float& size = finalSizes[i];
        const auto& info = visibleChildren[i];
        
        // Get constraints based on axis
        std::optional<float> minSize, maxSize;
        if constexpr (Axis == StackAxis::Horizontal) {
            minSize = info.child->getMinWidth();
            maxSize = info.child->getMaxWidth();
        } else {
            minSize = info.child->getMinHeight();
            maxSize = info.child->getMaxHeight();
        }
        
        // Apply constraints (only if distribution changed the size from baseSize)
        if (minSize && size < *minSize) {
            size = *minSize;
        }
        if (maxSize && size > *maxSize) {
            size = *maxSize;
        }
    }
    
    // Effective spacing is always at least the minimum spacing
    float effectiveSpacing = baseSpacing;

    // Apply justifyContent
    float startMainPos = (Axis == StackAxis::Horizontal) ? 
        (bounds.x + paddingVal.left) : 
        (bounds.y + paddingVal.top);
    float totalUsedSize = std::accumulate(finalSizes.begin(), finalSizes.end(), 0.0f);
    float totalAvailableSpace = availableMainSize - totalUsedSize;
    
    // Determine which spacing to use for positioning
    float positioningSpacing = effectiveSpacing;
    bool useDynamicSpacing = false;
    
    if (remainingSpace >= 0) {
        // Only use dynamic spacing when we have enough space
        useDynamicSpacing = (justifyContent == JustifyContent::spaceBetween || 
                             justifyContent == JustifyContent::spaceAround || 
                             justifyContent == JustifyContent::spaceEvenly);
        if (useDynamicSpacing) {
            positioningSpacing = dynamicSpacing;
        }
    }
    
    if (justifyContent == JustifyContent::center && remainingSpace >= 0) {
        startMainPos += totalAvailableSpace / 2.0f;
    } else if (justifyContent == JustifyContent::end) {
        // For end, position so that the rightmost child ends at the right padding boundary
        float totalSpacingSize = (visibleCount > 1) ? effectiveSpacing * (visibleCount - 1) : 0.0f;
        float totalUsedSizeWithSpacing = totalUsedSize + totalSpacingSize;
        
        if (Axis == StackAxis::Horizontal) {
            startMainPos = bounds.x + bounds.width - paddingVal.right - totalUsedSizeWithSpacing;
        } else {
            startMainPos = bounds.y + bounds.height - paddingVal.bottom - totalUsedSizeWithSpacing;
        }
    }
    // spaceBetween, spaceAround, and spaceEvenly don't need additional offset

    // Layout children
    float mainPos = startMainPos;
    result.childLayouts.reserve(visibleCount); // Reserve space for child layouts

    // Add initial spacing for spaceAround and spaceEvenly (only when we have enough space)
    if (remainingSpace >= 0) {
        if (justifyContent == JustifyContent::spaceAround) {
            mainPos += dynamicSpacing / 2.0f;  // x/2 space at the beginning
        } else if (justifyContent == JustifyContent::spaceEvenly) {
            mainPos += dynamicSpacing;  // x space at the beginning
        }
    }

    for (size_t i = 0; i < visibleCount; ++i) {
        const auto& info = visibleChildren[i];
        float childMainSize = finalSizes[i];
        float childCrossSize;

        // Determine child cross size based on alignItems
        if (alignItems == AlignItems::stretch) {
            childCrossSize = availableCrossSize;
        } else {
            // Use cached preferred size to avoid recalculation
            childCrossSize = (Axis == StackAxis::Horizontal) ? info.preferredSize.height : info.preferredSize.width;
        }

        // Apply alignItems
        float childCrossPos = (Axis == StackAxis::Horizontal) ? 
            (bounds.y + paddingVal.top) : 
            (bounds.x + paddingVal.left);
        if (alignItems == AlignItems::center) {
            childCrossPos += (availableCrossSize - childCrossSize) / 2.0f;
        } else if (alignItems == AlignItems::end) {
            childCrossPos += availableCrossSize - childCrossSize;
        }

        // Create child rect based on axis
        Rect childRect = (Axis == StackAxis::Horizontal) ?
            Rect{mainPos, childCrossPos, childMainSize, childCrossSize} :
            Rect{childCrossPos, mainPos, childCrossSize, childMainSize};
        
        LayoutNode childLayout = info.child->layout(ctx, childRect);
        result.childLayouts.push_back(std::move(childLayout));

        // Move to next position
        mainPos += childMainSize;
        if (i < visibleCount - 1) {
            mainPos += positioningSpacing;
        }
    }

    return result;
}

} // namespace flux
