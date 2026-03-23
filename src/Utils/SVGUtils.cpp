#include <Flux/Utils/SVGUtils.hpp>

#define NANOSVG_IMPLEMENTATION
#include <nanosvg.h>

namespace flux {

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

Color parseColor(unsigned int color) {
    // NanoSVG uses BGR format: NSVG_RGB(r, g, b) = r | (g << 8) | (b << 16)
    // So the format is 0x00BBGGRR
    float r = (color & 0xFF) / 255.0f;           // bits 0-7
    float g = ((color >> 8) & 0xFF) / 255.0f;    // bits 8-15
    float b = ((color >> 16) & 0xFF) / 255.0f;   // bits 16-23
    float a = ((color >> 24) & 0xFF) / 255.0f;   // bits 24-31

    return Color(r, g, b, a);
}

FillStyle parsePaint(const NSVGpaint& paint) {
    switch (paint.type) {
        case NSVG_PAINT_NONE:
            return FillStyle::none();

        case NSVG_PAINT_COLOR: {
            Color color = parseColor(paint.color);
            return FillStyle::solid(color);
        }

        case NSVG_PAINT_LINEAR_GRADIENT: {
            if (paint.gradient && paint.gradient->nstops >= 2) {
                Color sc = parseColor(paint.gradient->stops[0].color);
                Color ec = parseColor(paint.gradient->stops[paint.gradient->nstops - 1].color);
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
                Color sc = parseColor(paint.gradient->stops[0].color);
                Color ec = parseColor(paint.gradient->stops[paint.gradient->nstops - 1].color);
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

SVGPath parseShape(NSVGshape* shape) {
    if (!shape) {
        return SVGPath();
    }

    // Convert NanoSVG paint (color or gradient) to FillStyle
    FillStyle fillStyle = parsePaint(shape->fill);
    fillStyle.fillRule = (shape->fillRule == NSVG_FILLRULE_EVENODD) ? FillStyle::FillRule::EvenOdd
                                                                    : FillStyle::FillRule::NonZero;

    // Convert stroke color
    Color strokeColor = parseColor(shape->stroke.color);
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

    // return the result
    return SVGPath {
        .path = path,
        .fillStyle = fillStyle,
        .strokeStyle = StrokeStyle::solid(strokeColor, strokeWidth),
        .opacity = shape->opacity
    };
}

SVGData parseSVG(const std::string& svg) {
    SVGData result;

    if (svg.empty()) {
        return result;
    }

    // NanoSVG mutates the buffer while parsing; use a writable copy.
    std::string mutableSvg = svg;

    // Parse SVG using NanoSVG
    NSVGimage* image = nsvgParse(mutableSvg.data(), "px", 96.0f);
    if (!image) {
        return result;
    }

    // Store original dimensions
    result.originalWidth = static_cast<float>(image->width);
    result.originalHeight = static_cast<float>(image->height);

    // Convert all shapes to Path structures during parsing
    for (NSVGshape* shape = image->shapes; shape != nullptr; shape = shape->next) {
        result.paths.push_back(parseShape(shape));
    }

    // Clean up NanoSVG data
    nsvgDelete(image);

    // return the result
    return result;
}

}