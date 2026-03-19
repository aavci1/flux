#version 450

// Per-vertex: unit quad corners
layout(location = 0) in vec2 inPos;        // [-1,-1] to [1,1]

// Per-instance data (already in NDC-ready screen coords)
layout(location = 1) in vec4 inRect;       // x, y, width, height (screen pixels)
layout(location = 2) in vec4 inCorners;    // topLeft, topRight, bottomRight, bottomLeft radius
layout(location = 3) in vec4 inFillColor;
layout(location = 4) in vec4 inStrokeColor;
layout(location = 5) in vec2 inStrokeOpacity; // x=strokeWidth, y=opacity
layout(location = 6) in vec2 inViewport;   // viewport width, height

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

    vec2 screenPos = center + inPos * paddedHalf;
    vec2 ndc = (screenPos / inViewport) * 2.0 - 1.0;
    ndc.y = -ndc.y;
    gl_Position = vec4(ndc, 0.0, 1.0);

    fragLocalPos = inPos * paddedHalf;
    fragHalfSize = halfSize;
    fragCorners = inCorners;
    fragFillColor = inFillColor;
    fragStrokeColor = inStrokeColor;
    fragStrokeWidth = inStrokeOpacity.x;
    fragOpacity = inStrokeOpacity.y;
}
