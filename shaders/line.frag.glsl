#version 450

layout(location = 0) in vec2 fragLocalPos;
layout(location = 1) in vec2 fragHalfSize;
layout(location = 2) in vec4 fragCorners;  // xy=lineDir (unused, line is always along X in local space)
layout(location = 3) in vec4 fragFillColor;
layout(location = 4) in vec4 fragStrokeColor; // used as line color
layout(location = 5) in float fragStrokeWidth;
layout(location = 6) in float fragOpacity;

layout(location = 0) out vec4 outColor;

void main() {
    // Line rendered as a capsule SDF: distance to segment from -halfSize.x to +halfSize.x
    float halfLen = fragHalfSize.x;
    float halfW = fragStrokeWidth * 0.5;
    vec2 p = fragLocalPos;
    p.x = clamp(p.x, -halfLen, halfLen);
    float d = length(fragLocalPos - vec2(p.x, 0.0)) - halfW;

    float alpha = 1.0 - smoothstep(-0.75, 0.75, d);
    vec4 color = fragStrokeColor;
    color.a *= alpha * fragOpacity;

    if (color.a < 0.001) discard;
    outColor = color;
}
