#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <nanosvg.h>
#include <string>
#include <memory>

namespace flux {

// SVG data structure using NanoSVG
struct SVGData {
    NSVGimage* image = nullptr;
    std::string content;

    ~SVGData() {
        if (image) {
            nsvgDelete(image);
        }
    }
};

// SVG view component
struct SVG {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> content = "";
    Property<bool> preserveAspectRatio = true;

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        if (static_cast<std::string>(content).empty()) {
            return;
        }

        // Parse SVG string using NanoSVG
        auto svgData = parsecontent(static_cast<std::string>(content));
        if (!svgData || !svgData->image) {
            return;
        }

        // Calculate scaling to fit within bounds
        EdgeInsets paddingVal = padding;
        Rect contentBounds = {
            bounds.x + paddingVal.left,
            bounds.y + paddingVal.top,
            bounds.width - paddingVal.horizontal(),
            bounds.height - paddingVal.vertical()
        };

        // Render SVG using NanoSVG data
        renderSVG(ctx, svgData->image, contentBounds);
    }

    Size preferredSize(TextMeasurement& /* textMeasurer */) const {
        EdgeInsets paddingVal = padding;

        if (!static_cast<std::string>(content).empty()) {
            auto svgData = parsecontent(static_cast<std::string>(content));
            if (svgData && svgData->image) {
                return {
                    static_cast<float>(svgData->image->width) + paddingVal.horizontal(),
                    static_cast<float>(svgData->image->height) + paddingVal.vertical()
                };
            }
        }

        // Default size
        return {200 + paddingVal.horizontal(), 200 + paddingVal.vertical()};
    }

private:
    std::shared_ptr<SVGData> parsecontent(const std::string& svgStr) const;
    void renderSVG(RenderContext& ctx, NSVGimage* image, const Rect& bounds) const;
    void renderShape(RenderContext& ctx, NSVGshape* shape, const Rect& bounds, float scale) const;
    void renderPath(RenderContext& ctx, NSVGpath* path, const Rect& bounds, float scale,
                    const Color& fillColor, const Color& strokeColor, float strokeWidth) const;

    // Helper functions
    Color nsvgColorToFluxColor(unsigned int color) const;
    float parseFloat(const std::string& str) const;
};

} // namespace flux