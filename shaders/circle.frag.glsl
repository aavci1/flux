#version 450

layout(location = 0) in vec2 fragLocalPos;
layout(location = 1) in vec2 fragHalfSize;
layout(location = 2) in vec4 fragCorners;  // unused for circle
layout(location = 3) in vec4 fragFillColor;
layout(location = 4) in vec4 fragStrokeColor;
layout(location = 5) in float fragStrokeWidth;
layout(location = 6) in float fragOpacity;

layout(location = 0) out vec4 outColor;

void main() {
    float radius = fragHalfSize.x;
    float d = length(fragLocalPos) - radius;

    float fillCoverage = 1.0 - smoothstep(-0.75, 0.75, d);

    float strokeCoverage = 0.0;
    if (fragStrokeWidth > 0.0) {
        float sd = abs(d) - fragStrokeWidth * 0.5;
        strokeCoverage = 1.0 - smoothstep(-0.75, 0.75, sd);
    }

    float fillA = fragFillColor.a * fillCoverage;
    float strokeA = fragStrokeColor.a * strokeCoverage;
    vec4 fillP = vec4(fragFillColor.rgb * fillA, fillA);
    vec4 strokeP = vec4(fragStrokeColor.rgb * strokeA, strokeA);
    vec4 blended = strokeP + fillP * (1.0 - strokeP.a);
    float outA = blended.a * fragOpacity;
    if (outA < 0.001) discard;
    vec3 outRgb = blended.rgb / max(blended.a, 1e-6);
    outColor = vec4(outRgb, outA);
}
