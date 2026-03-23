#include <Flux/Views/SVG.hpp>
#include <Flux/Utils/SVGUtils.hpp>

#include <functional>

namespace flux {

// Cache for parsed SVG data
static std::unordered_map<std::size_t, SVGData> svgCache;

// SVG public interface implementation
void SVG::render(RenderContext& ctx, const Rect& bounds) const {
    ViewHelpers::renderView(*this, ctx, bounds);

    const std::string svg = static_cast<std::string>(content);
    const std::size_t svgHash = std::hash<std::string>()(svg);

    // Get cached data or parse if not cached
    SVGData& data = svgCache[svgHash];
    if (data.paths.empty()) {
        data = parseSVG(svg);
        svgCache[svgHash] = data;
    }

    if (data.paths.empty()) {
        FLUX_LOG_ERROR("Failed to parse SVG: %s", svg.c_str());
        return;
    }

    FLUX_LOG_INFO("SVG: %s", svg.c_str());
    FLUX_LOG_INFO("SVG data: %f", data.originalWidth);
    FLUX_LOG_INFO("SVG data: %f", data.originalHeight);
    FLUX_LOG_INFO("SVG data: %zu", data.paths.size());
    // for (const auto& path : data.paths) {
    //     FLUX_LOG_INFO("SVG path: %s", path.path.toString().c_str());
    // }

    // Calculate scaling to fit within bounds
    EdgeInsets paddingVal = padding;
    Rect contentBounds = {
        bounds.x + paddingVal.left,
        bounds.y + paddingVal.top,
        bounds.width - paddingVal.horizontal(),
        bounds.height - paddingVal.vertical()
    };

    // Render using cached data
    float scaleX = bounds.width / data.originalWidth;
    float scaleY = bounds.height / data.originalHeight;

    // Save current transform
    ctx.save();

    if (preserveAspectRatio) {
        float scale = std::min(scaleX, scaleY);
        float offsetX = bounds.x + (bounds.width - data.originalWidth * scale) * 0.5f;
        float offsetY = bounds.y + (bounds.height - data.originalHeight * scale) * 0.5f;
        ctx.translate(offsetX, offsetY);
        ctx.scale(scale, scale);
    } else {
        ctx.translate(bounds.x, bounds.y);
        ctx.scale(scaleX, scaleY);
    }

    // Render all cached paths
    for (const auto& pathData : data.paths) {
        ctx.setFillStyle(pathData.fillStyle);
        ctx.setStrokeStyle(pathData.strokeStyle);
        ctx.drawPath(pathData.path);
    }

    // Restore transform
    ctx.restore();
}

Size SVG::preferredSize(TextMeasurement& /* textMeasurer */) const {
    const Size& sizeVal = size;
    if (sizeVal.width > 0 && sizeVal.height > 0) {
        return sizeVal;
    }

    EdgeInsets paddingVal = padding;

    const std::string svg = static_cast<std::string>(content);
    const std::size_t svgHash = std::hash<std::string>()(svg);

    // Get cached data or parse if not cached
    SVGData& data = svgCache[svgHash];
    if (data.paths.empty()) {
        data = parseSVG(svg);
        svgCache[svgHash] = data;
    }

    if (data.paths.empty()) {
        FLUX_LOG_ERROR("Failed to parse SVG: %s", svg.c_str());
        return Size(0.0f, 0.0f);
    }

    return {
        data.originalWidth + paddingVal.horizontal(),
        data.originalHeight + paddingVal.vertical()
    };
}

} // namespace flux