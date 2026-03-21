#pragma once

#include <Flux/Core/Types.hpp>
#include <memory>
#include <vector>
#include <string>

namespace flux {

class View;
struct LayoutNode;

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

    // Monotonic version bumped when this element's render output changes.
    // Used by CommandCompiler to skip recompilation of unchanged elements.
    uint64_t renderVersion_ = 0;
    // Max of renderVersion_ across this element and all descendants.
    uint64_t subtreeRenderVersion_ = 0;

    bool hasValidLayout() const {
        return !layoutDirty && cachedBounds.width > 0;
    }

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
};

} // namespace flux
