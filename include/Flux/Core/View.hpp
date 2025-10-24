#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <memory>
#include <vector>
#include <functional>
#include <optional>
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
    Property<CornerRadius> cornerRadius = CornerRadius{0, 0, 0, 0}; \
    Property<float> opacity = 1.0; \
    Property<bool> visible = true; \
    Property<bool> clip = false; \
    Property<float> rotation = 0; \
    Property<float> scaleX = 1.0; \
    Property<float> scaleY = 1.0; \
    Property<Point> offset = Point{0, 0}; \
    Property<float> expansionBias = 0.0f; \
    Property<float> compressionBias = 1.0f; \
    Property<int> colspan = 1; \
    Property<int> rowspan = 1; \
    Property<std::optional<CursorType>> cursor = std::nullopt; \
    Property<bool> focusable = false; \
    Property<std::string> focusKey = ""; \
    std::function<void()> onClick = nullptr; \
    std::function<void(float, float, int)> onMouseDown = nullptr; \
    std::function<void(float, float, int)> onMouseUp = nullptr; \
    std::function<void(float, float)> onMouseMove = nullptr; \
    std::function<void()> onMouseEnter = nullptr; \
    std::function<void()> onMouseLeave = nullptr; \
    std::function<void()> onDoubleClick = nullptr; \
    std::function<void()> onFocus = nullptr; \
    std::function<void()> onBlur = nullptr; \
    std::function<bool(const KeyEvent&)> onKeyDown = nullptr; \
    std::function<bool(const KeyEvent&)> onKeyUp = nullptr; \
    std::function<void(const std::string&)> onTextInput = nullptr; \
    std::function<void()> onChange = nullptr; \
    std::function<void(float, float)> onDragStart = nullptr; \
    std::function<void(float, float)> onDrag = nullptr; \
    std::function<void(float, float)> onDragEnd = nullptr; \
    std::function<void(float, float)> onDrop = nullptr

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
    virtual int getColspan() const = 0;
    virtual int getRowspan() const = 0;

    // Get the type name of the underlying component (demangled)
    virtual std::string getTypeName() const = 0;

    // New methods for children property handling
    virtual bool hasChildrenProperty() const = 0;
    virtual std::vector<View> getChildren() const = 0;

    // Event handling methods
    virtual bool handleMouseDown(float x, float y, int button) { (void)x; (void)y; (void)button; return false; }
    virtual bool handleMouseUp(float x, float y, int button) { (void)x; (void)y; (void)button; return false; }
    virtual bool handleMouseMove(float x, float y) { (void)x; (void)y; return false; }
    virtual bool isInteractive() const { return false; }
    
    // Keyboard event handling methods
    virtual bool handleKeyDown(const KeyEvent& event) { (void)event; return false; }
    virtual bool handleKeyUp(const KeyEvent& event) { (void)event; return false; }
    virtual bool handleTextInput(const TextInputEvent& event) { (void)event; return false; }
    
    // Focus management
    virtual bool canBeFocused() const { return false; }
    virtual std::string getFocusKey() const { return ""; }
    virtual void notifyFocusGained() {}
    virtual void notifyFocusLost() {}
    
    // Cursor management
    virtual std::optional<CursorType> getCursor() const = 0;
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

template<typename T>
struct has_onClick {
    template<typename U>
    static auto test(const U& u) -> decltype(u.onClick, std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<const T&>()))::value;
};

template<typename T>
struct has_handleKeyDown {
    template<typename U>
    static auto test(const U& u) -> decltype(u.handleKeyDown(std::declval<const KeyEvent&>()), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<const T&>()))::value;
};

template<typename T>
struct has_handleKeyUp {
    template<typename U>
    static auto test(const U& u) -> decltype(u.handleKeyUp(std::declval<const KeyEvent&>()), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<const T&>()))::value;
};

template<typename T>
struct has_handleTextInput {
    template<typename U>
    static auto test(const U& u) -> decltype(u.handleTextInput(std::declval<const TextInputEvent&>()), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<const T&>()))::value;
};

template<typename T>
struct has_init {
    template<typename U>
    static auto test(U&& u) -> decltype(u.init(), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<T>()))::value;
};

// Template wrapper that adapts ViewComponent to ViewInterface
template<ViewComponent T>
class ViewAdapter : public ViewInterface {
private:
    mutable T component;  // mutable because layout() needs to modify children

public:
    ViewAdapter(const T& comp) : component(comp) {
        if constexpr (has_init<T>::value) {
            component.init();
        }
    }
    ViewAdapter(T&& comp) : component(std::move(comp)) {
        if constexpr (has_init<T>::value) {
            component.init();
        }
    }

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

    bool isVisible() const override;
    bool shouldClip() const override;
    float getExpansionBias() const override;
    float getCompressionBias() const override;
    int getColspan() const override;
    int getRowspan() const override;

    std::string getTypeName() const override {
        return demangleTypeName(typeid(T).name());
    }

    // Event handling methods
    bool handleMouseDown(float x, float y, int button) override;
    bool handleMouseUp(float x, float y, int button) override;
    bool handleMouseMove(float x, float y) override;
    bool isInteractive() const override;
    
    // Keyboard event handling methods
    bool handleKeyDown(const KeyEvent& event) override;
    bool handleKeyUp(const KeyEvent& event) override;
    bool handleTextInput(const TextInputEvent& event) override;
    
    // Focus management
    bool canBeFocused() const override;
    std::string getFocusKey() const override;
    void notifyFocusGained() override;
    void notifyFocusLost() override;
    
    // Cursor management
    std::optional<CursorType> getCursor() const override;
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

    int getColspan() const {
        return component_ ? component_->getColspan() : 1;
    }

    int getRowspan() const {
        return component_ ? component_->getRowspan() : 1;
    }

    std::string getTypeName() const {
        return component_ ? component_->getTypeName() : "EmptyView";
    }

    bool isValid() const {
        return component_ != nullptr;
    }

    // Event handling delegation
    bool handleMouseDown(float x, float y, int button) {
        return component_ ? component_->handleMouseDown(x, y, button) : false;
    }

    bool handleMouseUp(float x, float y, int button) {
        return component_ ? component_->handleMouseUp(x, y, button) : false;
    }

    bool handleMouseMove(float x, float y) {
        return component_ ? component_->handleMouseMove(x, y) : false;
    }

    bool isInteractive() const {
        return component_ ? component_->isInteractive() : false;
    }

    // Keyboard event handling delegation
    bool handleKeyDown(const KeyEvent& event) {
        return component_ ? component_->handleKeyDown(event) : false;
    }

    bool handleKeyUp(const KeyEvent& event) {
        return component_ ? component_->handleKeyUp(event) : false;
    }

    bool handleTextInput(const TextInputEvent& event) {
        return component_ ? component_->handleTextInput(event) : false;
    }

    // Focus management delegation
    bool canBeFocused() const {
        return component_ ? component_->canBeFocused() : false;
    }

    std::string getFocusKey() const {
        return component_ ? component_->getFocusKey() : "";
    }

    void notifyFocusGained() {
        if (component_) component_->notifyFocusGained();
    }

    void notifyFocusLost() {
        if (component_) component_->notifyFocusLost();
    }

    std::optional<CursorType> getCursor() const {
        return component_ ? component_->getCursor() : std::nullopt;
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
    if constexpr (has_body<T>::value) {
        // If view has body(), render the body recursively (no parent decorations)
        View bodyView = component.body();
        if (bodyView.isValid()) {
            bodyView.render(ctx, bounds);
        }
    } else if constexpr (has_render<T>::value) {
        component.render(ctx, bounds);
    } else {
        // Default: apply basic view decorations
        // Note: children rendering is now handled by the renderer using LayoutNode.children
        ViewHelpers::renderView(component, ctx, bounds);
    }
}

template<ViewComponent T>
inline Size ViewAdapter<T>::preferredSize(TextMeasurement& textMeasurer) const {
    if constexpr (has_preferredSize<T>::value) {
        return component.preferredSize(textMeasurer);
    } else if constexpr (has_body<T>::value) {
        // If component has body() but no preferredSize(), use preferredSize of body()
        View bodyView = component.body();
        if (bodyView.isValid()) {
            return bodyView.preferredSize(textMeasurer);
        }
        // Fallback: minimal size with padding
        EdgeInsets paddingVal = component.padding;
        return {paddingVal.horizontal(), paddingVal.vertical()};
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
inline bool ViewAdapter<T>::isVisible() const {
    // If component has body(), inherit visible from body unless explicitly overridden
    if constexpr (has_body<T>::value) {
        View bodyView = component.body();
        if (bodyView.isValid()) {
            // If component's visible is not the default (true), use component's value
            if (static_cast<bool>(component.visible) != true) {
                return component.visible;
            }
            // Otherwise, use body's visible property
            return bodyView->isVisible();
        }
    }
    return component.visible;
}

template<ViewComponent T>
inline bool ViewAdapter<T>::shouldClip() const {
    // If component has body(), inherit clip from body unless explicitly overridden
    if constexpr (has_body<T>::value) {
        View bodyView = component.body();
        if (bodyView.isValid()) {
            // If component's clip is not the default (false), use component's value
            if (static_cast<bool>(component.clip) != false) {
                return component.clip;
            }
            // Otherwise, use body's clip property
            return bodyView->shouldClip();
        }
    }
    return component.clip;
}

template<ViewComponent T>
inline float ViewAdapter<T>::getExpansionBias() const {
    // If component has body(), inherit expansionBias from body unless explicitly overridden
    if constexpr (has_body<T>::value) {
        View bodyView = component.body();
        if (bodyView.isValid()) {
            // If component's expansionBias is not the default (0.0f), use component's value
            if (static_cast<float>(component.expansionBias) != 0.0f) {
                return component.expansionBias;
            }
            // Otherwise, use body's expansionBias property
            return bodyView->getExpansionBias();
        }
    }
    return component.expansionBias;
}

template<ViewComponent T>
inline float ViewAdapter<T>::getCompressionBias() const {
    // If component has body(), inherit compressionBias from body unless explicitly overridden
    if constexpr (has_body<T>::value) {
        View bodyView = component.body();
        if (bodyView.isValid()) {
            // If component's compressionBias is not the default (1.0f), use component's value
            if (static_cast<float>(component.compressionBias) != 1.0f) {
                return component.compressionBias;
            }
            // Otherwise, use body's compressionBias property
            return bodyView->getCompressionBias();
        }
    }
    return component.compressionBias;
}

template<ViewComponent T>
inline int ViewAdapter<T>::getColspan() const {
    // If component has body(), inherit colspan from body unless explicitly overridden
    if constexpr (has_body<T>::value) {
        View bodyView = component.body();
        if (bodyView.isValid()) {
            // If component's colspan is not the default (1), use component's value
            if (static_cast<int>(component.colspan) != 1) {
                return component.colspan;
            }
            // Otherwise, use body's colspan property
            return bodyView->getColspan();
        }
    }
    return component.colspan;
}

template<ViewComponent T>
inline int ViewAdapter<T>::getRowspan() const {
    // If component has body(), inherit rowspan from body unless explicitly overridden
    if constexpr (has_body<T>::value) {
        View bodyView = component.body();
        if (bodyView.isValid()) {
            // If component's rowspan is not the default (1), use component's value
            if (static_cast<int>(component.rowspan) != 1) {
                return component.rowspan;
            }
            // Otherwise, use body's rowspan property
            return bodyView->getRowspan();
        }
    }
    return component.rowspan;
}

template<ViewComponent T>
inline std::vector<View> ViewAdapter<T>::getChildren() const {
    if constexpr (has_children_property<T>::value) {
        // This evaluates the lambda if it's a lambda property
        return component.children;
    }
    return {};
}

template<ViewComponent T>
inline bool ViewAdapter<T>::handleMouseDown(float x, float y, int button) {
    bool handled = false;
    
    // Call onMouseDown callback if present
    if (component.onMouseDown) {
        component.onMouseDown(x, y, button);
        handled = true;
    }
    
    // Also call onClick for convenience (primary mouse button)
    if (button == 0 && component.onClick) {
        component.onClick();
        handled = true;
    }
    
    return handled;
}

template<ViewComponent T>
inline bool ViewAdapter<T>::handleMouseUp(float x, float y, int button) {
    if (component.onMouseUp) {
        component.onMouseUp(x, y, button);
        return true;
    }
    return false;
}

template<ViewComponent T>
inline bool ViewAdapter<T>::handleMouseMove(float x, float y) {
    if (component.onMouseMove) {
        component.onMouseMove(x, y);
        return true;
    }
    return false;
}

template<ViewComponent T>
inline bool ViewAdapter<T>::isInteractive() const {
    return component.onClick != nullptr || 
           component.onMouseDown != nullptr ||
           component.onMouseUp != nullptr ||
           component.onMouseMove != nullptr ||
           component.onMouseEnter != nullptr ||
           component.onMouseLeave != nullptr ||
           component.onDoubleClick != nullptr;
}

template<ViewComponent T>
inline std::optional<CursorType> ViewAdapter<T>::getCursor() const {
    return component.cursor;
}

template<ViewComponent T>
inline bool ViewAdapter<T>::handleKeyDown(const KeyEvent& event) {
    // First check if component has custom handleKeyDown method
    if constexpr (has_handleKeyDown<T>::value) {
        bool handled = component.handleKeyDown(event);
        if (handled) return true;
    }
    
    // Then check for onKeyDown callback
    if (component.onKeyDown) {
        return component.onKeyDown(event);
    }
    
    return false;
}

template<ViewComponent T>
inline bool ViewAdapter<T>::handleKeyUp(const KeyEvent& event) {
    // First check if component has custom handleKeyUp method
    if constexpr (has_handleKeyUp<T>::value) {
        bool handled = component.handleKeyUp(event);
        if (handled) return true;
    }
    
    // Then check for onKeyUp callback
    if (component.onKeyUp) {
        return component.onKeyUp(event);
    }
    
    return false;
}

template<ViewComponent T>
inline bool ViewAdapter<T>::handleTextInput(const TextInputEvent& event) {
    // First check if component has custom handleTextInput method
    if constexpr (has_handleTextInput<T>::value) {
        bool handled = component.handleTextInput(event);
        if (handled) return true;
    }
    
    // Then check for onTextInput callback
    if (component.onTextInput) {
        component.onTextInput(event.text);
        return true;
    }
    
    return false;
}

template<ViewComponent T>
inline bool ViewAdapter<T>::canBeFocused() const {
    return component.focusable;
}

template<ViewComponent T>
inline std::string ViewAdapter<T>::getFocusKey() const {
    return component.focusKey;
}

template<ViewComponent T>
inline void ViewAdapter<T>::notifyFocusGained() {
    if (component.onFocus) {
        component.onFocus();
    }
}

template<ViewComponent T>
inline void ViewAdapter<T>::notifyFocusLost() {
    if (component.onBlur) {
        component.onBlur();
    }
}

} // namespace flux

// Define a guard to indicate View.hpp has been fully processed
#define FLUX_VIEW_HPP_COMPLETE

// Now include LayoutTree.hpp which will provide the implementation of printLayoutTree
#include <Flux/Core/LayoutTree.hpp>
