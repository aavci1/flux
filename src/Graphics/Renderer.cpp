#include <Flux/Graphics/Renderer.hpp>
#include <Flux/Graphics/NanoVGRenderContext.hpp>
#include <Flux/Core/Window.hpp>
#include <Flux/Core/FocusState.hpp>
#include <Flux/Core/LayoutTree.hpp>
#include <optional>
#include <iostream>

namespace flux {

void Renderer::renderFrame(const Rect& bounds) {
    if (!renderContext_) return;

    // Begin the frame
    renderContext_->beginFrame();

    // Clear the frame buffer
    renderContext_->clear(Color(1, 1, 1, 1));

    // Build layout tree and render using framework approach
    if (rootView_.operator->()) {
        // Clear focusable views from previous frame
        if (window_) {
            window_->focus().clearFocusableViews();
        }

        // Layout the root view with RenderContext for accurate measurements
        LayoutNode layoutTree = rootView_->layout(*renderContext_, bounds);
        
        // Update layout cache for mouse event handling
        cachedLayoutTree_ = layoutTree;
        cachedBounds_ = bounds;
        layoutCacheValid_ = true;

        // Debug printing removed - extremely expensive with many views
        // std::cout << "\n=== Layout Tree ===" << std::endl;
        // printLayoutTree(layoutTree);
        // std::cout << "==================\n" << std::endl;

        // Set the global focused key in the render context for this frame
        // This allows views to check if they have focus during rendering
        if (window_) {
            auto* nvgContext = static_cast<NanoVGRenderContext*>(renderContext_);
            nvgContext->globalFocusedKey_ = window_->focus().getFocusedKey();
        }

        // Process pending keyboard events now that we have the layout tree
        // This is safe because the View objects in the layout tree are valid for this frame
        if (window_) {
            window_->processPendingEvents(layoutTree);
        }

        // Render the tree by traversing it
        renderTree(layoutTree);
    }

    // Present the frame
    renderContext_->present();
}

// Helper to collect the cursor during traversal
std::optional<CursorType> Renderer::collectCursor(const LayoutNode& node, const Point& point, std::optional<CursorType> inheritedCursor) {
    // Start with the inherited cursor from parent
    std::optional<CursorType> currentCursor = inheritedCursor;
    
    // If this node contains the point, update cursor based on this view
    if (node.bounds.contains(point)) {
        std::optional<CursorType> viewCursor = node.view.getCursor();
        
        // If this view has an explicit cursor, use it
        if (viewCursor.has_value()) {
            currentCursor = viewCursor;
        }
        // If this view is interactive but has no explicit cursor, use Pointer
        else if (node.view.isInteractive()) {
            currentCursor = CursorType::Pointer;
        }
        // Otherwise keep inherited cursor
        
        // Check children (in reverse order so topmost views are prioritized)
        for (auto it = node.children.rbegin(); it != node.children.rend(); ++it) {
            if (it->bounds.contains(point)) {
                // Recurse with current cursor as inherited
                std::optional<CursorType> childCursor = collectCursor(*it, point, currentCursor);
                if (childCursor.has_value()) {
                    return childCursor;  // Child determined the cursor
                }
            }
        }
        
        // No child determined cursor, return what we have
        return currentCursor;
    }
    
    return std::nullopt;  // Point not in this node
}

bool Renderer::findAndDispatchEvent(const LayoutNode& node, const Event& event, const Point& point) {
    // Search children first (reverse order so topmost views are checked first)
    for (auto it = node.children.rbegin(); it != node.children.rend(); ++it) {
        if (it->bounds.contains(point)) {
            if (findAndDispatchEvent(*it, event, point)) {
                return true;  // Event was handled by a child
            }
        }
    }

    // Check if the current view contains the point
    if (node.bounds.contains(point)) {
        // Dispatch click events to interactive views
        if (node.view.isInteractive()) {
            return dispatchEventToView(node.view, event, point);
        }
    }

    return false;
}

void Renderer::renderTree(const LayoutNode& node, Point parentOrigin) {
    // Register focusable views with FocusState
    if (window_ && node.view.canBeFocused()) {
        window_->focus().registerFocusableView(
            const_cast<View*>(&node.view), 
            node.bounds
        );
    }

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

    // Set the current view's focus key so it can check if it's focused
    renderContext_->setCurrentFocusKey(node.view.getFocusKey());

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

void Renderer::handleEvent(const struct Event& event, const Rect& windowBounds) {
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
    
    // For mouse move events, collect and set the cursor
    if (event.type == Event::MouseMove && window_) {
        // Traverse the view hierarchy once, collecting the cursor
        // Start with Default as the root cursor
        std::optional<CursorType> cursor = collectCursor(cachedLayoutTree_, eventPoint, CursorType::Default);
        
        // Set the resolved cursor (default to Default if no view specified)
        window_->setCursor(cursor.value_or(CursorType::Default));
    }
    
    // Dispatch other events
    findAndDispatchEvent(cachedLayoutTree_, event, eventPoint);
}

} // namespace flux

