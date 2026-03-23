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
        FLUX_LOG_ERROR("Failed to parse SVG");
        return;
    }

    EdgeInsets paddingVal = padding;
    Rect contentArea = {
        bounds.x + paddingVal.left,
        bounds.y + paddingVal.top,
        bounds.width - paddingVal.horizontal(),
        bounds.height - paddingVal.vertical()
    };
    if (contentArea.width <= 0.0f || contentArea.height <= 0.0f) {
        return;
    }
    if (data.originalWidth <= 0.0f || data.originalHeight <= 0.0f) {
        FLUX_LOG_ERROR("SVG has invalid dimensions");
        return;
    }

    float scaleX = contentArea.width / data.originalWidth;
    float scaleY = contentArea.height / data.originalHeight;

    ctx.save();

    if (preserveAspectRatio) {
        float scale = std::min(scaleX, scaleY);
        float offsetX = contentArea.x + (contentArea.width - data.originalWidth * scale) * 0.5f;
        float offsetY = contentArea.y + (contentArea.height - data.originalHeight * scale) * 0.5f;
        ctx.translate(offsetX, offsetY);
        ctx.scale(scale, scale);
    } else {
        ctx.translate(contentArea.x, contentArea.y);
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
        FLUX_LOG_ERROR("Failed to parse SVG");
        return Size(0.0f, 0.0f);
    }

    return {
        data.originalWidth + paddingVal.horizontal(),
        data.originalHeight + paddingVal.vertical()
    };
}

} // namespace flux