#include <Flux/Animation/Easing.hpp>
#include <algorithm>

namespace flux {

static float easeInQuadFn(float t) { return t * t; }
static float easeOutQuadFn(float t) { return t * (2.0f - t); }
static float easeInOutQuadFn(float t) {
    return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
}

static float easeInCubicFn(float t) { return t * t * t; }
static float easeOutCubicFn(float t) { float u = t - 1.0f; return u * u * u + 1.0f; }
static float easeInOutCubicFn(float t) {
    return t < 0.5f
        ? 4.0f * t * t * t
        : 1.0f + (t - 1.0f) * (2.0f * t - 2.0f) * (2.0f * t - 2.0f);
}

// CSS standard cubic-bezier approximations
static float easeInFn(float t) {
    return t * t * (3.0f - 2.0f * t) * 0.42f + t * t * t * 0.58f;
}

static float easeOutFn(float t) {
    return 1.0f - easeInFn(1.0f - t);
}

static float easeInOutFn(float t) {
    return t * t * (3.0f - 2.0f * t);
}

float evaluateEasing(EasingCurve curve, float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    switch (curve) {
        case EasingCurve::linear:       return t;
        case EasingCurve::easeIn:       return easeInFn(t);
        case EasingCurve::easeOut:      return easeOutFn(t);
        case EasingCurve::easeInOut:    return easeInOutFn(t);
        case EasingCurve::easeInQuad:   return easeInQuadFn(t);
        case EasingCurve::easeOutQuad:  return easeOutQuadFn(t);
        case EasingCurve::easeInOutQuad: return easeInOutQuadFn(t);
        case EasingCurve::easeInCubic:  return easeInCubicFn(t);
        case EasingCurve::easeOutCubic: return easeOutCubicFn(t);
        case EasingCurve::easeInOutCubic: return easeInOutCubicFn(t);
    }
    return t;
}

} // namespace flux
