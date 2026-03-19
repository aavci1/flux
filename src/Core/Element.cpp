#include <Flux/Core/Element.hpp>
#include <Flux/Core/View.hpp>
#include <Flux/Core/Log.hpp>

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
    requestApplicationRedraw();
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
    *description = newNode.view;
    description->setPropertyOwner(this);
    typeName = newNode.view.getTypeName();
    key = newNode.view.getKey();

    bool boundsChanged = (cachedBounds.x != newNode.bounds.x ||
                          cachedBounds.y != newNode.bounds.y ||
                          cachedBounds.width != newNode.bounds.width ||
                          cachedBounds.height != newNode.bounds.height);

    cachedBounds = newNode.bounds;
    lastConstraints = newNode.bounds;
    bodyDirty = false;
    layoutDirty = boundsChanged;

    reconcileChildren(newNode.children);
}

void Element::reconcileChildren(const std::vector<LayoutNode>& newChildren) {
    std::vector<bool> oldMatched(children.size(), false);
    std::vector<std::unique_ptr<Element>> result;
    result.reserve(newChildren.size());

    for (size_t i = 0; i < newChildren.size(); ++i) {
        const auto& newChild = newChildren[i];
        std::string newTypeName = newChild.view.getTypeName();
        std::string newKey = newChild.view.getKey();

        int matchIdx = -1;

        if (!newKey.empty()) {
            for (size_t j = 0; j < children.size(); ++j) {
                if (!oldMatched[j] && children[j]->key == newKey) {
                    matchIdx = static_cast<int>(j);
                    break;
                }
            }
        }

        if (matchIdx < 0) {
            for (size_t j = 0; j < children.size(); ++j) {
                if (!oldMatched[j] &&
                    children[j]->key.empty() &&
                    children[j]->typeName == newTypeName &&
                    children[j]->structuralIndex == i) {
                    matchIdx = static_cast<int>(j);
                    break;
                }
            }
        }

        if (matchIdx >= 0) {
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
        if (description && description->isValid()) {
            description->onUnmounted();
        }
        FLUX_LOG_TRACE("[ELEMENT] Unmounted %s", typeName.c_str());
    }
}

} // namespace flux
