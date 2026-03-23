#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <Flux/Graphics/Path.hpp>
#include <vector>
#include <string>

namespace flux {

struct SVGPath {
    Path path;
    FillStyle fillStyle;
    StrokeStyle strokeStyle;
    float opacity;
};

struct SVGData {
    std::vector<SVGPath> paths;
    float originalWidth = 0.0f;
    float originalHeight = 0.0f;
};

SVGData parseSVG(const std::string& svg);

} // namespace flux