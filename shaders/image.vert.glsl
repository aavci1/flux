#version 450

layout(location = 0) in vec2 inPos;

layout(location = 1) in vec4 inScreenRect;
layout(location = 2) in vec4 inUVRect;
layout(location = 3) in vec4 inTint;
layout(location = 4) in vec2 inViewport;

layout(location = 0) out vec2 fragUV;
layout(location = 1) out vec4 fragTint;

void main() {
    vec2 uv01 = (inPos + 1.0) * 0.5;
    vec2 screenPos = inScreenRect.xy + uv01 * inScreenRect.zw;
    vec2 ndc = (screenPos / inViewport) * 2.0 - 1.0;
    ndc.y = -ndc.y;
    gl_Position = vec4(ndc, 0.0, 1.0);

    fragUV = mix(inUVRect.xy, inUVRect.zw, uv01);
    fragTint = inTint;
}
