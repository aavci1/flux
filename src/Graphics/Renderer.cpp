#include <Flux/Graphics/Renderer.hpp>
#include <Flux/Graphics/NanoVGRenderContext.hpp>
#include <Flux/Core/Window.hpp>
#include <Flux/Core/FocusState.hpp>
#include <Flux/Core/LayoutTree.hpp>
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
        // Update cursor based on the view's cursor property (for mouse move events)
        if (event.type == Event::MouseMove) {
            updateCursorForView(node.view, event);
        }
        
        // Dispatch click events to interactive views
        if (node.view.isInteractive()) {
            return dispatchEventToView(node.view, event, point);
        }
    }

    return false;
}

void Renderer::updateCursorForView(const View& view, const Event& event) {
    if (!window_) return;
    
    CursorType viewCursor = view.getCursor();
    if (viewCursor != CursorType::Default || view.isInteractive()) {
        // Use view's cursor, or Pointer for interactive views without explicit cursor
        CursorType cursorToSet = (viewCursor != CursorType::Default) ? 
            viewCursor : 
            (view.isInteractive() ? CursorType::Pointer : CursorType::Default);
        window_->setCursor(cursorToSet);
    }
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

} // namespace flux

