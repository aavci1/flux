#include <Flux/Graphics/Renderer.hpp>
#include <Flux/Graphics/RenderCommandBuffer.hpp>
#include <Flux/Core/Window.hpp>
#include <Flux/Core/FocusState.hpp>
#include <Flux/Core/Element.hpp>
#include <Flux/Core/LayoutTree.hpp>
#include <Flux/Core/Runtime.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/EventTypes.hpp>
#include <Flux/Core/Environment.hpp>
#include <Flux/Core/OverlayManager.hpp>
#include <optional>
#include <vector>

namespace flux {

void Renderer::renderFrame(const Rect& bounds) {
    if (!renderContext_) return;

    renderContext_->beginFrame();
    renderContext_->clearEnvironmentStack();

    if (rootView_.operator->()) {
        // Always drain keyboard/text BEFORE clearFocusableViews().
        // 1) Auto-generated focus keys (TextInput_0, …) change when the tree is rebuilt;
        //    getFocusedElement() only matches the list from the *previous* frame.
        // 2) requestRedraw() sets layoutCacheValid_ = false, so never gate this on that flag —
        //    otherwise every keystroke skips the early dispatch and input is dropped again.
        if (window_) {
            window_->processPendingEvents(cachedLayoutTree_);
        }

        if (window_) {
            window_->focus().clearFocusableViews();
        }

        suppressRedrawRequests();
        cachedLayoutTree_ = rootView_->layout(*renderContext_, bounds);
        resumeRedrawRequests();
        cachedBounds_ = bounds;
        layoutCacheValid_ = true;

        Environment rootEnv = cachedLayoutTree_.environment.value_or(Environment::defaults());
        renderContext_->clear(rootEnv.theme.background);

        suppressRedrawRequests();
        if (!rootElement_) {
            rootElement_ = Element::buildTree(cachedLayoutTree_);
        } else {
            rootElement_->reconcile(cachedLayoutTree_);
        }
        resumeRedrawRequests();

        // Set the global focused/hovered/pressed keys in the render context
        if (window_) {
            renderContext_->setGlobalFocusedKey(window_->focus().getFocusedKey());
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

        // Attach command buffer for recording, then render the tree (full buffer each frame;
        // incremental dirty-subtree compile would require retained per-element command slices).
        commandBuffer_.clear();
        commandBuffer_.reserve(512);
        renderContext_->setRecordingBuffer(&commandBuffer_);
        renderTree(cachedLayoutTree_, rootElement_.get());
        renderOverlays(bounds);
        renderContext_->setRecordingBuffer(nullptr);

        // Dispatch deferred focus/blur notifications now that views are valid
        if (window_) {
            window_->focus().dispatchPendingFocusNotifications();
        }
    } else {
        renderContext_->clear(Color(1, 1, 1, 1));
    }

    // Present the frame
    renderContext_->present();

    cursorBlinkActive_ = window_ && !window_->focus().getFocusedKey().empty();
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

LayoutNode* Renderer::findNodeByPath(LayoutNode& root, const std::vector<size_t>& path) {
    LayoutNode* current = &root;
    for (size_t idx : path) {
        if (idx >= current->children.size()) return nullptr;
        current = &current->children[idx];
    }
    return current;
}

void Renderer::hitTest(const LayoutNode& node, const Point& point, std::vector<LayoutNode*>& path) {
    if (!node.bounds.contains(point)) return;
    path.push_back(const_cast<LayoutNode*>(&node));
    for (auto it = node.children.rbegin(); it != node.children.rend(); ++it) {
        if (it->bounds.contains(point)) {
            hitTest(*it, point, path);
            return;
        }
    }
}

bool Renderer::dispatchPointerToView(View& view, PointerEvent& event, const Rect& bounds) {
    Point local = {event.windowPosition.x - bounds.x, event.windowPosition.y - bounds.y};
    event.localPosition = local;
    switch (event.kind) {
        case PointerEvent::Kind::Down:
            return view.handleMouseDown(local.x, local.y, event.button);
        case PointerEvent::Kind::Up:
            return view.handleMouseUp(local.x, local.y, event.button);
        case PointerEvent::Kind::Move:
            return view.handleMouseMove(local.x, local.y);
        case PointerEvent::Kind::Scroll:
            return view.handleMouseScroll(local.x, local.y, event.scrollDeltaX, event.scrollDeltaY);
        default:
            return false;
    }
}

bool Renderer::dispatchPointerEvent(LayoutNode& root, PointerEvent& event) {
    std::vector<LayoutNode*> path;
    hitTest(root, event.windowPosition, path);
    if (path.empty()) return false;

    // 1. CAPTURE — root → target (exclusive of target)
    event.phase = EventBase::Phase::Capture;
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        if (path[i]->view.capturePointerEvent(event)) {
            event.handled = true;
            return true;
        }
    }

    // 2. TARGET — deepest node in path
    event.phase = EventBase::Phase::Target;
    LayoutNode* target = path.back();

    // Record tree path for mouse capture (for drag continuation)
    mouseCapture_.treePath.clear();
    for (size_t i = 1; i < path.size(); ++i) {
        LayoutNode* parent = path[i - 1];
        for (size_t c = 0; c < parent->children.size(); ++c) {
            if (&parent->children[c] == path[i]) {
                mouseCapture_.treePath.push_back(c);
                break;
            }
        }
    }

    if (target->view.isInteractive()) {
        bool handled = dispatchPointerToView(target->view, event, target->bounds);
        if (handled) {
            if (event.kind == PointerEvent::Kind::Down) {
                mouseCapture_.active = true;
                pressedBounds_ = target->bounds;
                hasPressedView_ = true;
            }
            event.handled = true;
            return true;
        }
    }

    // 3. BUBBLE — target → root (skip target, walk back toward root)
    event.phase = EventBase::Phase::Bubble;
    for (int i = static_cast<int>(path.size()) - 2; i >= 0; --i) {
        if (path[i]->view.isInteractive()) {
            bool handled = dispatchPointerToView(path[i]->view, event, path[i]->bounds);
            if (handled) {
                if (event.kind == PointerEvent::Kind::Down) {
                    mouseCapture_.active = true;
                    pressedBounds_ = path[i]->bounds;
                    hasPressedView_ = true;
                    mouseCapture_.treePath.resize(i);
                }
                event.handled = true;
                return true;
            }
        }
    }

    return false;
}

void Renderer::renderTree(LayoutNode& node, Element* element, Point parentOrigin) {
    renderContext_->pushEnvironment(node.environment.value_or(Environment::defaults()));

    bool animated = element && element->hasActiveAnimations();

    Rect visBounds = animated
        ? element->getAnimatedValue<Rect>("bounds", node.bounds)
        : node.bounds;

    std::string assignedFocusKey;
    if (window_ && node.view.canBeFocused() && element) {
        assignedFocusKey = window_->focus().registerFocusableElement(
            element,
            visBounds
        );
    }

    renderContext_->save();

    float relX = visBounds.x - parentOrigin.x;
    float relY = visBounds.y - parentOrigin.y;
    renderContext_->translate(relX, relY);

    Rect localBounds = {0, 0, visBounds.width, visBounds.height};

    auto vs = node.view.getVisualStyle();

    if (node.view.shouldClip()) {
        Path clipPath;
        clipPath.rect(localBounds);
        renderContext_->clipPath(clipPath);
    }

    renderContext_->setCurrentFocusKey(assignedFocusKey.empty() ? node.view.getFocusKey() : assignedFocusKey);
    renderContext_->setCurrentViewGlobalBounds(visBounds);
    renderContext_->setCurrentElement(element);

    Point offsetPt = animated
        ? element->getAnimatedValue<Point>("offset", vs.offset)
        : vs.offset;
    if (offsetPt.x != 0 || offsetPt.y != 0)
        renderContext_->translate(offsetPt.x, offsetPt.y);

    float rot = animated
        ? element->getAnimatedValue<float>("rotation", vs.rotation)
        : vs.rotation;
    if (rot != 0) renderContext_->rotate(rot);

    float sx = animated
        ? element->getAnimatedValue<float>("scaleX", vs.scaleX)
        : vs.scaleX;
    float sy = animated
        ? element->getAnimatedValue<float>("scaleY", vs.scaleY)
        : vs.scaleY;
    if (sx != 1.0f || sy != 1.0f) renderContext_->scale(sx, sy);

    float opacityVal = animated
        ? element->getAnimatedValue<float>("opacity", vs.opacity)
        : vs.opacity;
    if (opacityVal < 1.0f)
        renderContext_->setOpacity(opacityVal);

    node.view->render(*renderContext_, localBounds);

    Point currentOrigin = {visBounds.x, visBounds.y};
    size_t elemChildCount = element ? element->children.size() : 0;
    for (size_t i = 0; i < node.children.size(); ++i) {
        Element* childElement = (i < elemChildCount) ? element->children[i].get() : nullptr;
        renderTree(node.children[i], childElement, currentOrigin);
    }

    renderContext_->restore();
    renderContext_->popEnvironment();
}

void Renderer::handleEvent(const PointerEvent& event, const Rect& windowBounds) {
    if (!rootView_.isValid()) {
        return;
    }

    Point eventPoint = event.windowPosition;

    if (!windowBounds.contains(eventPoint)) {
        return;
    }

    if (!layoutCacheValid_ ||
        cachedBounds_.x != windowBounds.x || cachedBounds_.y != windowBounds.y ||
        cachedBounds_.width != windowBounds.width || cachedBounds_.height != windowBounds.height) {
        suppressRedrawRequests();
        renderContext_->clearEnvironmentStack();
        cachedLayoutTree_ = rootView_.layout(*renderContext_, windowBounds);
        resumeRedrawRequests();
        cachedBounds_ = windowBounds;
        layoutCacheValid_ = true;
    }

    bool needsRedraw = false;
    const bool isMove  = event.kind == PointerEvent::Kind::Move;
    const bool isDown  = event.kind == PointerEvent::Kind::Down;
    const bool isUp    = event.kind == PointerEvent::Kind::Up;

    if (isMove) {
        bool hoverChanged = updateHoverState(eventPoint);
        if (window_) {
            std::optional<CursorType> cursor = collectCursor(cachedLayoutTree_, eventPoint, CursorType::Default);
            window_->setCursor(cursor.value_or(CursorType::Default));
        }
        needsRedraw = hoverChanged;
    }

    if (isDown && window_) {
        window_->focus().focusViewAtPoint(eventPoint);
        needsRedraw = true;
    }

    if (isUp) {
        hasPressedView_ = false;
        needsRedraw = true;
    }

    PointerEvent ptrEvent = event;

    if (!overlayManager_.empty() && (!mouseCapture_.active || mouseCapture_.fromOverlay)) {
        if (mouseCapture_.active && mouseCapture_.fromOverlay && (isMove || isUp)) {
            dispatchPointerEventToOverlays(ptrEvent);
            needsRedraw = true;
            if (isUp) {
                mouseCapture_.active = false;
                mouseCapture_.fromOverlay = false;
            }
            requestApplicationRedraw();
            return;
        }
        if (dispatchPointerEventToOverlays(ptrEvent)) {
            if (mouseCapture_.active) mouseCapture_.fromOverlay = true;
            needsRedraw = true;
            requestApplicationRedraw();
            return;
        }
    }

    if (mouseCapture_.active && (isMove || isUp)) {
        LayoutNode* captured = findNodeByPath(cachedLayoutTree_, mouseCapture_.treePath);
        if (captured && captured->view.isInteractive()) {
            if (isMove) {
                pressedBounds_ = captured->bounds;
                hasPressedView_ = true;
            }
            dispatchPointerToView(captured->view, ptrEvent, captured->bounds);
            needsRedraw = true;
        }
        if (isUp) {
            mouseCapture_.active = false;
            mouseCapture_.fromOverlay = false;
        }
    } else if (!isMove) {
        mouseCapture_.treePath.clear();
        if (dispatchPointerEvent(cachedLayoutTree_, ptrEvent)) {
            needsRedraw = true;
        }
    }

    if (needsRedraw) {
        requestApplicationRedraw();
    }
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

bool Renderer::updateHoverState(const Point& point) {
    if (!layoutCacheValid_) return false;

    hasHoveredView_ = false;
    std::vector<View> newPath;

    // Check overlays first (reverse order — topmost overlay has priority)
    for (auto it = overlayManager_.entries().rbegin(); it != overlayManager_.entries().rend(); ++it) {
        if (it->layoutTree.bounds.contains(point)) {
            collectHoverPath(it->layoutTree, point, newPath);
            break;
        }
    }

    if (newPath.empty()) {
        collectHoverPath(cachedLayoutTree_, point, newPath);
    }

    size_t commonLen = 0;
    size_t minLen = std::min(hoveredViews_.size(), newPath.size());
    while (commonLen < minLen &&
           hoveredViews_[commonLen].getTypeName() == newPath[commonLen].getTypeName()) {
        ++commonLen;
    }

    bool changed = (commonLen != hoveredViews_.size() || commonLen != newPath.size());

    for (size_t i = hoveredViews_.size(); i > commonLen; --i) {
        hoveredViews_[i - 1].handleMouseLeave();
    }

    for (size_t i = commonLen; i < newPath.size(); ++i) {
        newPath[i].handleMouseEnter();
    }

    hoveredViews_ = std::move(newPath);
    return changed;
}

void Renderer::renderOverlays(const Rect& viewport) {
    if (overlayManager_.empty()) return;

    suppressRedrawRequests();
    overlayManager_.layoutOverlays(*renderContext_, viewport);
    resumeRedrawRequests();

    for (auto& entry : overlayManager_.entries()) {
        // Draw backdrop if configured
        if (entry.config.backdrop.a > 0.0f) {
            renderContext_->save();
            renderContext_->setFillColor(entry.config.backdrop);
            renderContext_->drawRect(viewport);
            renderContext_->restore();
        }

        renderTree(entry.layoutTree, entry.element.get());
    }
}

bool Renderer::dispatchPointerEventToOverlays(PointerEvent& event) {
    auto& entries = overlayManager_.entries();
    for (auto it = entries.rbegin(); it != entries.rend(); ++it) {
        if (it->layoutTree.bounds.contains(event.windowPosition)) {
            return dispatchPointerEvent(it->layoutTree, event);
        }

        if (it->config.dismissOnClickOutside &&
            event.kind == PointerEvent::Kind::Down) {
            std::string id = it->id;
            auto onDismiss = it->config.onDismiss;
            bool modal = it->config.modal;
            overlayManager_.hide(id);
            if (onDismiss) onDismiss();
            return modal;
        }
    }
    return false;
}

} // namespace flux

