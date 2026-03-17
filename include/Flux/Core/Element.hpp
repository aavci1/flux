#pragma once

#include <memory>
#include <vector>
#include <string>

namespace flux {

class View;
struct LayoutNode;

class Element {
public:
    std::string typeName;
    size_t structuralIndex = 0;

    std::unique_ptr<View> description;
    std::vector<std::unique_ptr<Element>> children;

    bool isMounted = false;
    bool bodyDirty = true;
    bool layoutDirty = true;

    Element();
    ~Element();

    Element(const Element&) = delete;
    Element& operator=(const Element&) = delete;
    Element(Element&&) noexcept;
    Element& operator=(Element&&) noexcept;

    static std::unique_ptr<Element> buildTree(const LayoutNode& node, size_t index = 0);

    void reconcile(const LayoutNode& newNode);

private:
    void reconcileChildren(const std::vector<LayoutNode>& newChildren);
    void mountSubtree();
    void unmountSubtree();
};

} // namespace flux
