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
