#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <vector>

namespace flux {

struct Grid {
    FLUX_VIEW_PROPERTIES;

    Property<std::vector<View>> children = {};
    Property<int> columns = 1;
    Property<int> rows = 1;
    Property<float> spacing = 0;

    LayoutNode layout(RenderContext& ctx, const Rect& bounds) {
        EdgeInsets paddingVal = padding;
        float spacingVal = spacing;
        float availableWidth = bounds.width - paddingVal.horizontal() - spacingVal * (columns - 1);
        float availableHeight = bounds.height - paddingVal.vertical() - spacingVal * (rows - 1);

        std::vector<View> childrenVec = children;
        
        int visibleCount = childrenVec.size();
        if (visibleCount == 0) {
            return LayoutNode(View(*this), bounds);
        }

        float cellWidth = availableWidth / columns;
        float cellHeight = availableHeight / rows;
        
        // Track which grid cells are occupied
        std::vector<std::vector<bool>> occupied(rows, std::vector<bool>(columns, false));
        
        std::vector<LayoutNode> childLayouts;
        
        for (const auto& childView : childrenVec) {
            if (!childView->isVisible()) continue;
            
            // Get span properties from the child view
            int colspan = childView->getColspan();
            int rowspan = childView->getRowspan();
            
            // Find the first available position
            int startRow = -1, startCol = -1;
            for (int row = 0; row <= rows - rowspan; row++) {
                for (int col = 0; col <= columns - colspan; col++) {
                    bool canPlace = true;
                    // Check if all required cells are available
                    for (int r = row; r < row + rowspan && canPlace; r++) {
                        for (int c = col; c < col + colspan && canPlace; c++) {
                            if (occupied[r][c]) {
                                canPlace = false;
                            }
                        }
                    }
                    if (canPlace) {
                        startRow = row;
                        startCol = col;
                        break;
                    }
                }
                if (startRow != -1) break;
            }
            
            // If we couldn't find a place, skip this item
            if (startRow == -1) continue;
            
            // Mark cells as occupied
            for (int r = startRow; r < startRow + rowspan; r++) {
                for (int c = startCol; c < startCol + colspan; c++) {
                    occupied[r][c] = true;
                }
            }
            
            // Calculate position and size
            float x = bounds.x + startCol * (cellWidth + spacing) + paddingVal.left;
            float y = bounds.y + startRow * (cellHeight + spacing) + paddingVal.top;
            float width = colspan * cellWidth + (colspan - 1) * spacing;
            float height = rowspan * cellHeight + (rowspan - 1) * spacing;
            
            Rect childRect = {x, y, width, height};
            LayoutNode childLayout = childView.layout(ctx, childRect);
            childLayouts.push_back(childLayout);
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
