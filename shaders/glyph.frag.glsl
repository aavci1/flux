#version 450

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec4 fragColor;

layout(binding = 0) uniform sampler2D uAtlas;

layout(location = 0) out vec4 outColor;

void main() {
    float alpha = texture(uAtlas, fragUV).r;
    outColor = vec4(fragColor.rgb, fragColor.a * alpha);
    if (outColor.a < 0.004) discard;
}
