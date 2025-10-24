#include <Flux/Graphics/Renderer.hpp>
#include <Flux/Core/Window.hpp>

namespace flux {

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

} // namespace flux

