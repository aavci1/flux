#include <Flux/Views/SVG.hpp>


#include <Flux/Graphics/Path.hpp>
#include <Flux/Core/ViewHelpers.hpp>

#include <iostream>
#include <algorithm>
#include <nanosvg.h>
#include <unordered_map>

namespace flux {

// Cache for parsed SVG data

// Cached parsed SVG path data (no NanoSVG dependencies)
struct CachedSVGPath {
    Path path;
    FillStyle fillStyle;
    StrokeStyle strokeStyle;
    PathWinding winding;
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
void drawCheckerboardBackground(RenderContext& ctx, const Rect& bounds);

CachedSVGData parseSVGContent(const std::string& svgStr) {
    CachedSVGData result;
    
    if (svgStr.empty()) {
        result.isValid = false;
        return result;
    }

    // Parse SVG using NanoSVG
    NSVGimage* image = nsvgParse(const_cast<char*>(svgStr.c_str()), "px", 96.0f);

    if (!image) {
        std::cout << "[SVG] Failed to parse SVG string" << std::endl;
        result.isValid = false;
        return result;
    }

    std::cout << "[SVG] Parsed SVG string: " << image->width << "x" << image->height << std::endl;

    // Store original dimensions
    result.originalWidth = static_cast<float>(image->width);
    result.originalHeight = static_cast<float>(image->height);

    // Convert all shapes to Path structures during parsing
    for (NSVGshape* shape = image->shapes; shape != nullptr; shape = shape->next) {
        parseShapeToPaths(shape, result.paths);
    }

    std::cout << "[SVG] Converted " << result.paths.size() << " paths from SVG shapes" << std::endl;

    // Clean up NanoSVG data
    nsvgDelete(image);
    
    result.isValid = true;
    return result;
}

void parseShapeToPaths(NSVGshape* shape, std::vector<CachedSVGPath>& paths) {
    if (!shape) return;

    std::cout << "[SVG] Parsing shape: " << shape->id << std::endl;

    // Convert NanoSVG color to Flux color
    Color fillColor = nsvgColorToFluxColor(shape->fill.color);
    Color strokeColor = nsvgColorToFluxColor(shape->stroke.color);
    float strokeWidth = shape->strokeWidth;

    // Apply opacity
    fillColor.a *= shape->opacity;
    strokeColor.a *= shape->opacity;

    // Parse each path in the shape
    for (NSVGpath* path = shape->paths; path != nullptr; path = path->next) {
        CachedSVGPath pathData;
        
        // Build the path
        pathData.path = buildPathFromSVG(path);
        
        // Determine winding direction based on area
        float area = calculatePathArea(path);
        std::cout << "[SVG] Path area: " << area << std::endl;
        pathData.winding = (area <= 0.0f) ? PathWinding::CounterClockwise : PathWinding::Clockwise;
        
        // Set fill style
        if (fillColor.a > 0.0f) {
            pathData.fillStyle = FillStyle::solid(fillColor);
            pathData.fillStyle.winding = pathData.winding;
        } else {
            pathData.fillStyle = FillStyle::none();
        }
        
        // Set stroke style
        if (strokeColor.a > 0.0f && strokeWidth > 0.0f) {
            pathData.strokeStyle = StrokeStyle::solid(strokeColor, strokeWidth);
        } else {
            pathData.strokeStyle = StrokeStyle::none();
        }
        
        pathData.opacity = shape->opacity;
        
        paths.push_back(pathData);
    }
}

Path buildPathFromSVG(NSVGpath* svgPath) {
    Path path;
    if (!svgPath || svgPath->npts < 2) return path;

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
    
    return path;
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
    float a = 1.0f;  // Alpha is stored separately in NanoSVG (opacity, etc.)

    return Color(r, g, b, a);
}

void renderCachedSVG(RenderContext& ctx, const CachedSVGData& data, const Rect& bounds) {
    drawCheckerboardBackground(ctx, bounds);

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
        // Set path winding
        ctx.setPathWinding(pathData.winding);
        ctx.setFillStyle(pathData.fillStyle);
        ctx.setStrokeStyle(pathData.strokeStyle);
        ctx.drawPath(pathData.path);
    }

    // Restore transform
    ctx.restore();
}

void drawCheckerboardBackground(RenderContext& ctx, const Rect& bounds) {
    // Draw checkerboard pattern
    const int squareSize = 20;
    const Color lightGray = Color(0.9f, 0.9f, 0.9f, 1.0f);
    const Color darkGray = Color(0.7f, 0.7f, 0.7f, 1.0f);

    ctx.setFillStyle(FillStyle::solid(lightGray));
    ctx.setStrokeStyle(StrokeStyle::none());
    ctx.drawRect(bounds);

    Path darkRects;
    for (int y = 0; y < bounds.height; y += squareSize) {
        for (int x = 0; x < bounds.width; x += squareSize) {
            bool isEven = ((x / squareSize) + (y / squareSize)) % 2 == 0;
            if (isEven) {
                continue;
            }

            darkRects.rect({static_cast<float>(x) + bounds.x, static_cast<float>(y) + bounds.y, static_cast<float>(squareSize), static_cast<float>(squareSize)});
        }
    }

    ctx.setFillStyle(FillStyle::solid(darkGray));
    ctx.setStrokeStyle(StrokeStyle::none());
    ctx.drawPath(darkRects);
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