#pragma once

#include <Flux/Graphics/Path.hpp>
#include <Flux/Core/Types.hpp>
#include <vector>
#include <cstdint>

namespace flux {

struct PathVertex {
    float x, y;
    float color[4];
    float viewport[2];
};
static_assert(sizeof(PathVertex) == 32);

struct TessellatedPath {
    std::vector<PathVertex> vertices;
};

class PathFlattener {
public:
    static std::vector<Point> flatten(const Path& path, float tolerance = 0.5f);

    /** Flatten path into one polyline per subpath (each moveTo starts a new subpath). */
    static std::vector<std::vector<Point>> flattenSubpaths(const Path& path, float tolerance = 0.5f);

    static TessellatedPath tessellateFill(const std::vector<Point>& polyline,
                                           const Color& color,
                                           float vpW, float vpH);

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
