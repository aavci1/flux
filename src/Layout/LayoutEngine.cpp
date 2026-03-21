#include <Flux/Layout/LayoutEngine.hpp>
#include <algorithm>
#include <cmath>
#include <numeric>

namespace flux {

std::vector<Rect> LayoutEngine::computeStack(
    StackAxis axis,
    const std::vector<StackChildInput>& children,
    float spacing,
    JustifyContent justifyContent,
    AlignItems alignItems,
    const EdgeInsets& padding,
    const Rect& bounds
) {
    std::vector<Rect> result;

    float availableMainSize = (axis == StackAxis::Horizontal)
        ? (bounds.width - padding.horizontal())
        : (bounds.height - padding.vertical());
    float availableCrossSize = (axis == StackAxis::Horizontal)
        ? (bounds.height - padding.vertical())
        : (bounds.width - padding.horizontal());

    struct Info {
        size_t originalIndex;
        float baseSize;
        float expansionBias;
        float compressionBias;
        Size intrinsicSize;
    };

    std::vector<Info> visible;
    visible.reserve(children.size());
    float totalBaseSize = 0;
    float totalExpansionBias = 0;
    float totalCompressionBias = 0;

    for (size_t i = 0; i < children.size(); ++i) {
        const auto& c = children[i];
        if (!c.visible) continue;

        float preferred = (axis == StackAxis::Horizontal) ? c.intrinsicSize.width : c.intrinsicSize.height;

        std::optional<float> minSize, maxSize;
        if (axis == StackAxis::Horizontal) {
            minSize = c.minWidth; maxSize = c.maxWidth;
        } else {
            minSize = c.minHeight; maxSize = c.maxHeight;
        }

        float baseSize = preferred;
        if (minSize && baseSize < *minSize) baseSize = *minSize;
        if (maxSize && baseSize > *maxSize) baseSize = *maxSize;

        visible.push_back({i, baseSize, c.expansionBias, c.compressionBias, c.intrinsicSize});
        totalBaseSize += baseSize;
        totalExpansionBias += c.expansionBias;
        totalCompressionBias += c.compressionBias;
    }

    size_t visibleCount = visible.size();
    if (visibleCount == 0) return result;

    float baseSpacing = spacing;
    float totalSpacing = baseSpacing * static_cast<float>(visibleCount - 1);
    float availableContentSize = availableMainSize - totalSpacing;
    float remainingSpace = availableContentSize - totalBaseSize;

    float dynamicSpacing = baseSpacing;
    if (visibleCount > 1 && remainingSpace >= 0) {
        float availableSpace = availableMainSize - totalBaseSize;
        if (justifyContent == JustifyContent::spaceBetween)
            dynamicSpacing = std::max(baseSpacing, availableSpace / static_cast<float>(visibleCount - 1));
        else if (justifyContent == JustifyContent::spaceAround)
            dynamicSpacing = std::max(baseSpacing, availableSpace / static_cast<float>(visibleCount));
        else if (justifyContent == JustifyContent::spaceEvenly)
            dynamicSpacing = std::max(baseSpacing, availableSpace / static_cast<float>(visibleCount + 1));
    }

    std::vector<float> finalSizes;
    finalSizes.reserve(visibleCount);

    if (remainingSpace > 0) {
        if (totalExpansionBias > 0) {
            for (const auto& info : visible)
                finalSizes.push_back(info.baseSize + (remainingSpace * info.expansionBias / totalExpansionBias));
        } else {
            for (const auto& info : visible) finalSizes.push_back(info.baseSize);
        }
    } else if (remainingSpace < 0) {
        if (totalCompressionBias > 0) {
            for (const auto& info : visible) {
                float ratio = info.compressionBias / totalCompressionBias;
                finalSizes.push_back(std::max(0.0f, info.baseSize - std::abs(remainingSpace) * ratio));
            }
        } else {
            if (availableContentSize > 0) {
                float ratio = availableContentSize / totalBaseSize;
                for (const auto& info : visible)
                    finalSizes.push_back(std::max(0.0f, info.baseSize * ratio));
            } else {
                for (size_t i = 0; i < visibleCount; ++i)
                    finalSizes.push_back(0.0f);
            }
        }
    } else {
        for (const auto& info : visible) finalSizes.push_back(info.baseSize);
    }

    for (size_t i = 0; i < visibleCount; ++i) {
        float& size = finalSizes[i];
        const auto& c = children[visible[i].originalIndex];
        std::optional<float> minSize, maxSize;
        if (axis == StackAxis::Horizontal) { minSize = c.minWidth; maxSize = c.maxWidth; }
        else { minSize = c.minHeight; maxSize = c.maxHeight; }
        if (minSize && size < *minSize) size = *minSize;
        if (maxSize && size > *maxSize) size = *maxSize;
    }

    float positioningSpacing = baseSpacing;
    bool useDynamicSpacing = false;
    if (remainingSpace >= 0) {
        useDynamicSpacing = (justifyContent == JustifyContent::spaceBetween ||
                             justifyContent == JustifyContent::spaceAround ||
                             justifyContent == JustifyContent::spaceEvenly);
        if (useDynamicSpacing) positioningSpacing = dynamicSpacing;
    }
    (void)useDynamicSpacing;

    float startMainPos = (axis == StackAxis::Horizontal)
        ? (bounds.x + padding.left) : (bounds.y + padding.top);
    float totalUsedSize = std::accumulate(finalSizes.begin(), finalSizes.end(), 0.0f);
    float totalAvailableSpace = availableMainSize - totalUsedSize;

    if (justifyContent == JustifyContent::center && remainingSpace >= 0) {
        startMainPos += totalAvailableSpace / 2.0f;
    } else if (justifyContent == JustifyContent::end) {
        float totalSpacingSize = (visibleCount > 1) ? baseSpacing * static_cast<float>(visibleCount - 1) : 0.0f;
        float totalUsedWithSpacing = totalUsedSize + totalSpacingSize;
        if (axis == StackAxis::Horizontal)
            startMainPos = bounds.x + bounds.width - padding.right - totalUsedWithSpacing;
        else
            startMainPos = bounds.y + bounds.height - padding.bottom - totalUsedWithSpacing;
    }

    float mainPos = startMainPos;
    if (remainingSpace >= 0) {
        if (justifyContent == JustifyContent::spaceAround)
            mainPos += dynamicSpacing / 2.0f;
        else if (justifyContent == JustifyContent::spaceEvenly)
            mainPos += dynamicSpacing;
    }

    result.resize(children.size());
    for (size_t i = 0; i < visibleCount; ++i) {
        const auto& info = visible[i];
        float childMainSize = finalSizes[i];
        float childCrossSize = (alignItems == AlignItems::stretch) ? availableCrossSize
            : ((axis == StackAxis::Horizontal) ? info.intrinsicSize.height : info.intrinsicSize.width);

        float childCrossPos = (axis == StackAxis::Horizontal)
            ? (bounds.y + padding.top) : (bounds.x + padding.left);
        if (alignItems == AlignItems::center)
            childCrossPos += (availableCrossSize - childCrossSize) / 2.0f;
        else if (alignItems == AlignItems::end)
            childCrossPos += availableCrossSize - childCrossSize;

        Rect r = (axis == StackAxis::Horizontal)
            ? Rect{mainPos, childCrossPos, childMainSize, childCrossSize}
            : Rect{childCrossPos, mainPos, childCrossSize, childMainSize};
        result[info.originalIndex] = r;

        mainPos += childMainSize;
        if (i < visibleCount - 1) mainPos += positioningSpacing;
    }

    return result;
}

std::vector<Rect> LayoutEngine::computeGrid(
    const std::vector<GridChildInput>& children,
    int columns,
    int rows,
    float spacing,
    const EdgeInsets& padding,
    const Rect& bounds
) {
    std::vector<Rect> result(children.size());

    float availableWidth = bounds.width - padding.horizontal() - spacing * static_cast<float>(columns - 1);
    float availableHeight = bounds.height - padding.vertical() - spacing * static_cast<float>(rows - 1);
    float cellWidth = availableWidth / static_cast<float>(columns);
    float cellHeight = availableHeight / static_cast<float>(rows);

    std::vector<std::vector<bool>> occupied(rows, std::vector<bool>(columns, false));

    for (size_t i = 0; i < children.size(); ++i) {
        const auto& c = children[i];
        if (!c.visible) continue;

        int startRow = -1, startCol = -1;
        for (int row = 0; row <= rows - c.rowspan; row++) {
            for (int col = 0; col <= columns - c.colspan; col++) {
                bool canPlace = true;
                for (int r = row; r < row + c.rowspan && canPlace; r++)
                    for (int cc = col; cc < col + c.colspan && canPlace; cc++)
                        if (occupied[r][cc]) canPlace = false;
                if (canPlace) { startRow = row; startCol = col; break; }
            }
            if (startRow != -1) break;
        }

        if (startRow == -1) continue;

        for (int r = startRow; r < startRow + c.rowspan; r++)
            for (int cc = startCol; cc < startCol + c.colspan; cc++)
                occupied[r][cc] = true;

        float x = bounds.x + static_cast<float>(startCol) * (cellWidth + spacing) + padding.left;
        float y = bounds.y + static_cast<float>(startRow) * (cellHeight + spacing) + padding.top;
        float width = static_cast<float>(c.colspan) * cellWidth + static_cast<float>(c.colspan - 1) * spacing;
        float height = static_cast<float>(c.rowspan) * cellHeight + static_cast<float>(c.rowspan - 1) * spacing;

        result[i] = {x, y, width, height};
    }

    return result;
}

} // namespace flux
