#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/LayoutTree.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Graphics/RenderContext.hpp>

namespace flux {

// Forward declaration
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

public:
    Renderer(RenderContext* ctx)
        : renderContext_(ctx), rootView_(), window_(nullptr) {}

    Renderer(RenderContext* ctx, View component)
        : renderContext_(ctx), rootView_(std::move(component)), window_(nullptr) {}

    void setWindow(Window* window) { window_ = window; }

    void renderFrame(const Rect& bounds) {
        if (!renderContext_) return;

        // Begin the frame
        renderContext_->beginFrame();

        // Clear the frame buffer
        renderContext_->clear(Color(1, 1, 1, 1));

        // Build layout tree and render using framework approach
        if (rootView_.operator->()) {
            // Layout the root view with RenderContext for accurate measurements
            LayoutNode layoutTree = rootView_->layout(*renderContext_, bounds);

            // Debug: Print layout tree before rendering
            std::cout << "\n=== Layout Tree ===" << std::endl;
            printLayoutTree(layoutTree);
            std::cout << "==================\n" << std::endl;

            // Render the tree by traversing it
            renderTree(layoutTree);
        }

        // Present the frame
        renderContext_->present();
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

        // Build layout tree and find + dispatch to the target view in one pass
        LayoutNode layoutTree = rootView_.layout(*renderContext_, windowBounds);
        findAndDispatchEvent(layoutTree, event, eventPoint);
    }

    void setRootView(View component) {
        rootView_ = std::move(component);
    }

private:
    void renderTree(const LayoutNode& node, Point parentOrigin = {0, 0}) {
        // Save the current rendering state
        renderContext_->save();

        // Calculate position relative to parent
        float relX = node.bounds.x - parentOrigin.x;
        float relY = node.bounds.y - parentOrigin.y;

        // Translate coordinate system to the view's position
        renderContext_->translate(relX, relY);

        // Create local bounds (view renders in its own coordinate system)
        Rect localBounds = {0, 0, node.bounds.width, node.bounds.height};

        // Clip rendering to the view's bounds if the view has clipping enabled
        if (node.view.shouldClip()) {
            Path clipPath;
            clipPath.rect(localBounds);
            renderContext_->clipPath(clipPath);
        }

        // Render the view with local coordinates
        node.view->render(*renderContext_, localBounds);

        // Recursively render children with current view's position as their parent origin
        Point currentOrigin = {node.bounds.x, node.bounds.y};
        for (const auto& child : node.children) {
            renderTree(child, currentOrigin);
        }

        // Restore the rendering state
        renderContext_->restore();
    }

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
