#pragma once

#include <Flux/Core/Types.hpp>
#include <vector>
#include <optional>

namespace flux {

enum class StackAxis { Horizontal, Vertical };

struct StackChildInput {
    Size intrinsicSize;
    float expansionBias = 0.0f;
    float compressionBias = 1.0f;
    std::optional<float> minWidth, maxWidth, minHeight, maxHeight;
    bool visible = true;
};

struct GridChildInput {
    int colspan = 1;
    int rowspan = 1;
    bool visible = true;
};

class LayoutEngine {
public:
    static std::vector<Rect> computeStack(
        StackAxis axis,
        const std::vector<StackChildInput>& children,
        float spacing,
        JustifyContent justifyContent,
        AlignItems alignItems,
        const EdgeInsets& padding,
        const Rect& bounds
    );

    static std::vector<Rect> computeGrid(
        const std::vector<GridChildInput>& children,
        int columns,
        int rows,
        float spacing,
        const EdgeInsets& padding,
        const Rect& bounds
    );
};

} // namespace flux
