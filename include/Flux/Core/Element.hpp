#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Animation/AnimationEngine.hpp>
#include <Flux/Animation/Animatable.hpp>
#include <any>
#include <array>
#include <memory>
#include <vector>
#include <string>

namespace flux {

class View;
struct LayoutNode;
struct Animation;

enum class AnimPropID : uint8_t {
    Opacity = 0,
    BackgroundColor,
    BorderColor,
    BorderWidth,
    CornerRadius,
    Rotation,
    ScaleX,
    ScaleY,
    Offset,
    Padding,
    Bounds,

    // Ad-hoc slots for view-specific implicit animations (hover effects, etc.)
    Custom0,
    Custom1,
    Custom2,
    Custom3,

    Count
};

static constexpr size_t kAnimPropCount = static_cast<size_t>(AnimPropID::Count);

class Element {
public:
    std::string typeName;
    std::string key;
    size_t structuralIndex = 0;

    std::unique_ptr<View> description;
    std::vector<std::unique_ptr<Element>> children;
    Element* parent = nullptr;

    bool isMounted = false;
    bool bodyDirty = true;
    bool layoutDirty = true;
    bool subtreeDirty = true;

    Rect cachedBounds = {0, 0, 0, 0};
    Rect lastConstraints = {0, 0, 0, 0};

    // Monotonic version bumped when this element's render output changes.
    // Used by CommandCompiler to skip recompilation of unchanged elements.
    uint64_t renderVersion_ = 0;
    // Max of renderVersion_ across this element and all descendants.
    uint64_t subtreeRenderVersion_ = 0;

    std::array<std::unique_ptr<AnimationStateBase>, kAnimPropCount> activeAnimations{};
    std::array<std::any, kAnimPropCount> animSnapshots_{};

    bool hasValidLayout() const {
        return !layoutDirty && cachedBounds.width > 0;
    }

    bool hasActiveAnimations() const {
        for (auto& a : activeAnimations) if (a) return true;
        return false;
    }

    size_t activeAnimationCount() const {
        size_t n = 0;
        for (auto& a : activeAnimations) if (a) ++n;
        return n;
    }

    template<typename T>
    T getAnimatedValue(AnimPropID prop, T fallback) const;

    template<Animatable T>
    void reconcileProperty(AnimPropID prop, T newValue, const Animation* config);

    template<Animatable T>
    T animateValue(AnimPropID prop, T targetValue);

    void markDirty();

    Element();
    ~Element();

    Element(const Element&) = delete;
    Element& operator=(const Element&) = delete;
    Element(Element&&) noexcept;
    Element& operator=(Element&&) noexcept;

    static std::unique_ptr<Element> buildTree(const LayoutNode& node, size_t index = 0);

    void reconcile(const LayoutNode& newNode);

    Element* findByFocusKey(const std::string& key);

    void bumpRenderVersion();

private:
    static uint64_t sNextRenderVersion_;

    void reconcileChildren(const std::vector<LayoutNode>& newChildren);
    void mountSubtree();
    void unmountSubtree();
    void reconcileAnimations(const View& newView, const Rect& newBounds);
};

template<typename T>
T Element::getAnimatedValue(AnimPropID prop, T fallback) const {
    auto& slot = activeAnimations[static_cast<size_t>(prop)];
    if (slot) {
        auto* state = dynamic_cast<const AnimationState<T>*>(slot.get());
        if (state) return state->current;
    }
    return fallback;
}

template<Animatable T>
void Element::reconcileProperty(AnimPropID prop, T newValue, const Animation* config) {
    size_t idx = static_cast<size_t>(prop);
    auto& snapshot = animSnapshots_[idx];
    if (snapshot.has_value()) {
        try {
            T prev = std::any_cast<T>(snapshot);
            bool changed = false;
            if constexpr (requires(const T& a, const T& b) { a != b; }) {
                changed = (prev != newValue);
            } else {
                changed = true;
            }
            if (changed && config) {
                T fromValue = prev;
                auto& animSlot = activeAnimations[idx];
                if (animSlot) {
                    auto* existing = dynamic_cast<AnimationState<T>*>(animSlot.get());
                    if (existing) fromValue = existing->current;
                }
                activeAnimations[idx] =
                    std::make_unique<AnimationState<T>>(fromValue, newValue, *config);
            }
        } catch (const std::bad_any_cast&) {}
    }
    snapshot = newValue;
}

template<Animatable T>
T Element::animateValue(AnimPropID prop, T targetValue) {
    static const Animation kImplicit = Animation::defaultImplicit();
    bool hadAnimations = hasActiveAnimations();
    reconcileProperty<T>(prop, targetValue, &kImplicit);
    if (!hadAnimations && hasActiveAnimations()) {
        AnimationEngine::instance().registerElement(this);
    }
    return getAnimatedValue<T>(prop, targetValue);
}

} // namespace flux
