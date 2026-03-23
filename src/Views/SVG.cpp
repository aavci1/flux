#include <Flux/Views/SVG.hpp>


#include <Flux/Graphics/Path.hpp>
#include <Flux/Core/ViewHelpers.hpp>

#include <algorithm>
#include <cmath>
#include <nanosvg.h>
#include <unordered_map>

namespace flux {

// Cache for parsed SVG data

// Cached parsed SVG path data (no NanoSVG dependencies)
struct CachedSVGPath {
    Path path;
    FillStyle fillStyle;
    StrokeStyle strokeStyle;
    float opacity;
};

// Cached SVG data
struct CachedSVGData {
    std::vector<CachedSVGPath> paths;
    float originalWidth = 0.0f;
    float originalHeight = 0.0f;
    bool isValid = false;
};

static std::unordered_map<std::string, CachedSVGData> svgCache;

// Regular functions (not members) for SVG parsing
namespace svg_impl {

// Forward declarations
void parseShapeToPaths(NSVGshape* shape, std::vector<CachedSVGPath>& paths);
Path buildPathFromSVG(NSVGpath* svgPath);
float calculatePathArea(NSVGpath* path);
Color nsvgColorToFluxColor(unsigned int color);
FillStyle nsvgPaintToFillStyle(const NSVGpaint& paint);

CachedSVGData parseSVGContent(const std::string& svgStr) {
    CachedSVGData result;
    
    if (svgStr.empty()) {
        result.isValid = false;
        return result;
    }

    // Parse SVG using NanoSVG
    NSVGimage* image = nsvgParse(const_cast<char*>(svgStr.c_str()), "px", 96.0f);

    if (!image) {
        result.isValid = false;
        return result;
    }

    // Store original dimensions
    result.originalWidth = static_cast<float>(image->width);
    result.originalHeight = static_cast<float>(image->height);

    // Convert all shapes to Path structures during parsing
    for (NSVGshape* shape = image->shapes; shape != nullptr; shape = shape->next) {
        parseShapeToPaths(shape, result.paths);
    }



    // Clean up NanoSVG data
    nsvgDelete(image);
    
    result.isValid = true;
    return result;
}

void parseShapeToPaths(NSVGshape* shape, std::vector<CachedSVGPath>& paths) {
    if (!shape) return;

    // Convert NanoSVG paint (color or gradient) to FillStyle
    FillStyle fillStyle = nsvgPaintToFillStyle(shape->fill);
    
    // Convert stroke color
    Color strokeColor = nsvgColorToFluxColor(shape->stroke.color);
    float strokeWidth = shape->strokeWidth;

    // Create one Flux path for all paths in this shape
    Path path;
    
    // Parse each NSVG path and add to the Flux path
    for (NSVGpath* svgPath = shape->paths; svgPath != nullptr; svgPath = svgPath->next) {
        if (!svgPath || svgPath->npts < 2) {
            continue;
        }

        // Calculate the signed area to determine winding
        auto area = calculatePathArea(svgPath);
        
        // In SVG, positive area means counter-clockwise (solid), negative means clockwise (hole)
        // Flux uses PathWinding::CounterClockwise for solid, PathWinding::Clockwise for holes
        // So we set the winding right before adding this path
        path.setWinding(area >= 0.0f ? PathWinding::Clockwise : PathWinding::CounterClockwise);

        // Move to first point
        path.moveTo({svgPath->pts[0], svgPath->pts[1]});
    
        // Draw cubic bezier curves
        for (int i = 1; i < svgPath->npts - 1; i += 3) {
            if (i + 2 < svgPath->npts) {
                path.bezierTo(
                    {svgPath->pts[i*2], svgPath->pts[i*2+1]},         // control point 1
                    {svgPath->pts[(i+1)*2], svgPath->pts[(i+1)*2+1]}, // control point 2
                    {svgPath->pts[(i+2)*2], svgPath->pts[(i+2)*2+1]}  // end point
                );
            }
        }
    
        // Close path if it's closed
        if (svgPath->closed) {
            path.close();
        }
    }
    
    // Add the complete path to the paths vector
    paths.push_back({
        .path = path,
        .fillStyle = fillStyle,
        .strokeStyle = StrokeStyle::solid(strokeColor, strokeWidth),
        .opacity = shape->opacity
    });
}

float calculatePathArea(NSVGpath* path) {
    if (!path || path->npts < 2) return 0.0f;
    
    // Calculate signed area using shoelace formula
    float area = 0.0f;
    
    // Use only the actual vertices (every 3rd point in cubic bezier data)
    for (int i = 0; i < path->npts - 1; i += 3) {
        float x1 = path->pts[i * 2];
        float y1 = path->pts[i * 2 + 1];
        float x2 = path->pts[(i + 3) * 2];
        float y2 = path->pts[(i + 3) * 2 + 1];
        area += (x1 * y2 - x2 * y1);
    }
    
    // Close the polygon if it's closed
    if (path->closed) {
        int lastVertex = (path->npts - 1) / 3 * 3;
        float x1 = path->pts[lastVertex * 2];
        float y1 = path->pts[lastVertex * 2 + 1];
        float x2 = path->pts[0];
        float y2 = path->pts[1];
        area += (x1 * y2 - x2 * y1);
    }
    
    return area * 0.5f;  // Shoelace formula gives 2*area
}

Color nsvgColorToFluxColor(unsigned int color) {
    // NanoSVG uses BGR format: NSVG_RGB(r, g, b) = r | (g << 8) | (b << 16)
    // So the format is 0x00BBGGRR
    float r = (color & 0xFF) / 255.0f;           // bits 0-7
    float g = ((color >> 8) & 0xFF) / 255.0f;    // bits 8-15
    float b = ((color >> 16) & 0xFF) / 255.0f;   // bits 16-23
    float a = ((color >> 24) & 0xFF) / 255.0f;   // bits 24-31

    return Color(r, g, b, a);
}

FillStyle nsvgPaintToFillStyle(const NSVGpaint& paint) {
    switch (paint.type) {
        case NSVG_PAINT_NONE:
            return FillStyle::none();
            
        case NSVG_PAINT_COLOR: {
            Color color = nsvgColorToFluxColor(paint.color);
            return FillStyle::solid(color);
        }
        
        case NSVG_PAINT_LINEAR_GRADIENT: {
            if (paint.gradient && paint.gradient->nstops >= 2) {
                Color sc = nsvgColorToFluxColor(paint.gradient->stops[0].color);
                Color ec = nsvgColorToFluxColor(paint.gradient->stops[paint.gradient->nstops - 1].color);
                float* xform = paint.gradient->xform;
                float dx = xform[2];
                float dy = xform[3];
                Point sp = {xform[4], xform[5]};
                Point ep;
                float gradLength = sqrtf(dx * dx + dy * dy);
                if (gradLength > 0.0001f) {
                    ep = {sp.x + dx, sp.y + dy};
                } else {
                    ep = {sp.x + 100, sp.y};
                }
                return FillStyle::linearGradient(sp, ep, sc, ec);
            }
            return FillStyle::linearGradient({0,0}, {100,0}, Colors::black, Colors::white);
        }

        case NSVG_PAINT_RADIAL_GRADIENT: {
            if (paint.gradient && paint.gradient->nstops >= 2) {
                Color sc = nsvgColorToFluxColor(paint.gradient->stops[0].color);
                Color ec = nsvgColorToFluxColor(paint.gradient->stops[paint.gradient->nstops - 1].color);
                float* xform = paint.gradient->xform;
                Point center = {xform[4], xform[5]};
                float outerR = 100.0f;
                float focusX = paint.gradient->fx;
                float focusY = paint.gradient->fy;
                float fdx = focusX - center.x;
                float fdy = focusY - center.y;
                outerR = sqrtf(fdx * fdx + fdy * fdy);
                return FillStyle::radialGradient(center, 0.0f, outerR, sc, ec);
            }
            return FillStyle::radialGradient({50,50}, 0.0f, 100.0f, Colors::black, Colors::white);
        }
        
        default:
            return FillStyle::none();
    }
}

void renderCachedSVG(RenderContext& ctx, const CachedSVGData& data, const Rect& bounds) {
    if (!data.isValid || data.paths.empty()) return;

    // Calculate scaling to fit within bounds
    float scaleX = bounds.width / data.originalWidth;
    float scaleY = bounds.height / data.originalHeight;
    float scale = std::min(scaleX, scaleY);

    // Center the SVG in the bounds
    float offsetX = bounds.x + (bounds.width - data.originalWidth * scale) * 0.5f;
    float offsetY = bounds.y + (bounds.height - data.originalHeight * scale) * 0.5f;

    // Save current transform
    ctx.save();

    // Apply transform
    ctx.translate(offsetX, offsetY);
    ctx.scale(scale, scale);

    // Render all cached paths
    for (const auto& pathData : data.paths) {
        ctx.setFillStyle(pathData.fillStyle);
        ctx.setStrokeStyle(pathData.strokeStyle);
        ctx.drawPath(pathData.path);
    }

    // Restore transform
    ctx.restore();
}

} // namespace svg_impl

// SVG public interface implementation
void SVG::render(RenderContext& ctx, const Rect& bounds) const {
    ViewHelpers::renderView(*this, ctx, bounds);

    const std::string currentContent = static_cast<std::string>(content);
    
    // Get cached data or parse if not cached
    CachedSVGData& cachedData = svgCache[currentContent];
    if (!cachedData.isValid) {
        cachedData = svg_impl::parseSVGContent(currentContent);
    }

    // Calculate scaling to fit within bounds
    EdgeInsets paddingVal = padding;
    Rect contentBounds = {
        bounds.x + paddingVal.left,
        bounds.y + paddingVal.top,
        bounds.width - paddingVal.horizontal(),
        bounds.height - paddingVal.vertical()
    };

    // Render using cached data
    svg_impl::renderCachedSVG(ctx, cachedData, contentBounds);
}

Size SVG::preferredSize(TextMeasurement& /* textMeasurer */) const {
    const Size& sizeVal = size;
    if (sizeVal.width > 0 && sizeVal.height > 0) {
        return sizeVal;
    }

    EdgeInsets paddingVal = padding;

    const std::string currentContent = static_cast<std::string>(content);
    
    // Get cached data or parse if not cached
    CachedSVGData& cachedData = svgCache[currentContent];
    if (!cachedData.isValid) {
        cachedData = svg_impl::parseSVGContent(currentContent);
    }

    if (cachedData.isValid) {
        return {
            cachedData.originalWidth + paddingVal.horizontal(),
            cachedData.originalHeight + paddingVal.vertical()
        };
    }

    // Default size
    return {200 + paddingVal.horizontal(), 200 + paddingVal.vertical()};
}

} // namespace flux