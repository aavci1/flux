#version 450

layout(location = 0) in vec2 inPos;

layout(location = 1) in vec4 inScreenRect;
layout(location = 2) in vec4 inUVRect;
layout(location = 3) in vec4 inTint;
layout(location = 4) in vec2 inViewport;
layout(location = 5) in vec4 inRotationPad;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec4 fragTint;

void main() {
    vec2 uv01 = (inPos + 1.0) * 0.5;
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
    fragTint = inTint;
}
