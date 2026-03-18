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
#include <string>

namespace flux {

class View;
struct LayoutNode;

std::string demangleTypeName(const char* mangledName);



// Tier 1: Visual + layout essentials (every component gets these)
#define FLUX_VIEW_PROPERTIES \
    Property<EdgeInsets> padding = {}; \
    Property<Color> backgroundColor = Colors::transparent; \
    Property<BackgroundImage> backgroundImage = BackgroundImage{}; \
    Property<Color> borderColor = Colors::transparent; \
    Property<float> borderWidth = 0; \
    Property<CornerRadius> cornerRadius = CornerRadius{0, 0, 0, 0}; \
    Property<float> opacity = 1.0; \
    Property<bool> visible = true; \
    Property<bool> clip = false; \
    Property<float> expansionBias = 0.0f; \
    Property<float> compressionBias = 1.0f; \
    Property<std::optional<float>> minWidth = std::nullopt; \
    Property<std::optional<float>> maxWidth = std::nullopt; \
    Property<std::optional<float>> minHeight = std::nullopt; \
    Property<std::optional<float>> maxHeight = std::nullopt; \
    Property<int> colspan = 1; \
    Property<int> rowspan = 1; \
    Property<std::optional<CursorType>> cursor = std::nullopt; \
    Property<bool> focusable = false; \
    Property<std::string> focusKey = ""

// Tier 2: Interactive components add these event callbacks
#define FLUX_INTERACTIVE_PROPERTIES \
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
    std::function<void(float, float, float, float)> onScroll = nullptr

// Tier 3: Transform properties (most components don't need these)
#define FLUX_TRANSFORM_PROPERTIES \
    Property<float> rotation = 0; \
    Property<float> scaleX = 1.0; \
    Property<float> scaleY = 1.0; \
    Property<Point> offset = Point{0, 0}

// Concept for what makes a View component
// All methods are now optional - if not defined, default implementations will be used
template<typename T>
concept ViewComponent =
    !std::is_same_v<std::remove_cvref_t<T>, View> &&
    requires(const T& t) {
        { t.visible } -> std::convertible_to<bool>;
        { t.padding };
    };

// Virtual interface for type erasure (runtime polymorphism)
class ViewInterface {
public:
    virtual ~ViewInterface() = default;

    // Core methods - now with default implementations
    virtual LayoutNode layout(RenderContext& ctx, const Rect& bounds) const = 0;

    virtual View body() const = 0;

    virtual void render(RenderContext& ctx, const Rect& bounds) const = 0;

    virtual Size preferredSize(TextMeasurement& textMeasurer) const = 0;


    // Access to common properties (all components have these via FLUX_VIEW_PROPERTIES macro)
    virtual bool isVisible() const = 0;
    virtual bool shouldClip() const = 0;
    virtual float getExpansionBias() const = 0;
    virtual float getCompressionBias() const = 0;
    virtual std::optional<float> getMinWidth() const = 0;
    virtual std::optional<float> getMaxWidth() const = 0;
    virtual std::optional<float> getMinHeight() const = 0;
    virtual std::optional<float> getMaxHeight() const = 0;
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
    virtual void handleMouseEnter() {}
    virtual void handleMouseLeave() {}
    virtual bool handleMouseScroll(float x, float y, float deltaX, float deltaY) { (void)x; (void)y; (void)deltaX; (void)deltaY; return false; }
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

    // Lifecycle
    virtual void onMounted() {}
    virtual void onUnmounted() {}
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

#define FLUX_DEFINE_HAS_FIELD(name) \
template<typename T> struct has_##name { \
    template<typename U> static auto test(const U& u) -> decltype(u.name, std::true_type{}); \
    static std::false_type test(...); \
    static constexpr bool value = decltype(test(std::declval<const T&>()))::value; \
};

FLUX_DEFINE_HAS_FIELD(onClick)
FLUX_DEFINE_HAS_FIELD(onMouseDown)
FLUX_DEFINE_HAS_FIELD(onMouseUp)
FLUX_DEFINE_HAS_FIELD(onMouseMove)
FLUX_DEFINE_HAS_FIELD(onMouseEnter)
FLUX_DEFINE_HAS_FIELD(onMouseLeave)
FLUX_DEFINE_HAS_FIELD(onDoubleClick)
FLUX_DEFINE_HAS_FIELD(onFocus)
FLUX_DEFINE_HAS_FIELD(onBlur)
FLUX_DEFINE_HAS_FIELD(onKeyDown)
FLUX_DEFINE_HAS_FIELD(onKeyUp)
FLUX_DEFINE_HAS_FIELD(onTextInput)
FLUX_DEFINE_HAS_FIELD(onChange)
FLUX_DEFINE_HAS_FIELD(onScroll)

#undef FLUX_DEFINE_HAS_FIELD

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

template<typename T>
struct has_onMount {
    template<typename U>
    static auto test(U&& u) -> decltype(u.onMount(), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<T>()))::value;
};

template<typename T>
struct has_onUnmount {
    template<typename U>
    static auto test(U&& u) -> decltype(u.onUnmount(), std::true_type{});
    static std::false_type test(...);
    static constexpr bool value = decltype(test(std::declval<T>()))::value;
};

// Template wrapper that adapts ViewComponent to ViewInterface
template<ViewComponent T>
class ViewAdapter : public ViewInterface {
private:
    mutable T component;
    mutable std::unique_ptr<View> cachedBody_;

    const View& getCachedBody() const;

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

    bool isVisible() const override;
    bool shouldClip() const override;
    float getExpansionBias() const override;
    float getCompressionBias() const override;
    std::optional<float> getMinWidth() const override;
    std::optional<float> getMaxWidth() const override;
    std::optional<float> getMinHeight() const override;
    std::optional<float> getMaxHeight() const override;
    int getColspan() const override;
    int getRowspan() const override;

    std::string getTypeName() const override {
        return demangleTypeName(typeid(T).name());
    }

    // Event handling methods
    bool handleMouseDown(float x, float y, int button) override;
    bool handleMouseUp(float x, float y, int button) override;
    bool handleMouseMove(float x, float y) override;
    void handleMouseEnter() override;
    void handleMouseLeave() override;
    bool handleMouseScroll(float x, float y, float deltaX, float deltaY) override;
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

    // Lifecycle
    void onMounted() override;
    void onUnmounted() override;
};

// Type-erased view container that supports any component type
class View {
private:
    std::shared_ptr<ViewInterface> component_;

public:
    View() : component_(nullptr) {}

    template<typename T>
    requires ViewComponent<std::remove_cvref_t<T>>
    View(T&& component)
        : component_(std::make_shared<ViewAdapter<std::remove_cvref_t<T>>>(std::forward<T>(component))) {}

    View(const View&) = default;
    View& operator=(const View&) = default;
    View(View&&) = default;
    View& operator=(View&&) = default;

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

    std::optional<float> getMinWidth() const {
        return component_ ? component_->getMinWidth() : std::nullopt;
    }

    std::optional<float> getMaxWidth() const {
        return component_ ? component_->getMaxWidth() : std::nullopt;
    }

    std::optional<float> getMinHeight() const {
        return component_ ? component_->getMinHeight() : std::nullopt;
    }

    std::optional<float> getMaxHeight() const {
        return component_ ? component_->getMaxHeight() : std::nullopt;
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

    void handleMouseEnter() {
        if (component_) component_->handleMouseEnter();
    }

    void handleMouseLeave() {
        if (component_) component_->handleMouseLeave();
    }

    bool handleMouseScroll(float x, float y, float deltaX, float deltaY) {
        return component_ ? component_->handleMouseScroll(x, y, deltaX, deltaY) : false;
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

    void onMounted() {
        if (component_) component_->onMounted();
    }

    void onUnmounted() {
        if (component_) component_->onUnmounted();
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

        if constexpr (has_body<T>::value) {
            node.resolvedBody = getCachedBody();
        }

        // Resolve children property if present and store result
        if constexpr (has_children_property<T>::value) {
            node.resolvedChildren = component.children;
        }

        // Build children layout nodes from resolved results
        std::vector<LayoutNode> childNodes;

        // Apply component's padding to get content bounds
        EdgeInsets componentPadding = component.padding;
        Rect contentBounds = {
            bounds.x + componentPadding.left,
            bounds.y + componentPadding.top,
            bounds.width - componentPadding.horizontal(),
            bounds.height - componentPadding.vertical()
        };

        // Add body result as child
        if (node.resolvedBody.has_value() && node.resolvedBody->isValid()) {
            LayoutNode bodyLayout = node.resolvedBody->layout(ctx, contentBounds);
            childNodes.push_back(std::move(bodyLayout));
        }

        // Add children property results as children
        if (node.resolvedChildren.has_value()) {
            for (const auto& childView : *node.resolvedChildren) {
                if (childView.isValid()) {
                    LayoutNode childLayout = childView.layout(ctx, contentBounds);
                    childNodes.push_back(std::move(childLayout));
                }
            }
        }

        node.children = std::move(childNodes);
        return node;
    }
}

template<ViewComponent T>
inline const View& ViewAdapter<T>::getCachedBody() const {
    if constexpr (has_body<T>::value)
        cachedBody_ = std::make_unique<View>(component.body());
    else if (!cachedBody_)
        cachedBody_ = std::make_unique<View>();
    return *cachedBody_;
}

template<ViewComponent T>
inline View ViewAdapter<T>::body() const {
    return getCachedBody();
}

template<ViewComponent T>
inline void ViewAdapter<T>::render(RenderContext& ctx, const Rect& bounds) const {
    if constexpr (has_body<T>::value) {
        const View& bodyView = getCachedBody();
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
        const View& bodyView = getCachedBody();
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
        const View& bodyView = getCachedBody();
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
        const View& bodyView = getCachedBody();
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
        const View& bodyView = getCachedBody();
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
        const View& bodyView = getCachedBody();
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
inline std::optional<float> ViewAdapter<T>::getMinWidth() const {
    // If component has body(), inherit minWidth from body unless explicitly overridden
    if constexpr (has_body<T>::value) {
        const View& bodyView = getCachedBody();
        if (bodyView.isValid()) {
            // If component's minWidth is set, use it
            auto minWidthVal = component.minWidth.get();
            if (minWidthVal) {
                return minWidthVal;
            }
            // Otherwise, use body's minWidth property
            return bodyView->getMinWidth();
        }
    }
    return component.minWidth.get();
}

template<ViewComponent T>
inline std::optional<float> ViewAdapter<T>::getMaxWidth() const {
    if constexpr (has_body<T>::value) {
        const View& bodyView = getCachedBody();
        if (bodyView.isValid()) {
            auto maxWidthVal = component.maxWidth.get();
            if (maxWidthVal) {
                return maxWidthVal;
            }
            return bodyView->getMaxWidth();
        }
    }
    return component.maxWidth.get();
}

template<ViewComponent T>
inline std::optional<float> ViewAdapter<T>::getMinHeight() const {
    if constexpr (has_body<T>::value) {
        const View& bodyView = getCachedBody();
        if (bodyView.isValid()) {
            auto minHeightVal = component.minHeight.get();
            if (minHeightVal) {
                return minHeightVal;
            }
            return bodyView->getMinHeight();
        }
    }
    return component.minHeight.get();
}

template<ViewComponent T>
inline std::optional<float> ViewAdapter<T>::getMaxHeight() const {
    if constexpr (has_body<T>::value) {
        const View& bodyView = getCachedBody();
        if (bodyView.isValid()) {
            auto maxHeightVal = component.maxHeight.get();
            if (maxHeightVal) {
                return maxHeightVal;
            }
            return bodyView->getMaxHeight();
        }
    }
    return component.maxHeight.get();
}

template<ViewComponent T>
inline int ViewAdapter<T>::getColspan() const {
    // If component has body(), inherit colspan from body unless explicitly overridden
    if constexpr (has_body<T>::value) {
        const View& bodyView = getCachedBody();
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
        const View& bodyView = getCachedBody();
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
    if constexpr (has_onMouseDown<T>::value) {
        if (component.onMouseDown) { component.onMouseDown(x, y, button); handled = true; }
    }
    if constexpr (has_onClick<T>::value) {
        if (button == 0 && component.onClick) handled = true;
    }
    return handled;
}

template<ViewComponent T>
inline bool ViewAdapter<T>::handleMouseUp(float x, float y, int button) {
    bool handled = false;
    if constexpr (has_onMouseUp<T>::value) {
        if (component.onMouseUp) { component.onMouseUp(x, y, button); handled = true; }
    }
    if constexpr (has_onClick<T>::value) {
        if (button == 0 && component.onClick) { component.onClick(); handled = true; }
    }
    return handled;
}

template<ViewComponent T>
inline bool ViewAdapter<T>::handleMouseMove(float x, float y) {
    if constexpr (has_onMouseMove<T>::value) {
        if (component.onMouseMove) { component.onMouseMove(x, y); return true; }
    }
    return false;
}

template<ViewComponent T>
inline void ViewAdapter<T>::handleMouseEnter() {
    if constexpr (has_onMouseEnter<T>::value) {
        if (component.onMouseEnter) component.onMouseEnter();
    }
}

template<ViewComponent T>
inline void ViewAdapter<T>::handleMouseLeave() {
    if constexpr (has_onMouseLeave<T>::value) {
        if (component.onMouseLeave) component.onMouseLeave();
    }
}

template<ViewComponent T>
inline bool ViewAdapter<T>::handleMouseScroll(float x, float y, float deltaX, float deltaY) {
    if constexpr (has_onScroll<T>::value) {
        if (component.onScroll) { component.onScroll(x, y, deltaX, deltaY); return true; }
    }
    return false;
}

template<ViewComponent T>
inline bool ViewAdapter<T>::isInteractive() const {
    bool result = false;
    if constexpr (has_onClick<T>::value) result = result || component.onClick != nullptr;
    if constexpr (has_onMouseDown<T>::value) result = result || component.onMouseDown != nullptr;
    if constexpr (has_onMouseUp<T>::value) result = result || component.onMouseUp != nullptr;
    if constexpr (has_onMouseMove<T>::value) result = result || component.onMouseMove != nullptr;
    if constexpr (has_onMouseEnter<T>::value) result = result || component.onMouseEnter != nullptr;
    if constexpr (has_onMouseLeave<T>::value) result = result || component.onMouseLeave != nullptr;
    if constexpr (has_onDoubleClick<T>::value) result = result || component.onDoubleClick != nullptr;
    if constexpr (has_onScroll<T>::value) result = result || component.onScroll != nullptr;
    return result;
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
    
    if constexpr (has_onKeyDown<T>::value) {
        if (component.onKeyDown) return component.onKeyDown(event);
    }
    return false;
}

template<ViewComponent T>
inline bool ViewAdapter<T>::handleKeyUp(const KeyEvent& event) {
    if constexpr (has_handleKeyUp<T>::value) {
        bool handled = component.handleKeyUp(event);
        if (handled) return true;
    }
    if constexpr (has_onKeyUp<T>::value) {
        if (component.onKeyUp) return component.onKeyUp(event);
    }
    return false;
}

template<ViewComponent T>
inline bool ViewAdapter<T>::handleTextInput(const TextInputEvent& event) {
    if constexpr (has_handleTextInput<T>::value) {
        bool handled = component.handleTextInput(event);
        if (handled) return true;
    }
    if constexpr (has_onTextInput<T>::value) {
        if (component.onTextInput) { component.onTextInput(event.text); return true; }
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
    if constexpr (has_onFocus<T>::value) {
        if (component.onFocus) component.onFocus();
    }
}

template<ViewComponent T>
inline void ViewAdapter<T>::notifyFocusLost() {
    if constexpr (has_onBlur<T>::value) {
        if (component.onBlur) component.onBlur();
    }
}

template<ViewComponent T>
inline void ViewAdapter<T>::onMounted() {
    if constexpr (has_onMount<T>::value) {
        component.onMount();
    }
}

template<ViewComponent T>
inline void ViewAdapter<T>::onUnmounted() {
    if constexpr (has_onUnmount<T>::value) {
        component.onUnmount();
    }
}

} // namespace flux

// Define a guard to indicate View.hpp has been fully processed
#define FLUX_VIEW_HPP_COMPLETE

// Now include LayoutTree.hpp which will provide the implementation of printLayoutTree
#include <Flux/Core/LayoutTree.hpp>
