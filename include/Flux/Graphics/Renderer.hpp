#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/LayoutTree.hpp>
#include <Flux/Core/Element.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Log.hpp>
#include <Flux/Core/FocusState.hpp>
#include <Flux/Graphics/RenderContext.hpp>

namespace flux {

// Forward declarations
class Window;

struct Event {
    enum Type {
        MouseMove,
        MouseDown,
        MouseUp,
        MouseScroll,
        KeyPress,
        KeyRelease,
        TextInput
    };

    Type type;
    union {
        struct { 
            float x, y; 
        } mouseMove;
        
        struct { 
            float x, y;
            int button;
        } mouseButton;
        
        struct {
            float x, y;
            float deltaX, deltaY;
        } mouseScroll;
        
        struct { 
            int key;
            uint32_t modifiers;
        } keyboard;
    };
    std::string text; // For TextInput events
};

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

    // Hover tracking: list of views currently under the pointer (root to deepest)
    std::vector<View> hoveredViews_;

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

    void handleEvent(const struct Event& event, const Rect& windowBounds);

    void setRootView(View component) {
        rootView_ = std::move(component);
        layoutCacheValid_ = false;
    }

    const LayoutNode& getCachedLayoutTree() const { return cachedLayoutTree_; }
    bool hasValidLayout() const { return layoutCacheValid_; }

private:
    void renderTree(LayoutNode& node, Point parentOrigin = {0, 0});

    bool findAndDispatchEvent(LayoutNode& node, const Event& event, const Point& point);
    
    // Collect cursor by traversing view hierarchy
    std::optional<CursorType> collectCursor(const LayoutNode& node, const Point& point, std::optional<CursorType> inheritedCursor);

    // Hover tracking
    void collectHoverPath(const LayoutNode& node, const Point& point, std::vector<View>& path);
    void updateHoverState(const Point& point);

    // Dispatch event to a specific view
    bool dispatchEventToView(const View& view, const Event& event, const Point& localPoint) {
        bool handled = false;
        
        switch (event.type) {
            case Event::MouseDown:
                // Need to make a mutable copy to call the handler
                {
                    View mutableView = view;
                    handled = mutableView.handleMouseDown(localPoint.x, localPoint.y, event.mouseButton.button);
                    if (handled) {
                        FLUX_LOG_DEBUG("[Renderer] MouseDown handled by %s at (%f, %f)", view.getTypeName().c_str(), localPoint.x, localPoint.y);
                    }
                }
                break;
                
            case Event::MouseUp:
                {
                    View mutableView = view;
                    handled = mutableView.handleMouseUp(localPoint.x, localPoint.y, event.mouseButton.button);
                }
                break;
                
            case Event::MouseMove:
                {
                    View mutableView = view;
                    handled = mutableView.handleMouseMove(localPoint.x, localPoint.y);
                }
                break;
                
            case Event::MouseScroll:
                {
                    View mutableView = view;
                    handled = mutableView.handleMouseScroll(
                        localPoint.x, 
                        localPoint.y, 
                        event.mouseScroll.deltaX,
                        event.mouseScroll.deltaY
                    );
                    if (handled) {
                        FLUX_LOG_DEBUG("[Renderer] MouseScroll handled by %s deltaX=%f deltaY=%f", view.getTypeName().c_str(), event.mouseScroll.deltaX, event.mouseScroll.deltaY);
                    }
                }
                break;
                
            default:
                break;
        }
        
        return handled;
    }
};

// Alias for clarity
using ImmediateModeRenderer = Renderer;

} // namespace flux
