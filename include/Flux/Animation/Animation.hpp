#pragma once

#include <Flux/Animation/Easing.hpp>
#include <optional>
#include <functional>

namespace flux {

struct Animation {
    float duration = 0.3f;
    EasingCurve curve = EasingCurve::easeInOut;
    float delay = 0.0f;
    std::optional<SpringParams> spring;

    static Animation none() {
        return Animation{.duration = 0.0f, .curve = EasingCurve::linear};
    }

    static Animation defaultImplicit() {
        return Animation{.duration = 0.2f, .curve = EasingCurve::easeInOut};
    }

    static Animation linear(float duration) {
        return Animation{.duration = duration, .curve = EasingCurve::linear};
    }

    static Animation easeIn(float duration) {
        return Animation{.duration = duration, .curve = EasingCurve::easeIn};
    }

    static Animation easeOut(float duration) {
        return Animation{.duration = duration, .curve = EasingCurve::easeOut};
    }

    static Animation easeInOut(float duration) {
        return Animation{.duration = duration, .curve = EasingCurve::easeInOut};
    }

    static Animation Spring(float stiffness = 170.0f, float damping = 26.0f) {
        return Animation{
            .duration = 0.0f,
            .curve = EasingCurve::linear,
            .spring = SpringParams{.stiffness = stiffness, .damping = damping}
        };
    }

    bool isNone() const { return duration <= 0.0f && !spring.has_value(); }
    bool isSpring() const { return spring.has_value(); }
};

void setCurrentAnimationContext(std::optional<Animation> anim);
std::optional<Animation> currentAnimationContext();
std::optional<Animation> consumePendingAnimationConfig();

template<typename F>
void withAnimation(Animation anim, F&& fn) {
    setCurrentAnimationContext(anim);
    std::forward<F>(fn)();
    setCurrentAnimationContext(std::nullopt);
}

} // namespace flux
