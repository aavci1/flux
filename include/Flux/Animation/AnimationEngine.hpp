#pragma once

#include <Flux/Animation/Animation.hpp>
#include <Flux/Animation/Animatable.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <cmath>

namespace flux {

class Element;

struct AnimationStateBase {
    virtual ~AnimationStateBase() = default;
    virtual bool tick(float dt) = 0;
    virtual bool isComplete() const = 0;
};

template<Animatable T>
struct AnimationState : AnimationStateBase {
    T from;
    T to;
    T current;
    Animation config;
    float elapsed = 0.0f;

    // Spring simulation state (normalized 0..1 displacement)
    float springPosition = 0.0f;
    float springVelocity = 0.0f;

    AnimationState(T from, T to, Animation config)
        : from(from), to(to), current(from), config(config) {}

    bool tick(float dt) override {
        if (config.delay > 0.0f) {
            config.delay -= dt;
            if (config.delay > 0.0f) return true;
            dt = -config.delay;
            config.delay = 0.0f;
        }

        elapsed += dt;

        if (config.isSpring()) {
            auto& sp = *config.spring;
            float omega = std::sqrt(sp.stiffness / sp.mass);
            float zeta = sp.damping / (2.0f * std::sqrt(sp.stiffness * sp.mass));

            // Semi-implicit Euler integration toward target (position=1.0)
            float displacement = springPosition - 1.0f;
            float springForce = -sp.stiffness * displacement;
            float dampingForce = -sp.damping * springVelocity;
            float acceleration = (springForce + dampingForce) / sp.mass;

            springVelocity += acceleration * dt;
            springPosition += springVelocity * dt;

            float t = std::clamp(springPosition, 0.0f, 2.0f);
            current = flux::lerp(from, to, std::clamp(t, 0.0f, 1.0f));

            (void)omega;
            (void)zeta;
            bool settled = std::abs(displacement) < 0.001f
                        && std::abs(springVelocity) < 0.001f;
            if (settled) {
                current = to;
                return false;
            }
            return true;
        }

        float progress = (config.duration > 0.0f)
            ? std::clamp(elapsed / config.duration, 0.0f, 1.0f)
            : 1.0f;
        float easedT = evaluateEasing(config.curve, progress);
        current = flux::lerp(from, to, easedT);

        return progress < 1.0f;
    }

    bool isComplete() const override {
        if (config.isSpring()) {
            float displacement = springPosition - 1.0f;
            return std::abs(displacement) < 0.001f
                && std::abs(springVelocity) < 0.001f;
        }
        return config.duration > 0.0f
            ? elapsed >= config.duration
            : true;
    }
};

class AnimationEngine {
public:
    static AnimationEngine& instance();

    void registerElement(Element* el);
    void unregisterElement(Element* el);

    void tick(float dt);
    bool hasActiveAnimations() const;

    std::unordered_set<Element*>& elements() { return elements_; }

private:
    std::unordered_set<Element*> elements_;
};

} // namespace flux
