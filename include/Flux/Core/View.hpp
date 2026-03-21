#pragma once

// Split header umbrella — includes the sub-headers in dependency order.

#include <Flux/Core/ViewInterface.hpp>
#include <Flux/Core/ViewTraits.hpp>

#include <memory>
#include <optional>
#include <vector>
#include <string>

namespace flux {

// ============================================================================
// View — type-erased view container that supports any component type
// ============================================================================

class View {
private:
    std::shared_ptr<ViewInterface> component_;

public:
    View() : component_(nullptr) {}

    template<typename T>
    requires ViewComponent<std::remove_cvref_t<T>>
    View(T&& component);

    View(const View&) = default;
    View& operator=(const View&) = default;
    View(View&&) = default;
    View& operator=(View&&) = default;

    LayoutNode layout(RenderContext& ctx, const Rect& bounds) const;

    void render(RenderContext& ctx, const Rect& bounds) const {
        if (component_) component_->render(ctx, bounds);
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        return component_ ? component_->preferredSize(textMeasurer) : Size{};
    }

    float heightForWidth(float width, TextMeasurement& textMeasurer) const {
        return component_ ? component_->heightForWidth(width, textMeasurer) : 0.0f;
    }

    VisualStyle getVisualStyle() const {
        return component_ ? component_->getVisualStyle() : VisualStyle{};
    }

    LayoutConstraints getLayoutConstraints() const {
        return component_ ? component_->getLayoutConstraints() : LayoutConstraints{};
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

    bool capturePointerEvent(PointerEvent& event) {
        return component_ ? component_->capturePointerEvent(event) : false;
    }

    bool isInteractive() const {
        return component_ ? component_->isInteractive() : false;
    }

    bool handleKeyDown(const KeyEvent& event) {
        return component_ ? component_->handleKeyDown(event) : false;
    }

    bool handleKeyUp(const KeyEvent& event) {
        return component_ ? component_->handleKeyUp(event) : false;
    }

    bool handleTextInput(const TextInputEvent& event) {
        return component_ ? component_->handleTextInput(event) : false;
    }

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

    std::string getKey() const {
        return component_ ? component_->getKey() : "";
    }

    void setPropertyOwner(Element* owner) {
        if (component_) component_->setPropertyOwner(owner);
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

    std::string getSelectedText() const {
        return component_ ? component_->getSelectedText() : "";
    }

    std::string cutSelectedText() {
        return component_ ? component_->cutSelectedText() : "";
    }

    void selectAll() {
        if (component_) component_->selectAll();
    }

    std::string getTextContent() const {
        return component_ ? component_->getTextContent() : "";
    }

    std::string getAccessibleValue() const {
        return component_ ? component_->getAccessibleValue() : "";
    }

    std::optional<Animation> getAnimation() const {
        return component_ ? component_->getAnimation() : std::nullopt;
    }
    float getOpacity() const {
        return component_ ? component_->getOpacity() : 1.0f;
    }
    Color getBackgroundColor() const {
        return component_ ? component_->getBackgroundColor() : Colors::transparent;
    }
    Color getBorderColor() const {
        return component_ ? component_->getBorderColor() : Colors::transparent;
    }
    float getBorderWidth() const {
        return component_ ? component_->getBorderWidth() : 0.0f;
    }
    CornerRadius getCornerRadius() const {
        return component_ ? component_->getCornerRadius() : CornerRadius{0, 0, 0, 0};
    }
    float getRotation() const {
        return component_ ? component_->getRotation() : 0.0f;
    }
    float getScaleX() const {
        return component_ ? component_->getScaleX() : 1.0f;
    }
    float getScaleY() const {
        return component_ ? component_->getScaleY() : 1.0f;
    }
    Point getOffset() const {
        return component_ ? component_->getOffset() : Point{0, 0};
    }
    EdgeInsets getPadding() const {
        return component_ ? component_->getPadding() : EdgeInsets{};
    }

    ViewInterface* operator->() { return component_.get(); }
    const ViewInterface* operator->() const { return component_.get(); }
    ViewInterface& operator*() { return *component_; }
    const ViewInterface& operator*() const { return *component_; }
};

// ============================================================================
// LayoutNode — layout tree node with view, bounds, and children
// ============================================================================

struct LayoutNode {
    View view;
    Rect bounds;
    std::vector<LayoutNode> children;

    std::optional<View> resolvedBody;
    std::optional<std::vector<View>> resolvedChildren;

    std::optional<Environment> environment;

    LayoutNode() : view(), bounds() {}
    LayoutNode(const View& v, const Rect& b) : view(v), bounds(b) {}
    LayoutNode(const View& v, const Rect& b, std::vector<LayoutNode>&& c)
        : view(v), bounds(b), children(std::move(c)) {}
};

inline LayoutNode View::layout(RenderContext& ctx, const Rect& bounds) const {
    LayoutNode node = component_->layout(ctx, bounds);
    node.view = *this;
    if (!node.environment.has_value()) {
        node.environment = ctx.environment();
    }
    return node;
}

} // namespace flux

#define FLUX_VIEW_HPP_COMPLETE

// ViewAdapter template implementations require the full View and LayoutNode
// definitions above, so it must be included after them.
#include <Flux/Core/ViewAdapter.hpp>

namespace flux {

template<typename T>
requires ViewComponent<std::remove_cvref_t<T>>
View::View(T&& component)
    : component_(std::make_shared<ViewAdapter<std::remove_cvref_t<T>>>(std::forward<T>(component))) {}

} // namespace flux

#include <Flux/Core/LayoutTree.hpp>
