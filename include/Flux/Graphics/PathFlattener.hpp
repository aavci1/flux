#pragma once

#include <Flux/Graphics/Path.hpp>
#include <Flux/Core/Types.hpp>
#include <vector>
#include <cstdint>

namespace flux {

/** Vertex layout for GPU path rendering (matches path shader). */
struct PathVertex {
    float x, y;
    float color[4];
    float viewport[2];
};
static_assert(sizeof(PathVertex) == 32);

struct TessellatedPath {
    std::vector<PathVertex> vertices;
};

/**
 * Converts Path commands to polylines (adaptive Bezier subdivision) and tessellates
 * fill/stroke for the GPU path pipeline (libtess2 + stroke expansion).
 */
class PathFlattener {
public:
    static std::vector<Point> flatten(const Path& path, float tolerance = 0.5f);

    /** One polyline per subpath (each moveTo starts a new subpath). */
    static std::vector<std::vector<Point>> flattenSubpaths(const Path& path, float tolerance = 0.5f);

    static TessellatedPath tessellateFill(const std::vector<Point>& polyline,
                                          const Color& color,
                                          float vpW, float vpH);

    /**
     * Tessellate multiple closed contours as one fill (holes, even-odd compound paths).
     * @param tessWindingRule libtess2 rule, e.g. TESS_WINDING_ODD or TESS_WINDING_NONZERO.
     */
    static TessellatedPath tessellateFillContours(const std::vector<std::vector<Point>>& contours,
                                                  const Color& color,
                                                  float vpW, float vpH,
                                                  int tessWindingRule);

    static TessellatedPath tessellateStroke(const std::vector<Point>& polyline,
                                            float strokeWidth,
                                            const Color& color,
                                            float vpW, float vpH);

private:
    static void flattenCubic(std::vector<Point>& out,
                             float x0, float y0, float x1, float y1,
                             float x2, float y2, float x3, float y3,
                             float tol, int depth);
    static void flattenQuad(std::vector<Point>& out,
                            float x0, float y0, float cx, float cy,
                            float x1, float y1, float tol, int depth);
};

} // namespace flux
