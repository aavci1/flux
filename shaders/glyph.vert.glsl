#version 450

// Per-vertex: unit quad corners
layout(location = 0) in vec2 inPos;

// Per-instance: glyph placement
layout(location = 1) in vec4 inScreenRect;  // x, y, width, height (screen pixels)
layout(location = 2) in vec4 inUVRect;      // u0, v0, u1, v1 (atlas coords)
layout(location = 3) in vec4 inColor;       // text color (RGBA)
layout(location = 4) in vec2 inViewport;    // viewport width, height
layout(location = 5) in vec4 inRotationPad; // .x = radians

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec4 fragColor;

void main() {
    vec2 uv01 = (inPos + 1.0) * 0.5;  // [0,1]

    vec2 center = inScreenRect.xy + inScreenRect.zw * 0.5;
    vec2 localFromCenter = (uv01 - vec2(0.5)) * inScreenRect.zw;
    float cr = cos(inRotationPad.x);
    float sr = sin(inRotationPad.x);
    vec2 rotated = vec2(localFromCenter.x * cr - localFromCenter.y * sr,
                        localFromCenter.x * sr + localFromCenter.y * cr);
    vec2 screenPos = center + rotated;
    vec2 ndc = (screenPos / inViewport) * 2.0 - 1.0;
    ndc.y = -ndc.y;
    gl_Position = vec4(ndc, 0.0, 1.0);

    fragUV = mix(inUVRect.xy, inUVRect.zw, uv01);
    fragColor = inColor;
}
