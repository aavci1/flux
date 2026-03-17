#include <catch2/catch_test_macros.hpp>
#include <Flux/Core/View.hpp>
#include <Flux/Core/Element.hpp>
#include <Flux/Core/LayoutTree.hpp>

using namespace flux;

static int mountCount = 0;
static int unmountCount = 0;

struct LifecycleWidget {
    FLUX_VIEW_PROPERTIES;
    Property<std::string> label = "test";

    void onMount() { ++mountCount; }
    void onUnmount() { ++unmountCount; }
};

struct SimpleWidget {
    FLUX_VIEW_PROPERTIES;
    Property<std::string> text = "hello";
};

TEST_CASE("Element tree can be built from LayoutNode tree", "[element]") {
    View rootView = SimpleWidget{ .text = "root" };
    LayoutNode rootNode(rootView, {0, 0, 800, 600});

    View child1 = SimpleWidget{ .text = "child1" };
    View child2 = SimpleWidget{ .text = "child2" };
    rootNode.children.push_back(LayoutNode(child1, {0, 0, 400, 600}));
    rootNode.children.push_back(LayoutNode(child2, {400, 0, 400, 600}));

    auto root = Element::buildTree(rootNode);

    REQUIRE(root != nullptr);
    REQUIRE(root->children.size() == 2);
    REQUIRE(root->isMounted);
    REQUIRE(root->children[0]->isMounted);
    REQUIRE(root->children[1]->isMounted);
}

TEST_CASE("Element tree reconciliation preserves existing elements", "[element]") {
    View rootView = SimpleWidget{ .text = "root" };
    LayoutNode node1(rootView, {0, 0, 800, 600});
    View child = SimpleWidget{ .text = "child" };
    node1.children.push_back(LayoutNode(child, {0, 0, 800, 600}));

    auto root = Element::buildTree(node1);
    Element* childPtr = root->children[0].get();

    LayoutNode node2(rootView, {0, 0, 800, 600});
    View child2 = SimpleWidget{ .text = "updated" };
    node2.children.push_back(LayoutNode(child2, {0, 0, 800, 600}));

    root->reconcile(node2);

    REQUIRE(root->children.size() == 1);
    REQUIRE(root->children[0].get() == childPtr);
}

TEST_CASE("Element tree calls onMount and onUnmount", "[element]") {
    mountCount = 0;
    unmountCount = 0;

    View rootView = SimpleWidget{ .text = "root" };
    LayoutNode node1(rootView, {0, 0, 800, 600});
    View lw = LifecycleWidget{ .label = "mounted" };
    node1.children.push_back(LayoutNode(lw, {0, 0, 400, 300}));

    auto root = Element::buildTree(node1);
    REQUIRE(mountCount == 1);
    REQUIRE(unmountCount == 0);

    LayoutNode node2(rootView, {0, 0, 800, 600});
    root->reconcile(node2);
    REQUIRE(unmountCount == 1);
}

TEST_CASE("Element tree reconciliation creates new elements for new children", "[element]") {
    mountCount = 0;
    unmountCount = 0;

    View rootView = SimpleWidget{ .text = "root" };
    LayoutNode node1(rootView, {0, 0, 800, 600});
    auto root = Element::buildTree(node1);
    REQUIRE(root->children.empty());

    LayoutNode node2(rootView, {0, 0, 800, 600});
    View lw = LifecycleWidget{ .label = "new" };
    node2.children.push_back(LayoutNode(lw, {0, 0, 400, 300}));

    root->reconcile(node2);
    REQUIRE(root->children.size() == 1);
    REQUIRE(root->children[0]->isMounted);
    REQUIRE(mountCount == 1);
}
