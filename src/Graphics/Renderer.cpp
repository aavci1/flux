#include <Flux/Graphics/Renderer.hpp>
#include <Flux/Graphics/NanoVGRenderContext.hpp>
#include <Flux/Core/Window.hpp>
#include <Flux/Core/FocusState.hpp>
#include <Flux/Core/Element.hpp>
#include <Flux/Core/LayoutTree.hpp>
#include <Flux/Core/Runtime.hpp>
#include <optional>

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
        // Store directly in the cache so View* pointers registered during
        // renderTree remain valid between frames.
        cachedLayoutTree_ = rootView_->layout(*renderContext_, bounds);
        cachedBounds_ = bounds;
        layoutCacheValid_ = true;

        // Reconcile persistent element tree
        if (!rootElement_) {
            rootElement_ = Element::buildTree(cachedLayoutTree_);
        } else {
            rootElement_->reconcile(cachedLayoutTree_);
        }

        // Set the global focused/hovered/pressed keys in the render context
        if (window_) {
            auto* nvgContext = static_cast<NanoVGRenderContext*>(renderContext_);
            nvgContext->globalFocusedKey_ = window_->focus().getFocusedKey();
        }
        if (hasHoveredView_) {
            renderContext_->setHoveredBounds(hoveredBounds_);
        } else {
            renderContext_->setHoveredBounds({});
        }
        if (hasPressedView_) {
            renderContext_->setPressedBounds(pressedBounds_);
        } else {
            renderContext_->clearPressedBounds();
        }

        // Render the tree (this also registers focusable views)
        renderTree(cachedLayoutTree_);

        // Process pending keyboard/text events after focusable views are registered
        if (window_) {
            window_->processPendingEvents(cachedLayoutTree_);
        }
    }

    // Present the frame
    renderContext_->present();

    // If a text input/area is focused, keep redrawing for cursor blink
    if (window_ && !window_->focus().getFocusedKey().empty()) {
        requestApplicationRedraw();
    }
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

bool Renderer::findAndDispatchEvent(LayoutNode& node, const Event& event, const Point& point) {
    for (size_t i = node.children.size(); i > 0; --i) {
        size_t idx = i - 1;
        if (node.children[idx].bounds.contains(point)) {
            mouseCapture_.treePath.push_back(idx);
            if (findAndDispatchEvent(node.children[idx], event, point)) {
                return true;
            }
            mouseCapture_.treePath.pop_back();
        }
    }

    if (node.bounds.contains(point) && node.view.isInteractive()) {
        Point localPoint = {point.x - node.bounds.x, point.y - node.bounds.y};
        bool handled = dispatchEventToView(node.view, event, localPoint);
        if (handled && event.type == Event::MouseDown) {
            mouseCapture_.active = true;
            pressedBounds_ = node.bounds;
            hasPressedView_ = true;
        }
        return handled;
    }

    return false;
}

LayoutNode* Renderer::findNodeByPath(LayoutNode& root, const std::vector<size_t>& path) {
    LayoutNode* current = &root;
    for (size_t idx : path) {
        if (idx >= current->children.size()) return nullptr;
        current = &current->children[idx];
    }
    return current;
}

void Renderer::renderTree(LayoutNode& node, Point parentOrigin) {
    // Register focusable views and capture the assigned key
    std::string assignedFocusKey;
    if (window_ && node.view.canBeFocused()) {
        assignedFocusKey = window_->focus().registerFocusableView(
            &node.view,
            node.bounds
        );
    }

    renderContext_->save();

    float relX = node.bounds.x - parentOrigin.x;
    float relY = node.bounds.y - parentOrigin.y;
    renderContext_->translate(relX, relY);

    Rect localBounds = {0, 0, node.bounds.width, node.bounds.height};

    if (node.view.shouldClip()) {
        Path clipPath;
        clipPath.rect(localBounds);
        renderContext_->clipPath(clipPath);
    }

    // Use the key returned by registerFocusableView (includes auto-generated keys)
    renderContext_->setCurrentFocusKey(assignedFocusKey.empty() ? node.view.getFocusKey() : assignedFocusKey);
    renderContext_->setCurrentViewGlobalBounds(node.bounds);

    // Render the view with local coordinates
    node.view->render(*renderContext_, localBounds);

    // Recursively render children with current view's position as their parent origin
    Point currentOrigin = {node.bounds.x, node.bounds.y};
    for (auto& child : node.children) {
        renderTree(child, currentOrigin);
    }

    // Restore the rendering state
    renderContext_->restore();
}

void Renderer::handleEvent(const struct Event& event, const Rect& windowBounds) {
    if (!rootView_.isValid()) {
        return;
    }

    Point eventPoint;
    
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
            return;
    }

    if (!windowBounds.contains(eventPoint)) {
        return;
    }

    if (!layoutCacheValid_ || 
        cachedBounds_.x != windowBounds.x || cachedBounds_.y != windowBounds.y ||
        cachedBounds_.width != windowBounds.width || cachedBounds_.height != windowBounds.height) {
        cachedLayoutTree_ = rootView_.layout(*renderContext_, windowBounds);
        cachedBounds_ = windowBounds;
        layoutCacheValid_ = true;
    }
    
    if (event.type == Event::MouseMove) {
        updateHoverState(eventPoint);
        if (window_) {
            std::optional<CursorType> cursor = collectCursor(cachedLayoutTree_, eventPoint, CursorType::Default);
            window_->setCursor(cursor.value_or(CursorType::Default));
        }
    }
    
    if (event.type == Event::MouseDown && window_) {
        window_->focus().focusViewAtPoint(eventPoint);
    }

    if (event.type == Event::MouseUp) {
        hasPressedView_ = false;
    }

    // During an active mouse capture (drag), route move/up to the
    // originally-pressed view instead of doing hit-testing.
    if (mouseCapture_.active && (event.type == Event::MouseMove || event.type == Event::MouseUp)) {
        LayoutNode* captured = findNodeByPath(cachedLayoutTree_, mouseCapture_.treePath);
        if (captured && captured->view.isInteractive()) {
            if (event.type == Event::MouseMove) {
                pressedBounds_ = captured->bounds;
                hasPressedView_ = true;
            }
            Point localPoint = {eventPoint.x - captured->bounds.x, eventPoint.y - captured->bounds.y};
            dispatchEventToView(captured->view, event, localPoint);
        }
        if (event.type == Event::MouseUp) {
            mouseCapture_.active = false;
        }
    } else if (event.type != Event::MouseMove) {
        mouseCapture_.treePath.clear();
        findAndDispatchEvent(cachedLayoutTree_, event, eventPoint);
    }

    requestApplicationRedraw();
}

void Renderer::collectHoverPath(const LayoutNode& node, const Point& point, std::vector<View>& path) {
    if (!node.bounds.contains(point)) return;
    path.push_back(node.view);
    if (node.view.isInteractive()) {
        hoveredBounds_ = node.bounds;
        hasHoveredView_ = true;
    }
    for (auto it = node.children.rbegin(); it != node.children.rend(); ++it) {
        if (it->bounds.contains(point)) {
            collectHoverPath(*it, point, path);
            return;
        }
    }
}

void Renderer::updateHoverState(const Point& point) {
    if (!layoutCacheValid_) return;

    hasHoveredView_ = false;
    std::vector<View> newPath;
    collectHoverPath(cachedLayoutTree_, point, newPath);

    size_t commonLen = 0;
    size_t minLen = std::min(hoveredViews_.size(), newPath.size());
    while (commonLen < minLen &&
           hoveredViews_[commonLen].getTypeName() == newPath[commonLen].getTypeName()) {
        ++commonLen;
    }

    for (size_t i = hoveredViews_.size(); i > commonLen; --i) {
        View v = hoveredViews_[i - 1];
        v.handleMouseLeave();
    }

    for (size_t i = commonLen; i < newPath.size(); ++i) {
        View v = newPath[i];
        v.handleMouseEnter();
    }

    hoveredViews_ = std::move(newPath);
}

} // namespace flux

