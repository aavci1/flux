#include <Flux/Graphics/PathFlattener.hpp>
#include <tesselator.h>
#include <algorithm>
#include <cmath>

namespace flux {

namespace {

constexpr float kTwoPi = 6.283185307179586476925286766559f;
constexpr int kMinArcSegments = 8;
constexpr float kDegenerateEdge = 1e-6f;
constexpr float kDegenerateEdgeStroke = 0.001f;
constexpr float kDupPointEpsSq = 1e-4f;

bool nearlySamePoint(const Point& a, const Point& b) {
    float dx = a.x - b.x, dy = a.y - b.y;
    return dx * dx + dy * dy < kDupPointEpsSq;
}

void appendArcSamples(std::vector<Point>& out, float cx, float cy, float r,
                      float a0, float sweep) {
    int segments = std::max(kMinArcSegments,
                            static_cast<int>(std::ceil(std::abs(sweep) * r * 0.25f)));
    for (int i = 1; i <= segments; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(segments);
        float a = a0 + sweep * t;
        out.push_back({cx + std::cos(a) * r, cy + std::sin(a) * r});
    }
}

/** Signed sweep from start to end angle matching canvas-style arc direction. */
float sweepForArc(float a0, float a1, bool clockwise) {
    float d = a1 - a0;
    if (!clockwise) {
        while (d < 0) d += kTwoPi;
        while (d >= kTwoPi) d -= kTwoPi;
        if (std::abs(d) < 1e-7f) d = kTwoPi;
    } else {
        while (d > 0) d -= kTwoPi;
        while (d <= -kTwoPi) d += kTwoPi;
        if (std::abs(d) < 1e-7f) d = -kTwoPi;
    }
    return d;
}

/**
 * Canvas-style arcTo: tangent to (current→p1) and (p1→p2), with given corner radius.
 * Updates current point to the end of the arc (on the ray toward p2).
 */
void flattenArcTo(std::vector<Point>& current, float& curX, float& curY,
                  float x1, float y1, float x2, float y2, float radius) {
    if (radius <= 0.f) {
        curX = x1;
        curY = y1;
        current.push_back({x1, y1});
        return;
    }
    float x0 = curX, y0 = curY;
    float ux0 = x0 - x1, uy0 = y0 - y1;
    float ux1 = x2 - x1, uy1 = y2 - y1;
    float len0 = std::sqrt(ux0 * ux0 + uy0 * uy0);
    float len1 = std::sqrt(ux1 * ux1 + uy1 * uy1);
    if (len0 < kDegenerateEdge || len1 < kDegenerateEdge) {
        curX = x1;
        curY = y1;
        current.push_back({x1, y1});
        return;
    }
    ux0 /= len0;
    uy0 /= len0;
    ux1 /= len1;
    uy1 /= len1;
    float cross = ux0 * uy1 - uy0 * ux1;
    float dot = ux0 * ux1 + uy0 * uy1;
    float theta = std::atan2(cross, dot);
    if (std::abs(theta) < 1e-7f) {
        curX = x1;
        curY = y1;
        current.push_back({x1, y1});
        return;
    }
    float half = std::abs(theta) * 0.5f;
    float t = radius / std::tan(half);
    float maxAlong = std::min(len0, len1);
    if (t > maxAlong) {
        t = maxAlong;
        radius = t * std::tan(half);
    }
    float sx = x1 + ux0 * t;
    float sy = y1 + uy0 * t;
    float ex = x1 + ux1 * t;
    float ey = y1 + uy1 * t;

    float dx = curX - sx, dy = curY - sy;
    if (dx * dx + dy * dy > kDupPointEpsSq) {
        current.push_back({sx, sy});
        curX = sx;
        curY = sy;
    }

    float chord = std::hypot(ex - sx, ey - sy);
    if (chord < kDegenerateEdge || radius <= 0.f) {
        curX = ex;
        curY = ey;
        current.push_back({ex, ey});
        return;
    }
    float mx = (sx + ex) * 0.5f, my = (sy + ey) * 0.5f;
    float halfChord = chord * 0.5f;
    float h = std::sqrt(std::max(0.f, radius * radius - halfChord * halfChord));
    float px = -(ey - sy);
    float py = ex - sx;
    float plen = std::sqrt(px * px + py * py);
    if (plen < kDegenerateEdge) {
        curX = ex;
        curY = ey;
        current.push_back({ex, ey});
        return;
    }
    px /= plen;
    py /= plen;
    float vx = x1 - mx, vy = y1 - my;
    float sign = (vx * px + vy * py) >= 0.f ? 1.f : -1.f;
    float cx = mx + px * h * sign;
    float cy = my + py * h * sign;

    float aS = std::atan2(sy - cy, sx - cx);
    float aE = std::atan2(ey - cy, ex - cx);
    float sweep = aE - aS;
    if (theta > 0.f) {
        while (sweep <= 0.f) sweep += kTwoPi;
        while (sweep > kTwoPi) sweep -= kTwoPi;
    } else {
        while (sweep >= 0.f) sweep -= kTwoPi;
        while (sweep < -kTwoPi) sweep += kTwoPi;
    }
    appendArcSamples(current, cx, cy, radius, aS, sweep);
    curX = ex;
    curY = ey;
}

void edgeNormalOpenPoly(const std::vector<Point>& polyline, size_t i, float& nx, float& ny) {
    const size_t n = polyline.size();
    if (i + 1 < n) {
        float dx = polyline[i + 1].x - polyline[i].x;
        float dy = polyline[i + 1].y - polyline[i].y;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < kDegenerateEdgeStroke) {
            nx = 0;
            ny = 1;
        } else {
            nx = -dy / len;
            ny = dx / len;
        }
    } else {
        float dx = polyline[i].x - polyline[i - 1].x;
        float dy = polyline[i].y - polyline[i - 1].y;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < kDegenerateEdgeStroke) {
            nx = 0;
            ny = 1;
        } else {
            nx = -dy / len;
            ny = dx / len;
        }
    }
}

/** Miter join at vertex p with neighbors; left normals of incoming/outgoing edges (open-chain stroke). */
void miterOffset(const Point& pPrev, const Point& p, const Point& pNext, float hw,
                 float& outX, float& outY, float& inX, float& inY) {
    float e0x = p.x - pPrev.x, e0y = p.y - pPrev.y;
    float e1x = pNext.x - p.x, e1y = pNext.y - p.y;
    float len0 = std::sqrt(e0x * e0x + e0y * e0y);
    float len1 = std::sqrt(e1x * e1x + e1y * e1y);
    if (len0 < 1e-6f || len1 < 1e-6f) {
        float nx = (len0 >= 1e-6f) ? (-e0y / len0) : 0.0f;
        float ny = (len0 >= 1e-6f) ? (e0x / len0) : 1.0f;
        outX = p.x + nx * hw;
        outY = p.y + ny * hw;
        inX = p.x - nx * hw;
        inY = p.y - ny * hw;
        return;
    }
    float n0x = -e0y / len0, n0y = e0x / len0;
    float n1x = -e1y / len1, n1y = e1x / len1;
    float mx = n0x + n1x, my = n0y + n1y;
    float mlen = std::sqrt(mx * mx + my * my);
    if (mlen < 1e-6f) {
        outX = p.x + n0x * hw;
        outY = p.y + n0y * hw;
        inX = p.x - n0x * hw;
        inY = p.y - n0y * hw;
        return;
    }
    mx /= mlen;
    my /= mlen;
    float denom = mx * n0x + my * n0y;
    if (std::fabs(denom) < 1e-6f) {
        outX = p.x + n0x * hw;
        outY = p.y + n0y * hw;
        inX = p.x - n0x * hw;
        inY = p.y - n0y * hw;
        return;
    }
    float scale = hw / denom;
    const float maxScale = hw * 8.0f;
    if (scale > maxScale) scale = maxScale;
    if (scale < -maxScale) scale = -maxScale;
    outX = p.x + mx * scale;
    outY = p.y + my * scale;
    inX = p.x - mx * scale;
    inY = p.y - my * scale;
}

TessellatedPath tessellateStrokeClosedRing(const std::vector<Point>& pts, float hw,
                                           const Color& color, float vpW, float vpH) {
    TessellatedPath result;
    const size_t n = pts.size();
    if (n < 3) return result;

    std::vector<Point> expanded;
    expanded.reserve(n * 2 + 2);

    for (size_t i = 0; i < n; i++) {
        const Point& pPrev = pts[(i + n - 1) % n];
        const Point& p = pts[i];
        const Point& pNext = pts[(i + 1) % n];
        float ox, oy, ix, iy;
        miterOffset(pPrev, p, pNext, hw, ox, oy, ix, iy);
        expanded.push_back({ox, oy});
    }
    for (int i = static_cast<int>(n) - 1; i >= 0; i--) {
        const Point& pPrev = pts[(static_cast<size_t>(i) + n - 1) % n];
        const Point& p = pts[static_cast<size_t>(i)];
        const Point& pNext = pts[(static_cast<size_t>(i) + 1) % n];
        float ox, oy, ix, iy;
        miterOffset(pPrev, p, pNext, hw, ox, oy, ix, iy);
        expanded.push_back({ix, iy});
    }
    expanded.push_back(expanded.front());

    return PathFlattener::tessellateFill(expanded, color, vpW, vpH);
}

} // namespace

std::vector<Point> PathFlattener::flatten(const Path& path, float tolerance) {
    auto subpaths = flattenSubpaths(path, tolerance);
    std::vector<Point> out;
    for (const auto& sub : subpaths)
        out.insert(out.end(), sub.begin(), sub.end());
    return out;
}

std::vector<std::vector<Point>> PathFlattener::flattenSubpaths(const Path& path, float tolerance) {
    std::vector<std::vector<Point>> result;
    std::vector<Point> current;
    float curX = 0, curY = 0;
    float startX = 0, startY = 0;

    auto startSubpath = [&](float x, float y) {
        if (!current.empty()) {
            result.push_back(std::move(current));
            current.clear();
        }
        curX = x;
        curY = y;
        startX = x;
        startY = y;
        current.push_back({x, y});
    };

    for (const auto& cmd : path.commands_) {
        switch (cmd.type) {
            case Path::CommandType::SetWinding:
                break;

            case Path::CommandType::MoveTo:
                startSubpath(cmd.data[0], cmd.data[1]);
                break;

            case Path::CommandType::LineTo:
                curX = cmd.data[0];
                curY = cmd.data[1];
                current.push_back({curX, curY});
                break;

            case Path::CommandType::QuadTo: {
                float cx = cmd.data[0], cy = cmd.data[1];
                float ex = cmd.data[2], ey = cmd.data[3];
                flattenQuad(current, curX, curY, cx, cy, ex, ey, tolerance, 0);
                curX = ex;
                curY = ey;
                break;
            }

            case Path::CommandType::BezierTo: {
                float c1x = cmd.data[0], c1y = cmd.data[1];
                float c2x = cmd.data[2], c2y = cmd.data[3];
                float ex = cmd.data[4], ey = cmd.data[5];
                flattenCubic(current, curX, curY, c1x, c1y, c2x, c2y, ex, ey, tolerance, 0);
                curX = ex;
                curY = ey;
                break;
            }

            case Path::CommandType::ArcTo: {
                if (cmd.data.size() < 5) break;
                flattenArcTo(current, curX, curY, cmd.data[0], cmd.data[1], cmd.data[2], cmd.data[3],
                             cmd.data[4]);
                break;
            }

            case Path::CommandType::Arc: {
                if (cmd.data.size() < 6) break;
                float cx = cmd.data[0], cy = cmd.data[1], r = cmd.data[2];
                float a0 = cmd.data[3], a1 = cmd.data[4];
                bool cw = cmd.data[5] > 0.5f;
                if (r <= 0.f) break;

                float sx = cx + std::cos(a0) * r;
                float sy = cy + std::sin(a0) * r;
                if (current.empty()) {
                    startSubpath(sx, sy);
                } else {
                    float dx = curX - sx, dy = curY - sy;
                    if (dx * dx + dy * dy > kDupPointEpsSq) {
                        current.push_back({sx, sy});
                        curX = sx;
                        curY = sy;
                    }
                }
                float sweep = sweepForArc(a0, a1, cw);
                appendArcSamples(current, cx, cy, r, a0, sweep);
                float endA = a0 + sweep;
                curX = cx + std::cos(endA) * r;
                curY = cy + std::sin(endA) * r;
                break;
            }

            case Path::CommandType::Rect: {
                if (cmd.data.size() >= 8) {
                    if (!current.empty()) {
                        result.push_back(std::move(current));
                        current.clear();
                    }
                    Rect r{cmd.data[0], cmd.data[1], cmd.data[2], cmd.data[3]};
                    CornerRadius cr{cmd.data[4], cmd.data[5], cmd.data[6], cmd.data[7]};
                    Path expanded;
                    expanded.rect(r, cr);
                    auto subs = flattenSubpaths(expanded, tolerance);
                    for (auto& sp : subs) {
                        if (!sp.empty())
                            result.push_back(std::move(sp));
                    }
                }
                break;
            }

            case Path::CommandType::Circle: {
                float cx = cmd.data[0], cy = cmd.data[1], r = cmd.data[2];
                int segments = std::max(16, static_cast<int>(r * 2));
                for (int i = 0; i <= segments; i++) {
                    float a = static_cast<float>(i) / static_cast<float>(segments) * kTwoPi;
                    current.push_back({cx + std::cos(a) * r, cy + std::sin(a) * r});
                }
                curX = cx + r;
                curY = cy;
                break;
            }

            case Path::CommandType::Ellipse: {
                float cx = cmd.data[0], cy = cmd.data[1];
                float rx = cmd.data[2], ry = cmd.data[3];
                float maxR = std::max(rx, ry);
                int segments = std::max(16, static_cast<int>(maxR * 2));
                for (int i = 0; i <= segments; i++) {
                    float a = static_cast<float>(i) / static_cast<float>(segments) * kTwoPi;
                    current.push_back({cx + std::cos(a) * rx, cy + std::sin(a) * ry});
                }
                curX = cx + rx;
                curY = cy;
                break;
            }

            case Path::CommandType::Close:
                if (!current.empty()) current.push_back({startX, startY});
                curX = startX;
                curY = startY;
                break;

            default:
                break;
        }
    }
    if (!current.empty())
        result.push_back(std::move(current));
    return result;
}

void PathFlattener::flattenCubic(std::vector<Point>& out, float x0, float y0, float x1, float y1,
                                 float x2, float y2, float x3, float y3, float tol, int depth) {
    if (depth > 10) {
        out.push_back({x3, y3});
        return;
    }
    float dx = x3 - x0, dy = y3 - y0;
    float d = std::abs((x1 - x3) * dy - (y1 - y3) * dx) + std::abs((x2 - x3) * dy - (y2 - y3) * dx);
    if (d * d < tol * (dx * dx + dy * dy)) {
        out.push_back({x3, y3});
        return;
    }
    float x01 = (x0 + x1) * 0.5f, y01 = (y0 + y1) * 0.5f;
    float x12 = (x1 + x2) * 0.5f, y12 = (y1 + y2) * 0.5f;
    float x23 = (x2 + x3) * 0.5f, y23 = (y2 + y3) * 0.5f;
    float x012 = (x01 + x12) * 0.5f, y012 = (y01 + y12) * 0.5f;
    float x123 = (x12 + x23) * 0.5f, y123 = (y12 + y23) * 0.5f;
    float x0123 = (x012 + x123) * 0.5f, y0123 = (y012 + y123) * 0.5f;
    flattenCubic(out, x0, y0, x01, y01, x012, y012, x0123, y0123, tol, depth + 1);
    flattenCubic(out, x0123, y0123, x123, y123, x23, y23, x3, y3, tol, depth + 1);
}

void PathFlattener::flattenQuad(std::vector<Point>& out, float x0, float y0, float cx, float cy,
                                float x1, float y1, float tol, int depth) {
    if (depth > 10) {
        out.push_back({x1, y1});
        return;
    }
    float dx = x1 - x0, dy = y1 - y0;
    float d = std::abs((cx - x1) * dy - (cy - y1) * dx);
    if (d * d < tol * (dx * dx + dy * dy)) {
        out.push_back({x1, y1});
        return;
    }
    float x01 = (x0 + cx) * 0.5f, y01 = (y0 + cy) * 0.5f;
    float xc1 = (cx + x1) * 0.5f, yc1 = (cy + y1) * 0.5f;
    float xm = (x01 + xc1) * 0.5f, ym = (y01 + yc1) * 0.5f;
    flattenQuad(out, x0, y0, x01, y01, xm, ym, tol, depth + 1);
    flattenQuad(out, xm, ym, xc1, yc1, x1, y1, tol, depth + 1);
}

TessellatedPath PathFlattener::tessellateFill(const std::vector<Point>& polyline, const Color& color,
                                              float vpW, float vpH) {
    TessellatedPath result;
    if (polyline.size() < 3) return result;

    TESStesselator* tess = tessNewTess(nullptr);
    if (!tess) return result;

    std::vector<float> coords;
    coords.reserve(polyline.size() * 2);
    for (const auto& p : polyline) {
        coords.push_back(p.x);
        coords.push_back(p.y);
    }

    tessAddContour(tess, 2, coords.data(), sizeof(float) * 2, static_cast<int>(polyline.size()));

    if (!tessTesselate(tess, TESS_WINDING_NONZERO, TESS_POLYGONS, 3, 2, nullptr)) {
        tessDeleteTess(tess);
        return result;
    }

    const float* verts = tessGetVertices(tess);
    const int* elems = tessGetElements(tess);
    int nelems = tessGetElementCount(tess);

    result.vertices.reserve(static_cast<size_t>(nelems) * 3);
    for (int i = 0; i < nelems; i++) {
        for (int j = 0; j < 3; j++) {
            int idx = elems[i * 3 + j];
            if (idx == TESS_UNDEF) continue;
            PathVertex v{};
            v.x = verts[idx * 2];
            v.y = verts[idx * 2 + 1];
            v.color[0] = color.r;
            v.color[1] = color.g;
            v.color[2] = color.b;
            v.color[3] = color.a;
            v.viewport[0] = vpW;
            v.viewport[1] = vpH;
            result.vertices.push_back(v);
        }
    }

    tessDeleteTess(tess);
    return result;
}

TessellatedPath PathFlattener::tessellateStroke(const std::vector<Point>& polyline, float strokeWidth,
                                                const Color& color, float vpW, float vpH) {
    TessellatedPath result;
    if (polyline.size() < 2) return result;

    float hw = strokeWidth * 0.5f;

    std::vector<Point> pts(polyline.begin(), polyline.end());
    bool closed = false;
    if (pts.size() >= 2 && nearlySamePoint(pts.front(), pts.back())) {
        pts.pop_back();
        closed = true;
    }
    if (closed && pts.size() >= 3)
        return tessellateStrokeClosedRing(pts, hw, color, vpW, vpH);

    std::vector<Point> expanded;
    expanded.reserve(polyline.size() * 2 + 2);

    for (size_t i = 0; i < polyline.size(); i++) {
        float nx, ny;
        edgeNormalOpenPoly(polyline, i, nx, ny);
        expanded.push_back({polyline[i].x + nx * hw, polyline[i].y + ny * hw});
    }
    for (int i = static_cast<int>(polyline.size()) - 1; i >= 0; i--) {
        float nx, ny;
        edgeNormalOpenPoly(polyline, static_cast<size_t>(i), nx, ny);
        expanded.push_back({polyline[static_cast<size_t>(i)].x - nx * hw,
                            polyline[static_cast<size_t>(i)].y - ny * hw});
    }
    expanded.push_back(expanded.front());

    return tessellateFill(expanded, color, vpW, vpH);
}

} // namespace flux
