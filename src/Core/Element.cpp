#include <Flux/Core/Element.hpp>
#include <Flux/Core/View.hpp>
#include <Flux/Core/Log.hpp>
#include <Flux/Animation/AnimationEngine.hpp>

namespace flux {

Element::Element() = default;

Element::~Element() {
    if (isMounted) {
        unmountSubtree();
    }
}

Element::Element(Element&&) noexcept = default;
Element& Element::operator=(Element&&) noexcept = default;

void Element::markDirty() {
    bodyDirty = true;
    if (description) {
        description->markBodyDirty();
    }
    for (Element* p = this; p && !p->subtreeDirty; p = p->parent) {
        p->subtreeDirty = true;
    }
    requestRedrawOnly();
}

std::unique_ptr<Element> Element::buildTree(const LayoutNode& node, size_t index) {
    auto element = std::make_unique<Element>();
    element->typeName = node.view.getTypeName();
    element->key = node.view.getKey();
    element->structuralIndex = index;
    element->description = std::make_unique<View>(node.view);
    element->cachedBounds = node.bounds;
    element->lastConstraints = node.bounds;

    for (size_t i = 0; i < node.children.size(); ++i) {
        auto child = buildTree(node.children[i], i);
        child->parent = element.get();
        element->children.push_back(std::move(child));
    }

    element->mountSubtree();
    return element;
}

void Element::reconcile(const LayoutNode& newNode) {
    bool boundsChanged = (cachedBounds.x != newNode.bounds.x ||
                          cachedBounds.y != newNode.bounds.y ||
                          cachedBounds.width != newNode.bounds.width ||
                          cachedBounds.height != newNode.bounds.height);

    if (!bodyDirty && !boundsChanged && !subtreeDirty) {
        return;
    }

    if (bodyDirty || boundsChanged) {
        reconcileAnimations(newNode.view, newNode.bounds);
        View oldView = *description;
        *description = newNode.view;
        (**description).transferStateFrom(*oldView);
        description->setPropertyOwner(this);
        typeName = newNode.view.getTypeName();
        key = newNode.view.getKey();
    }

    cachedBounds = newNode.bounds;
    lastConstraints = newNode.bounds;
    bodyDirty = false;
    layoutDirty = boundsChanged;
    subtreeDirty = false;

    reconcileChildren(newNode.children);
}

void Element::reconcileChildren(const std::vector<LayoutNode>& newChildren) {
    std::unordered_map<std::string, size_t> keyIndex;
    std::unordered_map<uint64_t, size_t> structIndex;

    for (size_t j = 0; j < children.size(); ++j) {
        if (!children[j]->key.empty()) {
            keyIndex[children[j]->key] = j;
        } else {
            auto h = std::hash<std::string>{}(children[j]->typeName);
            h ^= children[j]->structuralIndex * 0x9e3779b97f4a7c15ULL;
            structIndex[h] = j;
        }
    }

    std::vector<bool> oldMatched(children.size(), false);
    std::vector<std::unique_ptr<Element>> result;
    result.reserve(newChildren.size());

    for (size_t i = 0; i < newChildren.size(); ++i) {
        const auto& newChild = newChildren[i];
        std::string newKey = newChild.view.getKey();

        size_t matchIdx = SIZE_MAX;

        if (!newKey.empty()) {
            auto it = keyIndex.find(newKey);
            if (it != keyIndex.end() && !oldMatched[it->second]) {
                matchIdx = it->second;
            }
        }

        if (matchIdx == SIZE_MAX) {
            std::string newTypeName = newChild.view.getTypeName();
            auto h = std::hash<std::string>{}(newTypeName);
            h ^= i * 0x9e3779b97f4a7c15ULL;
            auto it = structIndex.find(h);
            if (it != structIndex.end() && !oldMatched[it->second]) {
                matchIdx = it->second;
            }
        }

        if (matchIdx != SIZE_MAX) {
            oldMatched[matchIdx] = true;
            children[matchIdx]->reconcile(newChild);
            result.push_back(std::move(children[matchIdx]));
        } else {
            result.push_back(buildTree(newChild, i));
        }
    }

    for (size_t j = 0; j < children.size(); ++j) {
        if (!oldMatched[j] && children[j]) {
            children[j]->unmountSubtree();
        }
    }

    children = std::move(result);
    for (auto& child : children) {
        child->parent = this;
    }
}

Element* Element::findByFocusKey(const std::string& key) {
    if (description && description->isValid() && description->getFocusKey() == key) {
        return this;
    }
    for (auto& child : children) {
        Element* found = child->findByFocusKey(key);
        if (found) return found;
    }
    return nullptr;
}

void Element::mountSubtree() {
    if (!isMounted) {
        isMounted = true;
        if (description && description->isValid()) {
            description->setPropertyOwner(this);
            description->onMounted();

            auto vs = description->getVisualStyle();
            reconcileProperty<float>("opacity",          vs.opacity,         nullptr);
            reconcileProperty<Color>("backgroundColor",  vs.backgroundColor, nullptr);
            reconcileProperty<Color>("borderColor",      vs.borderColor,     nullptr);
            reconcileProperty<float>("borderWidth",      vs.borderWidth,     nullptr);
            reconcileProperty<CornerRadius>("cornerRadius", vs.cornerRadius, nullptr);
            reconcileProperty<float>("rotation",         vs.rotation,        nullptr);
            reconcileProperty<float>("scaleX",           vs.scaleX,          nullptr);
            reconcileProperty<float>("scaleY",           vs.scaleY,          nullptr);
            reconcileProperty<Point>("offset",           vs.offset,          nullptr);
            reconcileProperty<EdgeInsets>("padding",      vs.padding,         nullptr);
            reconcileProperty<Rect>("bounds",            cachedBounds,       nullptr);
        }
        FLUX_LOG_TRACE("[ELEMENT] Mounted %s", typeName.c_str());
    }
    for (auto& child : children) {
        child->mountSubtree();
    }
}

void Element::unmountSubtree() {
    for (auto& child : children) {
        child->unmountSubtree();
    }
    if (isMounted) {
        isMounted = false;
        if (!activeAnimations.empty()) {
            activeAnimations.clear();
            AnimationEngine::instance().unregisterElement(this);
        }
        if (description && description->isValid()) {
            description->onUnmounted();
        }
        FLUX_LOG_TRACE("[ELEMENT] Unmounted %s", typeName.c_str());
    }
}

void Element::reconcileAnimations(const View& newView, const Rect& newBounds) {
    if (!description || !description->isValid()) return;

    auto vs = newView.getVisualStyle();
    auto ctx = currentAnimationContext();
    auto pending = consumePendingAnimationConfig();
    std::optional<Animation> effectiveAnim = vs.animation ? vs.animation : (ctx ? ctx : pending);

    const Animation* config = nullptr;
    Animation resolved;
    if (effectiveAnim && !effectiveAnim->isNone()) {
        resolved = *effectiveAnim;
        config = &resolved;
    }

    bool hadAnimations = !activeAnimations.empty();

    reconcileProperty<float>("opacity",            vs.opacity,         config);
    reconcileProperty<Color>("backgroundColor",    vs.backgroundColor, config);
    reconcileProperty<Color>("borderColor",        vs.borderColor,     config);
    reconcileProperty<float>("borderWidth",        vs.borderWidth,     config);
    reconcileProperty<CornerRadius>("cornerRadius", vs.cornerRadius,   config);
    reconcileProperty<float>("rotation",           vs.rotation,        config);
    reconcileProperty<float>("scaleX",             vs.scaleX,          config);
    reconcileProperty<float>("scaleY",             vs.scaleY,          config);
    reconcileProperty<Point>("offset",             vs.offset,          config);
    reconcileProperty<EdgeInsets>("padding",        vs.padding,         config);
    reconcileProperty<Rect>("bounds",              newBounds,          config);

    if (!hadAnimations && !activeAnimations.empty()) {
        AnimationEngine::instance().registerElement(this);
    }
}

} // namespace flux
