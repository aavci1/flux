#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <string>
#include <vector>

namespace flux {

// SVG view component
struct SVG {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> content = "";
    Property<bool> preserveAspectRatio = true;
    Property<Size> size = Size{-1.0f, -1.0f};

    void render(RenderContext& ctx, const Rect& bounds) const;
    Size preferredSize(TextMeasurement& textMeasurer) const;
};

} // namespace flux