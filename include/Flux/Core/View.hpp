#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Core/State.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <memory>
#include <vector>
#include <functional>
#include <concepts>
#include <typeinfo>
#include <iostream>
#include <string>
#include <optional>
#include <cxxabi.h>

namespace flux {

// Forward declarations
class View;
struct LayoutNode;

// Helper function to demangle C++ type names
inline std::string demangleTypeName(const char* mangledName) {
    int status = 0;
    char* demangled = abi::__cxa_demangle(mangledName, nullptr, nullptr, &status);

    if (status == 0 && demangled) {
        std::string result(demangled);
        free(demangled);
        return result;
    }

    // If demangling fails, return the mangled name
    return std::string(mangledName);
}



// Macro to inject common view properties into components
// This allows designated initializers to work while maintaining code reuse
#define FLUX_VIEW_PROPERTIES \
    Property<EdgeInsets> margin = {}; \
    Property<EdgeInsets> padding = {}; \
    Property<Color> backgroundColor = Colors::transparent; \
    Property<BackgroundImage> backgroundImage = BackgroundImage{}; \
    Property<Color> borderColor = Colors::transparent; \
    Property<float> borderWidth = 0; \
    Property<float> cornerRadius = 0; \
    Property<float> opacity = 1.0; \
    Property<bool> visible = true; \
    Property<bool> clip = false; \
    Property<float> rotation = 0; \
    Property<float> scaleX = 1.0; \
    Property<float> scaleY = 1.0; \
    Property<Point> offset = Point{0, 0}; \
    Property<Shadow> shadow = Shadow{}; \
    Property<float> expansionBias = 0.0f; \
    Property<float> compressionBias = 1.0f

// Concept for what makes a View component
// All methods are now optional - if not defined, default implementations will be used
template<typename T>
concept ViewComponent = (!std::is_same_v<std::remove_cvref_t<T>, View>);

// Virtual interface for type erasure (runtime polymorphism)
class ViewInterface {
public:
    virtual ~ViewInterface() = default;

    // Core methods - now with default implementations
    virtual LayoutNode layout(RenderContext& ctx, const Rect& bounds) const = 0;

    virtual View body() const = 0;

    virtual void render(RenderContext& ctx, const Rect& bounds) const = 0;

    virtual Size preferredSize(TextMeasurement& textMeasurer) const = 0;

    virtual std::unique_ptr<ViewInterface> clone() const = 0;

    // Access to common properties (all components have these via FLUX_VIEW_PROPERTIES macro)
    virtual bool isVisible() const = 0;
    virtual bool shouldClip() const = 0;
    virtual float getExpansionBias() const = 0;
    virtual float getCompressionBias() const = 0;

    // Get the type name of the underlying component (demangled)
    virtual std::string getTypeName() const = 0;

    // New methods for children property handling
    virtual bool hasChildrenProperty() const = 0;
    virtual std::vector<View> getChildren() const = 0;
};

// SFINAE helpers to detect if methods exist
template<typename T>
struct has_layout {
    template<typename U>
    static auto test(U&& u) -> decltype(u.layout(std::declval<RenderContext&>(), std::declval<const Rect&>()), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<T>()))::value;
};

template<typename T>
struct has_body {
    template<typename U>
    static auto test(const U& u) -> decltype(u.body(), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<const T&>()))::value;
};

template<typename T>
struct has_render {
    template<typename U>
    static auto test(const U& u) -> decltype(u.render(std::declval<RenderContext&>(), std::declval<const Rect&>()), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<const T&>()))::value;
};

template<typename T>
struct has_preferredSize {
    template<typename U>
    static auto test(const U& u) -> decltype(u.preferredSize(std::declval<TextMeasurement&>()), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<const T&>()))::value;
};

template<typename T>
struct has_children_property {
    template<typename U>
    static auto test(const U& u) -> decltype(u.children, std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<const T&>()))::value;
};

// Template wrapper that adapts ViewComponent to ViewInterface
template<ViewComponent T>
class ViewAdapter : public ViewInterface {
private:
    mutable T component;  // mutable because layout() needs to modify children

public:
    ViewAdapter(const T& comp) : component(comp) {}
    ViewAdapter(T&& comp) : component(std::move(comp)) {}

    LayoutNode layout(RenderContext& ctx, const Rect& bounds) const override;
    View body() const override;
    void render(RenderContext& ctx, const Rect& bounds) const override;
    Size preferredSize(TextMeasurement& textMeasurer) const override;

    // New children property methods
    bool hasChildrenProperty() const override;
    std::vector<View> getChildren() const override;

    std::unique_ptr<ViewInterface> clone() const override {
        return std::make_unique<ViewAdapter<T>>(component);
    }

    bool isVisible() const override {
        return component.visible;
    }

    bool shouldClip() const override {
        return component.clip;
    }

    float getExpansionBias() const override {
        return component.expansionBias;
    }

    float getCompressionBias() const override {
        return component.compressionBias;
    }

    std::string getTypeName() const override {
        return demangleTypeName(typeid(T).name());
    }
};

// Type-erased view container that supports any component type
class View {
private:
    std::unique_ptr<ViewInterface> component_;

public:
    // Default constructor
    View() : component_(nullptr) {}

    // Constructor from any component type that satisfies ViewComponent
    template<typename T>
    requires ViewComponent<std::remove_cvref_t<T>>
    View(T&& component)
        : component_(std::make_unique<ViewAdapter<std::remove_cvref_t<T>>>(std::forward<T>(component))) {}

    // Move constructor and assignment
    View(View&&) = default;
    View& operator=(View&&) = default;

    // Copy constructor and assignment
    View(const View& other)
        : component_(other.component_ ? other.component_->clone() : nullptr) {}

    View& operator=(const View& other) {
        if (this != &other) {
            component_ = other.component_ ? other.component_->clone() : nullptr;
        }
        return *this;
    }

    // Delegate methods to wrapped component
    LayoutNode layout(RenderContext& ctx, const Rect& bounds) const;

    void render(RenderContext& ctx, const Rect& bounds) const {
        if (component_) {
            component_->render(ctx, bounds);
        }
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        return component_ ? component_->preferredSize(textMeasurer) : Size{};
    }

    bool shouldClip() const {
        return component_ ? component_->shouldClip() : false;
    }

    float getExpansionBias() const {
        return component_ ? component_->getExpansionBias() : 0.0f;
    }

    float getCompressionBias() const {
        return component_ ? component_->getCompressionBias() : 1.0f;
    }

    std::string getTypeName() const {
        return component_ ? component_->getTypeName() : "EmptyView";
    }

    bool isValid() const {
        return component_ != nullptr;
    }

    ViewInterface* operator->() { return component_.get(); }
    const ViewInterface* operator->() const { return component_.get(); }
    ViewInterface& operator*() { return *component_; }
    const ViewInterface& operator*() const { return *component_; }
};

// Layout tree node - contains view, bounds, and children
// Enhanced to store resolved body() and children results for reuse between layout and render phases
struct LayoutNode {
    View view;  // Copy of the view
    Rect bounds;
    std::vector<LayoutNode> children;

    // Store resolved body() and children results to avoid duplicate calls
    std::optional<View> resolvedBody;  // Result of body() call
    std::optional<std::vector<View>> resolvedChildren;  // Result of children lambda evaluation

    LayoutNode() : view(), bounds() {}
    LayoutNode(const View& v, const Rect& b) : view(v), bounds(b) {}
    LayoutNode(const View& v, const Rect& b, std::vector<LayoutNode>&& c)
        : view(v), bounds(b), children(std::move(c)) {}
};

// Now that LayoutNode is defined, implement the View::layout method

inline LayoutNode View::layout(RenderContext& ctx, const Rect& bounds) const {
    return component_->layout(ctx, bounds);
}

// Now implement the ViewAdapter methods with proper default implementations

template<ViewComponent T>
inline LayoutNode ViewAdapter<T>::layout(RenderContext& ctx, const Rect& bounds) const {
    if constexpr (has_layout<T>::value) {
        return component.layout(ctx, bounds);
    } else {
        // Default layout: resolve body() and children, store results in LayoutNode
        LayoutNode node(View(component), bounds);

        // Resolve body() if present and store result
        if constexpr (has_body<T>::value) {
            node.resolvedBody = component.body();
        }

        // Resolve children property if present and store result
        if constexpr (has_children_property<T>::value) {
            node.resolvedChildren = component.children;
        }

        // Build children layout nodes from resolved results
        std::vector<LayoutNode> childNodes;

        // Add body result as child
        if (node.resolvedBody.has_value() && node.resolvedBody->isValid()) {
            LayoutNode bodyLayout = node.resolvedBody->layout(ctx, bounds);
            childNodes.push_back(std::move(bodyLayout));
        }

        // Add children property results as children
        if (node.resolvedChildren.has_value()) {
            for (const auto& childView : *node.resolvedChildren) {
                if (childView.isValid()) {
                    LayoutNode childLayout = childView.layout(ctx, bounds);
                    childNodes.push_back(std::move(childLayout));
                }
            }
        }

        node.children = std::move(childNodes);
        return node;
    }
}

template<ViewComponent T>
inline View ViewAdapter<T>::body() const {
    if constexpr (has_body<T>::value) {
        return component.body();
    } else {
        // Default: no children (return empty view)
        return View{};
    }
}

template<ViewComponent T>
inline void ViewAdapter<T>::render(RenderContext& ctx, const Rect& bounds) const {
    if constexpr (has_render<T>::value) {
        component.render(ctx, bounds);
    } else {
        // Default: apply basic view decorations
        // Note: children rendering is now handled by the renderer using LayoutNode.children
        // No more body() calls here - results are stored in LayoutNode and reused
        ViewHelpers::renderView(component, ctx, bounds);
    }
}

template<ViewComponent T>
inline Size ViewAdapter<T>::preferredSize(TextMeasurement& textMeasurer) const {
    if constexpr (has_preferredSize<T>::value) {
        return component.preferredSize(textMeasurer);
    } else {
        // Default: minimal size with padding
        EdgeInsets paddingVal = component.padding;
        return {paddingVal.horizontal(), paddingVal.vertical()};
    }
}

template<ViewComponent T>
inline bool ViewAdapter<T>::hasChildrenProperty() const {
    if constexpr (has_children_property<T>::value) {
        return true;
    }
    return false;
}

template<ViewComponent T>
inline std::vector<View> ViewAdapter<T>::getChildren() const {
    if constexpr (has_children_property<T>::value) {
        // This evaluates the lambda if it's a lambda property
        return component.children;
    }
    return {};
}

} // namespace flux

// Define a guard to indicate View.hpp has been fully processed
#define FLUX_VIEW_HPP_COMPLETE

// Now include LayoutTree.hpp which will provide the implementation of printLayoutTree
#include <Flux/Core/LayoutTree.hpp>
