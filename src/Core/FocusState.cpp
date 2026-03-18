#include <Flux/Core/FocusState.hpp>
#include <Flux/Core/LayoutTree.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/Log.hpp>
#include <sstream>
#include <algorithm>

namespace flux {

FocusState::FocusState()
    : focusedKey_("") {
    FLUX_LOG_DEBUG("[FOCUS] Focus management initialized (key-based tracking)");
}

void FocusState::registerFocusableView(View* view, const Rect& bounds) {
    if (!view) {
        FLUX_LOG_WARN("[FOCUS] Attempted to register null view");
        return;
    }

    if (bounds.width <= 0 || bounds.height <= 0) {
        FLUX_LOG_WARN("[FOCUS] Skipping view with invalid bounds");
        return;
    }

    std::string key = view->getFocusKey();
    if (key.empty()) {
        key = generateAutoKey(view, static_cast<int>(focusableViews_.size()));
    }
    
    FLUX_LOG_DEBUG("[FOCUS] Registered focusable view #%zu (%s) with key '%s' at (%.0f, %.0f, %.0fx%.0f)",
                   focusableViews_.size(), view->getTypeName().c_str(), key.c_str(),
                   bounds.x, bounds.y, bounds.width, bounds.height);

    focusableViews_.push_back({view, bounds, key});
}

void FocusState::clearFocusableViews() {
    focusableViews_.clear();
}

void FocusState::focusNext() {
    if (focusableViews_.empty()) {
        FLUX_LOG_DEBUG("[FOCUS] No focusable views available");
        return;
    }

    int currentIndex = findViewIndexByKey(focusedKey_);
    int nextIndex;
    
    if (currentIndex == -1) {
        nextIndex = 0;
    } else {
        nextIndex = (currentIndex + 1) % static_cast<int>(focusableViews_.size());
    }
    
    std::string newKey = focusableViews_[nextIndex].key;
    FLUX_LOG_DEBUG("[FOCUS] Moving focus: index %d -> %d (key: '%s', total: %zu views)",
                   currentIndex, nextIndex, newKey.c_str(), focusableViews_.size());

    notifyFocusChange(newKey);
    requestApplicationRedraw();
}

void FocusState::focusPrevious() {
    if (focusableViews_.empty()) {
        FLUX_LOG_DEBUG("[FOCUS] No focusable views available");
        return;
    }

    int currentIndex = findViewIndexByKey(focusedKey_);
    int prevIndex;
    
    if (currentIndex == -1) {
        prevIndex = static_cast<int>(focusableViews_.size()) - 1;
    } else {
        prevIndex = currentIndex - 1;
        if (prevIndex < 0) {
            prevIndex = static_cast<int>(focusableViews_.size()) - 1;
        }
    }
    
    std::string newKey = focusableViews_[prevIndex].key;
    FLUX_LOG_DEBUG("[FOCUS] Moving focus: index %d -> %d (key: '%s', total: %zu views)",
                   currentIndex, prevIndex, newKey.c_str(), focusableViews_.size());

    notifyFocusChange(newKey);
    requestApplicationRedraw();
}

View* FocusState::getFocusedView() const {
    int focusedIndex = findViewIndexByKey(focusedKey_);
    if (focusedIndex >= 0 && focusedIndex < static_cast<int>(focusableViews_.size())) {
        return focusableViews_[focusedIndex].view;
    }
    return nullptr;
}

void FocusState::clearFocus() {
    FLUX_LOG_DEBUG("[FOCUS] Clearing focus (was on key '%s')", focusedKey_.c_str());
    focusedKey_.clear();
}

bool FocusState::focusViewAtPoint(const Point& point) {
    for (int i = static_cast<int>(focusableViews_.size()) - 1; i >= 0; --i) {
        if (focusableViews_[i].bounds.contains(point)) {
            std::string newKey = focusableViews_[i].key;
            if (newKey == focusedKey_) return true;

            FLUX_LOG_DEBUG("[FOCUS] Found focusable view at click point (%.0f, %.0f) - key '%s'",
                           point.x, point.y, newKey.c_str());

            notifyFocusChange(newKey);
            return true;
        }
    }
    
    if (!focusedKey_.empty()) {
        FLUX_LOG_DEBUG("[FOCUS] Clearing focus (clicked non-focusable area)");
        notifyFocusChange("");
    }
    return false;
}

View* FocusState::findViewByKey(LayoutNode& root, const std::string& key) {
    if (key.empty()) {
        return nullptr;
    }
    
    if (root.view.getFocusKey() == key) {
        return &root.view;
    }
    
    for (auto& child : root.children) {
        if (View* found = findViewByKey(child, key)) {
            return found;
        }
    }
    
    return nullptr;
}

bool FocusState::dispatchKeyDownToFocused(LayoutNode& root, const KeyEvent& event) {
    (void)root;
    
    if (focusedKey_.empty()) {
        return false;
    }
    
    int focusedIndex = findViewIndexByKey(focusedKey_);
    if (focusedIndex < 0 || focusedIndex >= static_cast<int>(focusableViews_.size())) {
        FLUX_LOG_WARN("[FOCUS] Focused view '%s' not found in current frame", focusedKey_.c_str());
        return false;
    }

    View* focusedView = focusableViews_[focusedIndex].view;
    if (!focusedView) {
        FLUX_LOG_WARN("[FOCUS] Focused view pointer is null");
        return false;
    }
    
    bool handled = focusedView->handleKeyDown(event);
    if (handled) {
        FLUX_LOG_DEBUG("[FOCUS] Key down handled by focused view '%s'", focusedKey_.c_str());
    } else {
        FLUX_LOG_DEBUG("[FOCUS] Key down not handled by focused view '%s'", focusedKey_.c_str());
    }
    return handled;
}

bool FocusState::dispatchKeyUpToFocused(LayoutNode& root, const KeyEvent& event) {
    (void)root;
    
    if (focusedKey_.empty()) {
        return false;
    }
    
    int focusedIndex = findViewIndexByKey(focusedKey_);
    if (focusedIndex < 0 || focusedIndex >= static_cast<int>(focusableViews_.size())) {
        return false;
    }
    
    View* focusedView = focusableViews_[focusedIndex].view;
    if (!focusedView) {
        return false;
    }
    
    bool handled = focusedView->handleKeyUp(event);
    if (handled) {
        FLUX_LOG_DEBUG("[FOCUS] Key up handled by focused view '%s'", focusedKey_.c_str());
    }
    return handled;
}

bool FocusState::dispatchTextInputToFocused(LayoutNode& root, const TextInputEvent& event) {
    (void)root;
    
    if (focusedKey_.empty()) {
        return false;
    }
    
    int focusedIndex = findViewIndexByKey(focusedKey_);
    if (focusedIndex < 0 || focusedIndex >= static_cast<int>(focusableViews_.size())) {
        return false;
    }
    
    View* focusedView = focusableViews_[focusedIndex].view;
    if (!focusedView) {
        return false;
    }
    
    bool handled = focusedView->handleTextInput(event);
    if (handled) {
        FLUX_LOG_DEBUG("[FOCUS] Text input '%s' handled by focused view '%s'",
                       event.text.c_str(), focusedKey_.c_str());
    }
    return handled;
}

int FocusState::findViewIndexByKey(const std::string& key) const {
    if (key.empty()) {
        return -1;
    }
    
    for (size_t i = 0; i < focusableViews_.size(); ++i) {
        if (focusableViews_[i].key == key) {
            return static_cast<int>(i);
        }
    }
    
    return -1;
}

std::string FocusState::generateAutoKey(const View* view, int registrationIndex) const {
    std::ostringstream oss;
    oss << view->getTypeName() << "_" << registrationIndex;
    return oss.str();
}

void FocusState::notifyFocusChange(const std::string& newKey) {
    if (newKey == focusedKey_) return;
    focusedKey_ = newKey;

    // Focus notifications are deferred to render time via isCurrentViewFocused()
    // to avoid dangling pointer issues with View* during event dispatch.
}

} // namespace flux

