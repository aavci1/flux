#pragma once

#include <Flux/Core/ViewInterface.hpp>
#include <Flux/Core/ViewTraits.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <memory>
#include <typeinfo>
#include <cstdio>

namespace flux {

template<ViewComponent T>
class ViewAdapter : public ViewInterface {
private:
    mutable T component;
    mutable std::unique_ptr<View> cachedBody_;
    mutable uint64_t cachedBodyGen_ = 0;

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
    float heightForWidth(float width, TextMeasurement& textMeasurer) const override;

    bool hasChildrenProperty() const override;
    std::vector<View> getChildren() const override;

    VisualStyle getVisualStyle() const override;
    LayoutConstraints getLayoutConstraints() const override;

    std::string getTypeName() const override {
        return demangleTypeName(typeid(T).name());
    }

    bool handleMouseDown(float x, float y, int button) override;
    bool handleMouseUp(float x, float y, int button) override;
    bool handleMouseMove(float x, float y) override;
    void handleMouseEnter() override;
    void handleMouseLeave() override;
    bool handleMouseScroll(float x, float y, float deltaX, float deltaY) override;
    bool isInteractive() const override;
    bool capturePointerEvent(PointerEvent& event) override;

    bool handleKeyDown(const KeyEvent& event) override;
    bool handleKeyUp(const KeyEvent& event) override;
    bool handleTextInput(const TextInputEvent& event) override;
    
    bool canBeFocused() const override;
    std::string getFocusKey() const override;
    void notifyFocusGained() override;
    void notifyFocusLost() override;

    void transferStateFrom(const ViewInterface& old) override;

    std::string getKey() const override;

    void setPropertyOwner(Element* owner) override;
    
    std::optional<CursorType> getCursor() const override;

    void onMounted() override;
    void onUnmounted() override;

    std::string getSelectedText() const override;
    std::string cutSelectedText() override;
    void selectAll() override;

    std::string getTextContent() const override;
    std::string getAccessibleValue() const override;
};

// =============================================================================
// ViewAdapter<T> method implementations
// =============================================================================

template<ViewComponent T>
inline LayoutNode ViewAdapter<T>::layout(RenderContext& ctx, const Rect& bounds) const {
    if constexpr (has_layout<T>::value) {
        return component.layout(ctx, bounds);
    } else {
        LayoutNode node(View(component), bounds);
        node.environment = ctx.environment();

        if constexpr (has_body<T>::value) {
            node.resolvedBody = getCachedBody();
        }

        if constexpr (has_children_property<T>::value) {
            node.resolvedChildren = component.children;
        }

        std::vector<LayoutNode> childNodes;

        EdgeInsets componentPadding = component.padding;
        Rect contentBounds = {
            bounds.x + componentPadding.left,
            bounds.y + componentPadding.top,
            bounds.width - componentPadding.horizontal(),
            bounds.height - componentPadding.vertical()
        };

        if (node.resolvedBody.has_value() && node.resolvedBody->isValid()) {
            LayoutNode bodyLayout = node.resolvedBody->layout(ctx, contentBounds);
            childNodes.push_back(std::move(bodyLayout));
        }

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
    if constexpr (has_body<T>::value) {
        uint64_t gen = currentBodyGeneration();
        if (!cachedBody_ || cachedBodyGen_ != gen) {
            cachedBody_ = std::make_unique<View>(component.body());
            cachedBodyGen_ = gen;
        }
    } else if (!cachedBody_) {
        cachedBody_ = std::make_unique<View>();
    }
    return *cachedBody_;
}

template<ViewComponent T>
inline View ViewAdapter<T>::body() const {
    return getCachedBody();
}

template<ViewComponent T>
inline void ViewAdapter<T>::render(RenderContext& ctx, const Rect& bounds) const {
    if constexpr (has_render<T>::value) {
        component.render(ctx, bounds);
    }
    if constexpr (has_body<T>::value) {
        const View& bodyView = getCachedBody();
        if (bodyView.isValid()) {
            bodyView.render(ctx, bounds);
        }
    } else if constexpr (!has_render<T>::value) {
        ViewHelpers::renderView(component, ctx, bounds);
    }
}

template<ViewComponent T>
inline Size ViewAdapter<T>::preferredSize(TextMeasurement& textMeasurer) const {
    if constexpr (has_preferredSize<T>::value) {
        return component.preferredSize(textMeasurer);
    } else if constexpr (has_body<T>::value) {
        const View& bodyView = getCachedBody();
        if (bodyView.isValid()) {
            return bodyView.preferredSize(textMeasurer);
        }
        EdgeInsets paddingVal = component.padding;
        return {paddingVal.horizontal(), paddingVal.vertical()};
    } else {
        EdgeInsets paddingVal = component.padding;
        return {paddingVal.horizontal(), paddingVal.vertical()};
    }
}

template<ViewComponent T>
inline float ViewAdapter<T>::heightForWidth(float width, TextMeasurement& textMeasurer) const {
    if constexpr (has_heightForWidth<T>::value) {
        return component.heightForWidth(width, textMeasurer);
    } else if constexpr (has_body<T>::value) {
        const View& bodyView = getCachedBody();
        if (bodyView.isValid()) {
            return bodyView.heightForWidth(width, textMeasurer);
        }
        EdgeInsets paddingVal = component.padding;
        return paddingVal.vertical();
    } else {
        return preferredSize(textMeasurer).height;
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
inline VisualStyle ViewAdapter<T>::getVisualStyle() const {
    VisualStyle s;
    s.opacity = component.opacity;
    s.backgroundColor = component.backgroundColor;
    s.borderColor = component.borderColor;
    s.borderWidth = component.borderWidth;
    s.cornerRadius = component.cornerRadius;
    s.padding = component.padding;
    if constexpr (decltype(ViewHelpers::detail::hasRotation<T>(0))::value)
        s.rotation = component.rotation;
    if constexpr (decltype(ViewHelpers::detail::hasScaleX<T>(0))::value)
        s.scaleX = component.scaleX;
    if constexpr (decltype(ViewHelpers::detail::hasScaleY<T>(0))::value)
        s.scaleY = component.scaleY;
    if constexpr (decltype(ViewHelpers::detail::hasOffset<T>(0))::value)
        s.offset = component.offset;
    return s;
}

template<ViewComponent T>
inline LayoutConstraints ViewAdapter<T>::getLayoutConstraints() const {
    LayoutConstraints lc;
    lc.visible = component.visible;
    lc.clip = component.clip;
    lc.expansionBias = component.expansionBias;
    lc.compressionBias = component.compressionBias;
    lc.minWidth = component.minWidth.get();
    lc.maxWidth = component.maxWidth.get();
    lc.minHeight = component.minHeight.get();
    lc.maxHeight = component.maxHeight.get();
    lc.colspan = component.colspan;
    lc.rowspan = component.rowspan;

    if constexpr (has_body<T>::value) {
        const View& bodyView = getCachedBody();
        if (bodyView.isValid()) {
            LayoutConstraints bodyLc = bodyView->getLayoutConstraints();
            if (lc.visible == true) lc.visible = bodyLc.visible;
            if (lc.clip == false) lc.clip = bodyLc.clip;
            if (lc.expansionBias == 0.0f) lc.expansionBias = bodyLc.expansionBias;
            if (lc.compressionBias == 1.0f) lc.compressionBias = bodyLc.compressionBias;
            if (!lc.minWidth) lc.minWidth = bodyLc.minWidth;
            if (!lc.maxWidth) lc.maxWidth = bodyLc.maxWidth;
            if (!lc.minHeight) lc.minHeight = bodyLc.minHeight;
            if (!lc.maxHeight) lc.maxHeight = bodyLc.maxHeight;
            if (lc.colspan == 1) lc.colspan = bodyLc.colspan;
            if (lc.rowspan == 1) lc.rowspan = bodyLc.rowspan;
        }
    }

    return lc;
}

template<ViewComponent T>
inline std::vector<View> ViewAdapter<T>::getChildren() const {
    if constexpr (has_children_property<T>::value) {
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
inline bool ViewAdapter<T>::capturePointerEvent(PointerEvent& event) {
    if constexpr (has_capturePointerEvent<T>::value) {
        return component.capturePointerEvent(event);
    }
    return false;
}

template<ViewComponent T>
inline std::optional<CursorType> ViewAdapter<T>::getCursor() const {
    return component.cursor;
}

template<ViewComponent T>
inline bool ViewAdapter<T>::handleKeyDown(const KeyEvent& event) {
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
inline std::string ViewAdapter<T>::getKey() const {
    return component.key;
}

template<ViewComponent T>
inline void ViewAdapter<T>::setPropertyOwner(Element* owner) {
    component.padding.setOwner(owner);
    component.backgroundColor.setOwner(owner);
    component.backgroundImage.setOwner(owner);
    component.borderColor.setOwner(owner);
    component.borderWidth.setOwner(owner);
    component.cornerRadius.setOwner(owner);
    component.opacity.setOwner(owner);
    component.visible.setOwner(owner);
    component.clip.setOwner(owner);
    component.expansionBias.setOwner(owner);
    component.compressionBias.setOwner(owner);
    component.minWidth.setOwner(owner);
    component.maxWidth.setOwner(owner);
    component.minHeight.setOwner(owner);
    component.maxHeight.setOwner(owner);
    component.colspan.setOwner(owner);
    component.rowspan.setOwner(owner);
    component.cursor.setOwner(owner);
    component.focusable.setOwner(owner);
    component.focusKey.setOwner(owner);
    component.key.setOwner(owner);
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
inline void ViewAdapter<T>::transferStateFrom(const ViewInterface& old) {
    if constexpr (has_transferState<T>::value) {
        auto* src = dynamic_cast<const ViewAdapter<T>*>(&old);
        if (src) component.transferState(src->component);
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

template<ViewComponent T>
inline std::string ViewAdapter<T>::getSelectedText() const {
    if constexpr (has_selection_state<T>::value) {
        if (component.selStart != component.selEnd) {
            std::string val = component.value;
            size_t sMin = std::min(component.selStart, component.selEnd);
            size_t sMax = std::max(component.selStart, component.selEnd);
            if (sMax <= val.size()) {
                return val.substr(sMin, sMax - sMin);
            }
        }
    }
    return "";
}

template<ViewComponent T>
inline std::string ViewAdapter<T>::cutSelectedText() {
    if constexpr (has_selection_state<T>::value) {
        if (component.selStart != component.selEnd) {
            std::string val = component.value;
            size_t sMin = std::min(component.selStart, component.selEnd);
            size_t sMax = std::max(component.selStart, component.selEnd);
            if (sMax <= val.size()) {
                std::string selected = val.substr(sMin, sMax - sMin);
                val.erase(sMin, sMax - sMin);
                component.caretPos = sMin;
                component.selStart = component.selEnd = sMin;
                component.value = val;
                if constexpr (has_onValueChange<T>::value) {
                    if (component.onValueChange) component.onValueChange(val);
                }
                return selected;
            }
        }
    }
    return "";
}

template<ViewComponent T>
inline void ViewAdapter<T>::selectAll() {
    if constexpr (has_selection_state<T>::value) {
        std::string val = component.value;
        component.selStart = 0;
        component.selEnd = component.caretPos = val.size();
        requestApplicationRedraw();
    }
}

template<ViewComponent T>
inline std::string ViewAdapter<T>::getTextContent() const {
    std::string result;
    if constexpr (has_text_string<T>::value) {
        result = static_cast<std::string>(component.text);
    }
    if constexpr (has_value_string<T>::value) {
        if (result.empty()) result = static_cast<std::string>(component.value);
    }
    if constexpr (has_label_string<T>::value) {
        std::string lbl = static_cast<std::string>(component.label);
        if (!lbl.empty()) {
            if (!result.empty()) result = lbl + ": " + result;
            else result = lbl;
        }
    }
    if constexpr (has_placeholder_string<T>::value) {
        if (result.empty()) {
            std::string ph = static_cast<std::string>(component.placeholder);
            if (!ph.empty()) result = ph;
        }
    }
    return result;
}

template<ViewComponent T>
inline std::string ViewAdapter<T>::getAccessibleValue() const {
    if constexpr (has_value_float<T>::value) {
        float v = static_cast<float>(component.value);
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.4g", v);
        return std::string(buf);
    }
    if constexpr (has_fontSize_float<T>::value && !has_value_float<T>::value) {
        float v = static_cast<float>(component.fontSize);
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.4g", v);
        return std::string(buf);
    }
    if constexpr (has_checked_bool<T>::value) {
        return static_cast<bool>(component.checked) ? "true" : "false";
    }
    if constexpr (has_isOn_bool<T>::value) {
        return static_cast<bool>(component.isOn) ? "true" : "false";
    }
    if constexpr (has_selected_bool<T>::value) {
        return static_cast<bool>(component.selected) ? "true" : "false";
    }
    return "";
}

} // namespace flux
