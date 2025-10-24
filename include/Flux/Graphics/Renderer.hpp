#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/LayoutTree.hpp>
#include <Flux/Core/Types.hpp>
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

    void handleEvent(const struct Event& event, const Rect& windowBounds) {
        // Handle input events by finding the target view and dispatching to it
        if (!rootView_.isValid()) {
            return;
        }

        Point eventPoint;
        
        // Extract coordinates based on event type
        switch (event.type) {
            case Event::MouseMove:
                eventPoint = {event.mouseMove.x, event.mouseMove.y};
                break;
            case Event::MouseDown:
            case Event::MouseUp:
                eventPoint = {event.mouseButton.x, event.mouseButton.y};
                break;
            case Event::MouseScroll:
                eventPoint = {event.mouseScroll.x, event.mouseScroll.y};
                break;
            default:
                return;  // Don't process events without coordinates
        }

        // Check if the event point is within window bounds
        if (!windowBounds.contains(eventPoint)) {
            return;  // Event is outside the window
        }

        // Use cached layout tree if valid and bounds match, otherwise rebuild
        if (!layoutCacheValid_ || 
            cachedBounds_.x != windowBounds.x || cachedBounds_.y != windowBounds.y ||
            cachedBounds_.width != windowBounds.width || cachedBounds_.height != windowBounds.height) {
            cachedLayoutTree_ = rootView_.layout(*renderContext_, windowBounds);
            cachedBounds_ = windowBounds;
            layoutCacheValid_ = true;
        }
        
        findAndDispatchEvent(cachedLayoutTree_, event, eventPoint);
    }

    void setRootView(View component) {
        rootView_ = std::move(component);
        layoutCacheValid_ = false;  // Invalidate cache when root view changes
    }

private:
    void renderTree(const LayoutNode& node, Point parentOrigin = {0, 0});

    // Find and dispatch event to the deepest interactive view in the layout tree
    bool findAndDispatchEvent(const LayoutNode& node, const Event& event, const Point& point);
    
    // Helper to update cursor based on view
    void updateCursorForView(const View& view, const Event& event);

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
                        std::cout << "[Renderer] MouseDown handled by " << view.getTypeName() 
                                  << " at (" << localPoint.x << ", " << localPoint.y << ")\n";
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
                
            default:
                break;
        }
        
        return handled;
    }
};

// Alias for clarity
using ImmediateModeRenderer = Renderer;

} // namespace flux
