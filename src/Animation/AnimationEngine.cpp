#include <Flux/Animation/AnimationEngine.hpp>
#include <Flux/Animation/Animation.hpp>
#include <Flux/Core/Element.hpp>
#include <Flux/Core/Runtime.hpp>
#include <vector>

namespace flux {

static thread_local std::optional<Animation> g_animationContext;
static std::optional<Animation> g_pendingAnimationConfig;

void setCurrentAnimationContext(std::optional<Animation> anim) {
    g_animationContext = anim;
    if (anim.has_value()) {
        g_pendingAnimationConfig = anim;
    }
}

std::optional<Animation> currentAnimationContext() {
    return g_animationContext;
}

std::optional<Animation> consumePendingAnimationConfig() {
    auto result = g_pendingAnimationConfig;
    g_pendingAnimationConfig = std::nullopt;
    return result;
}

AnimationEngine& AnimationEngine::instance() {
    return Runtime::instance().animationEngine();
}

void AnimationEngine::registerElement(Element* el) {
    elements_.insert(el);
}

void AnimationEngine::unregisterElement(Element* el) {
    elements_.erase(el);
}

void AnimationEngine::tick(float dt) {
    std::vector<Element*> finished;

    for (Element* el : elements_) {
        for (auto& slot : el->activeAnimations) {
            if (!slot) continue;
            bool stillRunning = slot->tick(dt);
            if (!stillRunning) {
                slot.reset();
            }
        }
        if (!el->hasActiveAnimations()) {
            finished.push_back(el);
        }
    }

    for (Element* el : finished) {
        elements_.erase(el);
    }
}

bool AnimationEngine::hasActiveAnimations() const {
    return !elements_.empty();
}

} // namespace flux
