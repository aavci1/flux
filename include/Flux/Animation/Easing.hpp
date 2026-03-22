#pragma once

#include <cmath>
#include <optional>

namespace flux {

enum class EasingCurve {
    linear,
    easeIn,
    easeOut,
    easeInOut,
    easeInQuad,
    easeOutQuad,
    easeInOutQuad,
    easeInCubic,
    easeOutCubic,
    easeInOutCubic,
};

struct SpringParams {
    float stiffness = 170.0f;
    float damping = 26.0f;
    float mass = 1.0f;
};

float evaluateEasing(EasingCurve curve, float t);

} // namespace flux
