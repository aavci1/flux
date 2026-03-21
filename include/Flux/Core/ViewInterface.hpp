#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Core/Environment.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Core/EventTypes.hpp>
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

class ViewInterface {
public:
    virtual ~ViewInterface() = default;

    // Core methods - now with default implementations
    virtual LayoutNode layout(RenderContext& ctx, const Rect& bounds) const = 0;

    virtual View body() const = 0;

    virtual void render(RenderContext& ctx, const Rect& bounds) const = 0;

    virtual Size preferredSize(TextMeasurement& textMeasurer) const = 0;

    virtual float heightForWidth(float width, TextMeasurement& textMeasurer) const = 0;

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

    // Event handling methods (target + bubble phase)
    virtual bool handleMouseDown(float x, float y, int button) { (void)x; (void)y; (void)button; return false; }
    virtual bool handleMouseUp(float x, float y, int button) { (void)x; (void)y; (void)button; return false; }
    virtual bool handleMouseMove(float x, float y) { (void)x; (void)y; return false; }
    virtual void handleMouseEnter() {}
    virtual void handleMouseLeave() {}
    virtual bool handleMouseScroll(float x, float y, float deltaX, float deltaY) { (void)x; (void)y; (void)deltaX; (void)deltaY; return false; }
    virtual bool isInteractive() const { return false; }

    // Capture phase — called root→target before the target handler.
    // Return true to stop propagation before it reaches the target.
    virtual bool capturePointerEvent(PointerEvent&) { return false; }

    // Keyboard event handling methods
    virtual bool handleKeyDown(const KeyEvent& event) { (void)event; return false; }
    virtual bool handleKeyUp(const KeyEvent& event) { (void)event; return false; }
    virtual bool handleTextInput(const TextInputEvent& event) { (void)event; return false; }

    // Focus management
    virtual bool canBeFocused() const { return false; }
    virtual std::string getFocusKey() const { return ""; }
    virtual void notifyFocusGained() {}
    virtual void notifyFocusLost() {}

    // Reconciliation identity
    virtual std::string getKey() const { return ""; }

    // Property ownership for targeted dirty marking
    virtual void setPropertyOwner(Element* owner) { (void)owner; }

    // Cursor management
    virtual std::optional<CursorType> getCursor() const = 0;

    // Lifecycle
    virtual void onMounted() {}
    virtual void onUnmounted() {}

    // Clipboard / selection
    virtual std::string getSelectedText() const { return ""; }
    virtual std::string cutSelectedText() { return ""; }
    virtual void selectAll() {}

    // Testing/accessibility introspection
    virtual std::string getTextContent() const { return ""; }
    virtual std::string getAccessibleValue() const { return ""; }
};

} // namespace flux
