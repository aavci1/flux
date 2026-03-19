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

    float fillCoverage = 1.0 - smoothstep(-0.75, 0.75, d);

    float strokeCoverage = 0.0;
    if (fragStrokeWidth > 0.0) {
        float sd = abs(d) - fragStrokeWidth * 0.5;
        strokeCoverage = 1.0 - smoothstep(-0.75, 0.75, sd);
    }

    // Output straight-alpha color to match pipeline blend state.
    float fillAlpha = fragFillColor.a * fillCoverage;
    float strokeAlpha = fragStrokeColor.a * strokeCoverage * (1.0 - fillCoverage);
    float outAlpha = (fillAlpha + strokeAlpha) * fragOpacity;

    if (outAlpha < 0.001) discard;

    vec3 premul = fragFillColor.rgb * fillAlpha + fragStrokeColor.rgb * strokeAlpha;
    vec3 outRgb = (outAlpha > 0.0) ? (premul / max(fillAlpha + strokeAlpha, 1e-6)) : vec3(0.0);
    outColor = vec4(outRgb, outAlpha);
}
