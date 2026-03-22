#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/LayoutTree.hpp>
#include <Flux/Core/Element.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Log.hpp>
#include <Flux/Core/FocusState.hpp>
#include <Flux/Core/EventTypes.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <Flux/Graphics/RenderCommandBuffer.hpp>
#include <Flux/Core/OverlayManager.hpp>

namespace flux {

// Forward declarations
class Window;

class Renderer {
private:
    RenderContext* renderContext_;
    View rootView_;
    Window* window_;  // Reference to window for cursor changes
    
    // Layout cache to avoid rebuilding on every mouse event
    mutable LayoutNode cachedLayoutTree_;
    mutable Rect cachedBounds_ = {0, 0, 0, 0};
    mutable bool layoutCacheValid_ = false;

    // Persistent element tree for identity and lifecycle
    std::unique_ptr<Element> rootElement_;

    // Render command buffer (populated each frame alongside immediate draws)
    RenderCommandBuffer commandBuffer_;

    // Hover tracking: list of views currently under the pointer (root to deepest)
    std::vector<View> hoveredViews_;

    // Bounds of the deepest hovered interactive view (for render-time queries)
    Rect hoveredBounds_{};
    bool hasHoveredView_ = false;

    // Bounds of the currently pressed view (mouseDown → set, mouseUp → clear)
    Rect pressedBounds_{};
    bool hasPressedView_ = false;

    bool cursorBlinkActive_ = false;

    // Mouse capture: when MouseDown is handled by an interactive view, all
    // subsequent MouseMove/MouseUp events are routed to the same view
    // (identified by its path in the layout tree) until MouseUp releases it.
    struct {
        std::vector<size_t> treePath;
        bool active = false;
        bool fromOverlay = false;
    } mouseCapture_;

    LayoutNode* findNodeByPath(LayoutNode& root, const std::vector<size_t>& path);

public:
    Renderer(RenderContext* ctx)
        : renderContext_(ctx), rootView_(), window_(nullptr) {}

    Renderer(RenderContext* ctx, View component)
        : renderContext_(ctx), rootView_(std::move(component)), window_(nullptr) {}

    void setWindow(Window* window) { window_ = window; }

    void renderFrame(const Rect& bounds);
    
    // Invalidate layout cache (called when UI state changes)
    void invalidateLayoutCache() {
        layoutCacheValid_ = false;
    }

    void handleEvent(const PointerEvent& event, const Rect& windowBounds);

    void setRootView(View component) {
        rootView_ = std::move(component);
        layoutCacheValid_ = false;
    }

    const LayoutNode& getCachedLayoutTree() const { return cachedLayoutTree_; }
    const OverlayManager& getOverlayManager() const { return overlayManager_; }
    bool hasValidLayout() const { return layoutCacheValid_; }
    bool isCursorBlinkActive() const { return cursorBlinkActive_; }
    const RenderCommandBuffer& lastCommandBuffer() const { return commandBuffer_; }

    OverlayManager& overlayManager() { return overlayManager_; }

private:
    void renderTree(LayoutNode& node, Element* element, Point parentOrigin = {0, 0});

    // Unified event pipeline: hit test → capture → target → bubble
    bool dispatchPointerEvent(LayoutNode& root, PointerEvent& event);
    void hitTest(const LayoutNode& node, const Point& point, std::vector<LayoutNode*>& path);
    bool dispatchPointerToView(View& view, PointerEvent& event, const Rect& bounds);
    
    // Collect cursor by traversing view hierarchy
    std::optional<CursorType> collectCursor(const LayoutNode& node, const Point& point, std::optional<CursorType> inheritedCursor);

    // Overlay rendering and event dispatch
    OverlayManager overlayManager_;
    void renderOverlays(const Rect& viewport);
    bool dispatchPointerEventToOverlays(PointerEvent& event);

    // Hover tracking
    void collectHoverPath(const LayoutNode& node, const Point& point, std::vector<View>& path);
    bool updateHoverState(const Point& point);

};

// Alias for clarity
using ImmediateModeRenderer = Renderer;

} // namespace flux
