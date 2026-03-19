#version 450

// Same layout as sdf_quad; for lines, inCorners.xy = (cos(angle), sin(angle))
layout(location = 0) in vec2 inPos;

layout(location = 1) in vec4 inRect;
layout(location = 2) in vec4 inCorners;    // xy = line direction (cos, sin)
layout(location = 3) in vec4 inFillColor;
layout(location = 4) in vec4 inStrokeColor;
layout(location = 5) in vec2 inStrokeOpacity;
layout(location = 6) in vec2 inViewport;

layout(location = 0) out vec2 fragLocalPos;
layout(location = 1) out vec2 fragHalfSize;
layout(location = 2) out vec4 fragCorners;
layout(location = 3) out vec4 fragFillColor;
layout(location = 4) out vec4 fragStrokeColor;
layout(location = 5) out float fragStrokeWidth;
layout(location = 6) out float fragOpacity;

void main() {
    vec2 halfSize = inRect.zw * 0.5;
    vec2 center = inRect.xy + halfSize;

    float pad = max(inStrokeOpacity.x, 1.0);
    vec2 paddedHalf = halfSize + pad;
    vec2 localOffset = inPos * paddedHalf;

    // Rotate quad so line direction matches instance angle (corners.xy = cos, sin)
    float cosA = inCorners.x;
    float sinA = inCorners.y;
    vec2 rotatedOffset = vec2(localOffset.x * cosA - localOffset.y * sinA,
                              localOffset.x * sinA + localOffset.y * cosA);

    vec2 screenPos = center + rotatedOffset;
    vec2 ndc = (screenPos / inViewport) * 2.0 - 1.0;
    ndc.y = -ndc.y;
    gl_Position = vec4(ndc, 0.0, 1.0);

    // Fragment expects local space with line along X (unchanged)
    fragLocalPos = localOffset;
    fragHalfSize = halfSize;
    fragCorners = inCorners;
    fragFillColor = inFillColor;
    fragStrokeColor = inStrokeColor;
    fragStrokeWidth = inStrokeOpacity.x;
    fragOpacity = inStrokeOpacity.y;
}
