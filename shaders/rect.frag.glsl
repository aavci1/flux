#version 450

layout(location = 0) in vec2 fragLocalPos;
layout(location = 1) in vec2 fragHalfSize;
layout(location = 2) in vec4 fragCorners;
layout(location = 3) in vec4 fragFillColor;
layout(location = 4) in vec4 fragStrokeColor;
layout(location = 5) in float fragStrokeWidth;
layout(location = 6) in float fragOpacity;

layout(location = 0) out vec4 outColor;

float roundedRectSDF(vec2 p, vec2 halfSize, vec4 corners) {
    // Select corner radius based on quadrant
    float r = (p.x > 0.0)
        ? ((p.y > 0.0) ? corners.z : corners.y)   // right: bottomRight or topRight
        : ((p.y > 0.0) ? corners.w : corners.x);   // left:  bottomLeft  or topLeft

    vec2 q = abs(p) - halfSize + r;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r;
}

void main() {
    float d = roundedRectSDF(fragLocalPos, fragHalfSize, fragCorners);

    float fillAlpha = 1.0 - smoothstep(-0.75, 0.75, d);

    float strokeAlpha = 0.0;
    if (fragStrokeWidth > 0.0) {
        float sd = abs(d) - fragStrokeWidth * 0.5;
        strokeAlpha = 1.0 - smoothstep(-0.75, 0.75, sd);
    }

    vec4 fill = fragFillColor * fillAlpha;
    vec4 stroke = fragStrokeColor * strokeAlpha * (1.0 - fillAlpha);
    vec4 color = fill + stroke;
    color.a *= fragOpacity;

    if (color.a < 0.001) discard;
    outColor = color;
}
