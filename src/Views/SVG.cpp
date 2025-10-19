#include <Flux/Views/SVG.hpp>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <typeinfo>
#include <nanosvg.h>

namespace flux {

std::shared_ptr<SVGData> SVG::parsecontent(const std::string& svgStr) const {
    auto svgData = std::make_shared<SVGData>();
    svgData->content = svgStr;

    // Parse SVG using NanoSVG
    svgData->image = nsvgParse(const_cast<char*>(svgStr.c_str()), "px", 96.0f);

    std::cout << "[SVG] Parsed SVG string: " << svgData->image->width << "x" << svgData->image->height << std::endl;

    if (!svgData->image) {
        std::cout << "[SVG] Failed to parse SVG string" << std::endl;
        return nullptr;
    }


    return svgData;
}

void SVG::renderSVG(RenderContext& ctx, NSVGimage* image, const Rect& bounds) const {
    drawCheckerboardBackground(ctx, bounds);

    if (!image) return;

    // Calculate scaling to fit within bounds
    float scaleX = bounds.width / static_cast<float>(image->width);
    float scaleY = bounds.height / static_cast<float>(image->height);
    float scale = std::min(scaleX, scaleY);

    // Center the SVG in the bounds
    float offsetX = bounds.x + (bounds.width - static_cast<float>(image->width) * scale) * 0.5f;
    float offsetY = bounds.y + (bounds.height - static_cast<float>(image->height) * scale) * 0.5f;

    // Save current transform
    ctx.save();

    // Apply transform
    ctx.translate(offsetX, offsetY);
    ctx.scale(scale, scale);

    // Render all shapes
    for (NSVGshape* shape = image->shapes; shape != nullptr; shape = shape->next) {
        std::cout << "[SVG] Rendering shape: " << shape->id << std::endl;
        renderShape(ctx, shape, bounds, scale);
    }

    // Restore transform
    ctx.restore();
}

void SVG::renderShape(RenderContext& ctx, NSVGshape* shape, const Rect& bounds, float scale) const {
    if (!shape) return;

    std::cout << "[SVG] Shape: " << shape->id << std::endl;
    std::cout << "[SVG] Shape fill color: " << shape->fill.color << std::endl;
    std::cout << "[SVG] Shape stroke color: " << shape->stroke.color << std::endl;
    std::cout << "[SVG] Shape stroke width: " << shape->strokeWidth << std::endl;
    std::cout << "[SVG] Shape opacity: " << shape->opacity << std::endl;
    std::cout << "[SVG] Shape fillRule: " << (int)shape->fillRule << std::endl;

    // Convert NanoSVG color to Flux color
    Color fillColor = nsvgColorToFluxColor(shape->fill.color);
    Color strokeColor = nsvgColorToFluxColor(shape->stroke.color);
    float strokeWidth = shape->strokeWidth;

    std::cout << "[SVG] Fill color: " << fillColor.r << ", " << fillColor.g << ", " << fillColor.b << ", " << fillColor.a << std::endl;

    // Apply opacity
    fillColor.a *= shape->opacity;
    strokeColor.a *= shape->opacity;

    // Based on VCV Rack's implementation, we can properly handle SVG fill rules
    // by using ray casting to determine if each path is a hole or solid shape
    // This works with NanoVG's even-odd fill rule
    
    std::cout << "[SVG] Shape fillRule: " << (shape->fillRule == NSVG_FILLRULE_EVENODD ? "EVENODD" : "NONZERO") << std::endl;
    
    // Render each path individually with proper winding directions
    // This approach is more similar to VCV Rack's implementation
    
    for (NSVGpath* path = shape->paths; path != nullptr; path = path->next) {
        // Use area calculation to determine winding direction
        // Positive area = counter-clockwise = solid shape
        // Negative area = clockwise = hole
        float area = calculatePathArea(path);
        bool isHole = area <= 0.0f;
        
        ctx.beginPath();
        
        if (isHole) {
            ctx.setPathWinding(PathWinding::Clockwise);  // Hole (NVG_CW)
            std::cout << "[SVG] Path is a hole (area=" << area << ")" << std::endl;
        } else {
            ctx.setPathWinding(PathWinding::CounterClockwise);  // Solid (NVG_CCW)
            std::cout << "[SVG] Path is solid (area=" << area << ")" << std::endl;
        }
        
        addPathToContext(ctx, path);
        
        // Fill this individual path
        if (fillColor.a > 0.0f) {
            ctx.setFillColor(fillColor);
            ctx.fill();
        }
        
        // Stroke this individual path
        if (strokeColor.a > 0.0f && strokeWidth > 0.0f) {
            ctx.setStrokeColor(strokeColor);
            ctx.setStrokeWidth(strokeWidth);
            ctx.stroke();
        }
    }
}

void SVG::addPathToContext(RenderContext& ctx, NSVGpath* path) const {
    if (!path || path->npts < 2) return;

    // Based on VCV Rack's implementation, we need to determine if this path
    // is a hole or solid shape using ray casting algorithm
    
    // Move to first point
    ctx.moveTo({path->pts[0], path->pts[1]});

    // Draw cubic bezier curves
    for (int i = 1; i < path->npts - 1; i += 3) {
        if (i + 2 < path->npts) {
            ctx.bezierTo(
                {path->pts[i*2], path->pts[i*2+1]},         // control point 1
                {path->pts[(i+1)*2], path->pts[(i+1)*2+1]}, // control point 2
                {path->pts[(i+2)*2], path->pts[(i+2)*2+1]}  // end point
            );
        }
    }

    // Close path if it's closed
    if (path->closed) {
        ctx.closePath();
    }
}

bool SVG::isPathSolid(NSVGpath* path) const {
    if (!path || path->npts < 2) return true;
    
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
    
    // Positive area = counter-clockwise = solid shape
    // Negative area = clockwise = hole
    return area > 0.0f;
}

bool SVG::isPathHole(NSVGpath* path, NSVGpath* allPaths) const {
    if (!path || path->npts < 2) return false;
    
    // Based on VCV Rack's ray casting algorithm
    // Draw a line from a point on this path to a point outside the boundary
    // and count crossings with other paths. Even crossings = solid, odd = hole
    
    float p0x = path->pts[0];
    float p0y = path->pts[1];
    float p1x = path->bounds[0] - 1.0f;  // Point outside bounds
    float p1y = path->bounds[1] - 1.0f;
    
    int crossings = 0;
    
    // Iterate all paths in the shape
    for (NSVGpath* path2 = allPaths; path2; path2 = path2->next) {
        if (path2 == path) continue;
        if (path2->npts < 4) continue;
        
        // Iterate all line segments in path2 - more closely matching VCV Rack
        for (int i = 1; i < path2->npts + 3; i += 3) {
            float* p = &path2->pts[2 * i];
            
            // Previous point
            float p2x = p[-2];
            float p2y = p[-1];
            
            // Current point
            float p3x, p3y;
            if (i < path2->npts) {
                p3x = p[4];
                p3y = p[5];
            } else {
                p3x = path2->pts[0];
                p3y = path2->pts[1];
            }
            
            // Check if line segments intersect
            float crossing = getLineCrossing(p0x, p0y, p1x, p1y, p2x, p2y, p3x, p3y);
            float crossing2 = getLineCrossing(p2x, p2y, p3x, p3y, p0x, p0y, p1x, p1y);
            
            if (0.0f <= crossing && crossing < 1.0f && 0.0f <= crossing2) {
                crossings++;
            }
        }
    }
    
    // Even crossings = solid, odd crossings = hole
    return (crossings % 2) == 1;
}

float SVG::getLineCrossing(float p0x, float p0y, float p1x, float p1y, 
                          float p2x, float p2y, float p3x, float p3y) const {
    // Based on VCV Rack's implementation
    float bx = p2x - p0x;
    float by = p2y - p0y;
    float dx = p1x - p0x;
    float dy = p1y - p0y;
    float ex = p3x - p2x;
    float ey = p3y - p2y;
    
    float m = dx * ey - dy * ex;
    
    // Check if lines are parallel
    if (std::abs(m) < 1e-6f) return NAN;
    
    return -(dx * by - dy * bx) / m;
}

float SVG::calculatePathArea(NSVGpath* path) const {
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

Color SVG::nsvgColorToFluxColor(unsigned int color) const {
    // NanoSVG uses BGR format: NSVG_RGB(r, g, b) = r | (g << 8) | (b << 16)
    // So the format is 0x00BBGGRR
    float r = (color & 0xFF) / 255.0f;           // bits 0-7
    float g = ((color >> 8) & 0xFF) / 255.0f;    // bits 8-15
    float b = ((color >> 16) & 0xFF) / 255.0f;   // bits 16-23
    float a = 1.0f;  // Alpha is stored separately in NanoSVG (opacity, etc.)

    return Color(r, g, b, a);
}

float SVG::parseFloat(const std::string& str) const {
    try {
        return std::stof(str);
    } catch (...) {
        return 0.0f;
    }
}

void SVG::drawCheckerboardBackground(RenderContext& ctx, const Rect& bounds) const {
    // Draw checkerboard pattern
    const int squareSize = 20;
    const Color lightGray = Color(0.9f, 0.9f, 0.9f, 1.0f);
    const Color darkGray = Color(0.7f, 0.7f, 0.7f, 1.0f);
    
    for (int y = 0; y < bounds.height; y += squareSize) {
        for (int x = 0; x < bounds.width; x += squareSize) {
            bool isEven = ((x / squareSize) + (y / squareSize)) % 2 == 0;
            Color color = isEven ? lightGray : darkGray;
            ctx.drawRect({static_cast<float>(x) + bounds.x, static_cast<float>(y) + bounds.y, static_cast<float>(squareSize), static_cast<float>(squareSize)}, color);
        }
    }
}

} // namespace flux