#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Core/Environment.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Core/EventTypes.hpp>
#include <Flux/Animation/Animation.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <memory>
#include <optional>
#include <vector>
#include <functional>
#include <string>

namespace flux {

class View;
struct LayoutNode;
class Element;

std::string demangleTypeName(const char* mangledName);

// Grouped property structs — returned by a single virtual call each,
// replacing ~21 individual virtual property accessors.

struct VisualStyle {
    float opacity = 1.0f;
    Color backgroundColor = Colors::transparent;
    Color borderColor = Colors::transparent;
    float borderWidth = 0.0f;
    CornerRadius cornerRadius = {0, 0, 0, 0};
    float rotation = 0.0f;
    float scaleX = 1.0f;
    float scaleY = 1.0f;
    Point offset = {0, 0};
    EdgeInsets padding = {};
    std::optional<Animation> animation;
};

struct LayoutConstraints {
    bool visible = true;
    bool clip = false;
    float expansionBias = 0.0f;
    float compressionBias = 1.0f;
    std::optional<float> minWidth;
    std::optional<float> maxWidth;
    std::optional<float> minHeight;
    std::optional<float> maxHeight;
    int colspan = 1;
    int rowspan = 1;
};

class ViewInterface {
public:
    virtual ~ViewInterface() = default;

    // Core rendering / layout
    virtual LayoutNode layout(RenderContext& ctx, const Rect& bounds) const = 0;
    virtual View body() const = 0;
    virtual void render(RenderContext& ctx, const Rect& bounds) const = 0;
    virtual Size preferredSize(TextMeasurement& textMeasurer) const = 0;
    virtual float heightForWidth(float width, TextMeasurement& textMeasurer) const = 0;

    // Grouped property accessors (two virtual calls replace ~21)
    virtual VisualStyle getVisualStyle() const { return {}; }
    virtual LayoutConstraints getLayoutConstraints() const { return {}; }

    // Non-virtual convenience — layout constraints
    bool isVisible() const { return getLayoutConstraints().visible; }
    bool shouldClip() const { return getLayoutConstraints().clip; }
    float getExpansionBias() const { return getLayoutConstraints().expansionBias; }
    float getCompressionBias() const { return getLayoutConstraints().compressionBias; }
    std::optional<float> getMinWidth() const { return getLayoutConstraints().minWidth; }
    std::optional<float> getMaxWidth() const { return getLayoutConstraints().maxWidth; }
    std::optional<float> getMinHeight() const { return getLayoutConstraints().minHeight; }
    std::optional<float> getMaxHeight() const { return getLayoutConstraints().maxHeight; }
    int getColspan() const { return getLayoutConstraints().colspan; }
    int getRowspan() const { return getLayoutConstraints().rowspan; }

    // Non-virtual convenience — visual style
    std::optional<Animation> getAnimation() const { return getVisualStyle().animation; }
    float getOpacity() const { return getVisualStyle().opacity; }
    Color getBackgroundColor() const { return getVisualStyle().backgroundColor; }
    Color getBorderColor() const { return getVisualStyle().borderColor; }
    float getBorderWidth() const { return getVisualStyle().borderWidth; }
    CornerRadius getCornerRadius() const { return getVisualStyle().cornerRadius; }
    float getRotation() const { return getVisualStyle().rotation; }
    float getScaleX() const { return getVisualStyle().scaleX; }
    float getScaleY() const { return getVisualStyle().scaleY; }
    Point getOffset() const { return getVisualStyle().offset; }
    EdgeInsets getPadding() const { return getVisualStyle().padding; }

    // Identity
    virtual std::string getTypeName() const = 0;

    // Children
    virtual bool hasChildrenProperty() const = 0;
    virtual std::vector<View> getChildren() const = 0;

    // Mouse events
    virtual bool handleMouseDown(float x, float y, int button) { (void)x; (void)y; (void)button; return false; }
    virtual bool handleMouseUp(float x, float y, int button) { (void)x; (void)y; (void)button; return false; }
    virtual bool handleMouseMove(float x, float y) { (void)x; (void)y; return false; }
    virtual void handleMouseEnter() {}
    virtual void handleMouseLeave() {}
    virtual bool handleMouseScroll(float x, float y, float deltaX, float deltaY) { (void)x; (void)y; (void)deltaX; (void)deltaY; return false; }
    virtual bool isInteractive() const { return false; }

    // Pointer capture
    virtual bool capturePointerEvent(PointerEvent&) { return false; }
    
    // Keyboard events
    virtual bool handleKeyDown(const KeyEvent& event) { (void)event; return false; }
    virtual bool handleKeyUp(const KeyEvent& event) { (void)event; return false; }
    virtual bool handleTextInput(const TextInputEvent& event) { (void)event; return false; }
    
    // Focus
    virtual bool canBeFocused() const { return false; }
    virtual std::string getFocusKey() const { return ""; }
    virtual void notifyFocusGained() {}
    virtual void notifyFocusLost() {}

    // Reconciliation identity
    virtual std::string getKey() const { return ""; }

    // Property ownership
    virtual void setPropertyOwner(Element* owner) { (void)owner; }

    // Body cache invalidation (called by Element::markDirty on property change)
    virtual void markBodyDirty() {}
    
    // Cursor
    virtual std::optional<CursorType> getCursor() const = 0;

    // Lifecycle
    virtual void onMounted() {}
    virtual void onUnmounted() {}

    // Clipboard / selection
    virtual std::string getSelectedText() const { return ""; }
    virtual std::string cutSelectedText() { return ""; }
    virtual void selectAll() {}

    // Accessibility
    virtual std::string getTextContent() const { return ""; }
    virtual std::string getAccessibleValue() const { return ""; }
};

} // namespace flux
