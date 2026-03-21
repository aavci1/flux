#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Animation/AnimationEngine.hpp>
#include <Flux/Animation/Animatable.hpp>
#include <any>
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>

namespace flux {

class View;
struct LayoutNode;
struct Animation;

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

    Rect cachedBounds = {0, 0, 0, 0};
    Rect lastConstraints = {0, 0, 0, 0};

    // Active property animations keyed by property name
    std::unordered_map<std::string, std::unique_ptr<AnimationStateBase>> activeAnimations;

    // Type-erased snapshots of the previous property values.
    // reconcileProperty<T>() stores and retrieves from this map,
    // so adding a new animated property only requires one call.
    std::unordered_map<std::string, std::any> animSnapshots_;

    bool hasValidLayout() const {
        return !layoutDirty && cachedBounds.width > 0;
    }

    bool hasActiveAnimations() const { return !activeAnimations.empty(); }

    template<typename T>
    T getAnimatedValue(const std::string& propName, T fallback) const;

    // Compare newValue against the stored snapshot for propName.
    // If changed and config is non-null, create/replace an AnimationState.
    // Always updates the snapshot to newValue.
    template<Animatable T>
    void reconcileProperty(const char* propName, T newValue, const Animation* config);

    // Render-time convenience: reconcile + return the current animated value.
    // Ideal for computed values like hover/pressed color adjustments that
    // bypass the property system and are only known at render time.
    template<Animatable T>
    T animateValue(const char* propName, T targetValue);

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

private:
    void reconcileChildren(const std::vector<LayoutNode>& newChildren);
    void mountSubtree();
    void unmountSubtree();
    void reconcileAnimations(const View& newView, const Rect& newBounds);
};

template<typename T>
T Element::getAnimatedValue(const std::string& propName, T fallback) const {
    auto it = activeAnimations.find(propName);
    if (it != activeAnimations.end()) {
        auto* state = dynamic_cast<const AnimationState<T>*>(it->second.get());
        if (state) return state->current;
    }
    return fallback;
}

template<Animatable T>
void Element::reconcileProperty(const char* propName, T newValue, const Animation* config) {
    auto it = animSnapshots_.find(propName);
    if (it != animSnapshots_.end()) {
        try {
            T prev = std::any_cast<T>(it->second);
            bool changed = false;
            if constexpr (requires(const T& a, const T& b) { a != b; }) {
                changed = (prev != newValue);
            } else {
                changed = true;
            }
            if (changed && config) {
                T fromValue = prev;
                auto animIt = activeAnimations.find(propName);
                if (animIt != activeAnimations.end()) {
                    auto* existing = dynamic_cast<AnimationState<T>*>(animIt->second.get());
                    if (existing) fromValue = existing->current;
                }
                activeAnimations[propName] =
                    std::make_unique<AnimationState<T>>(fromValue, newValue, *config);
            }
        } catch (const std::bad_any_cast&) {}
    }
    animSnapshots_[propName] = newValue;
}

template<Animatable T>
T Element::animateValue(const char* propName, T targetValue) {
    static const Animation kImplicit = Animation::defaultImplicit();
    bool hadAnimations = !activeAnimations.empty();
    reconcileProperty<T>(propName, targetValue, &kImplicit);
    if (!hadAnimations && !activeAnimations.empty()) {
        AnimationEngine::instance().registerElement(this);
    }
    return getAnimatedValue<T>(propName, targetValue);
}

} // namespace flux
