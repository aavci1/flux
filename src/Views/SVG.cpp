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
    std::cout << "[SVG] Shape paths: " << shape->paths << std::endl;

    // Convert NanoSVG color to Flux color
    Color fillColor = Colors::transparent; //nsvgColorToFluxColor(shape->fill.color);
    Color strokeColor = Colors::black; //nsvgColorToFluxColor(shape->stroke.color);
    float strokeWidth = 1.0f; //shape->strokeWidth;

    std::cout << "[SVG] Fill color: " << fillColor.r << ", " << fillColor.g << ", " << fillColor.b << ", " << fillColor.a << std::endl;


    // Apply opacity
    fillColor.a *= shape->opacity;
    strokeColor.a *= shape->opacity;

    // Render all paths in this shape
    for (NSVGpath* path = shape->paths; path != nullptr; path = path->next) {
        std::cout << "[SVG] Rendering path with " << path->npts << " points" << std::endl;
        renderPath(ctx, path, bounds, scale, fillColor, strokeColor, strokeWidth);
    }
}

void SVG::renderPath(RenderContext& ctx, NSVGpath* path, const Rect& /* bounds */, float /* scale */,
                     const Color& fillColor, const Color& strokeColor, float strokeWidth) const {
    if (!path || path->npts < 2) return;

    // Use generic path building methods to render cubic bezier curves directly
    ctx.beginPath();

    // Move to first point
    ctx.moveTo({path->pts[0], path->pts[1]});

    // Draw cubic bezier curves using the generic bezierTo method
    for (int i = 1; i < path->npts - 1; i += 3) {
        if (i + 5 < path->npts * 2) {
            ctx.bezierTo(
                {path->pts[i*2], path->pts[i*2+1]},      // c1
                {path->pts[(i+1)*2], path->pts[(i+1)*2+1]}, // c2
                {path->pts[(i+2)*2], path->pts[(i+2)*2+1]}  // end
            );
        }
    }

    // Close path if it's closed
    if (path->closed) {
        ctx.closePath();
    }

    // Fill and stroke the path
    if (fillColor.a > 0.0f) {
        ctx.setFillColor(fillColor);
        ctx.fill();
    }

    if (strokeColor.a > 0.0f && strokeWidth > 0.0f) {
        ctx.setStrokeColor(strokeColor);
        ctx.setStrokeWidth(strokeWidth);
        ctx.stroke();
    }
}

Color SVG::nsvgColorToFluxColor(unsigned int color) const {
    // NanoSVG uses RGBA format, but there's a bug where green and blue channels are swapped
    // We need to swap them back to get the correct colors
    float r = ((color >> 24) & 0xFF) / 255.0f;
    float g = ((color >> 8) & 0xFF) / 255.0f;   // Swap: use blue position for green
    float b = ((color >> 16) & 0xFF) / 255.0f;  // Swap: use green position for blue
    float a = (color & 0xFF) / 255.0f;

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