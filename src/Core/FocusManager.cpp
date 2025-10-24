#include <Flux/Core/FocusManager.hpp>
#include <Flux/Core/Window.hpp>
#include <Flux/Core/Application.hpp>
#include <Flux/Core/LayoutTree.hpp>
#include <algorithm>
#include <iostream>
#include <sstream>

namespace flux {

FocusManager::FocusManager(Window* window)
    : window_(window), focusedKey_("") {
    std::cout << "[FOCUS] FocusManager initialized (key-based tracking)\n";
}

void FocusManager::registerFocusableView(View* view, const Rect& bounds) {
    if (!view) {
        std::cout << "[FOCUS] Attempted to register null view\n";
        return;
    }

    // Additional safety checks
    if (bounds.width <= 0 || bounds.height <= 0) {
        std::cout << "[FOCUS] Skipping view with invalid bounds\n";
        return;
    }

    // Get explicit focus key or generate an auto key
    std::string key = view->getFocusKey();
    if (key.empty()) {
        // No explicit key, generate one based on type and registration order
        key = generateAutoKey(view, static_cast<int>(focusableViews_.size()));
    }
    
    std::cout << "[FOCUS] Registered focusable view #" << focusableViews_.size() 
              << " (" << view->getTypeName() << ") with key '" << key << "' at ("
              << bounds.x << ", " << bounds.y << ", "
              << bounds.width << "x" << bounds.height << ")\n";
    
    focusableViews_.push_back({view, bounds, key});
}

void FocusManager::clearFocusableViews() {
    // Keep the focused key - it will be matched against newly registered views
    // This allows focus to persist across frame rebuilds
    focusableViews_.clear();
}

void FocusManager::focusNext() {
    if (focusableViews_.empty()) {
        std::cout << "[FOCUS] No focusable views available\n";
        return;
    }

    int currentIndex = findViewIndexByKey(focusedKey_);
    int nextIndex;
    
    if (currentIndex == -1) {
        // No view currently focused, focus the first one
        nextIndex = 0;
    } else {
        // Move to next view, wrapping around
        nextIndex = (currentIndex + 1) % static_cast<int>(focusableViews_.size());
    }
    
    focusedKey_ = focusableViews_[nextIndex].key;
    
    std::cout << "[FOCUS] Moving focus: index " << currentIndex << " -> " << nextIndex 
              << " (key: '" << focusedKey_ << "', total: " << focusableViews_.size() << " views)\n";
    
    // Request redraw to show focus change
    if (window_) {
        Application::instance().requestRedraw();
    }
}

void FocusManager::focusPrevious() {
    if (focusableViews_.empty()) {
        std::cout << "[FOCUS] No focusable views available\n";
        return;
    }

    int currentIndex = findViewIndexByKey(focusedKey_);
    int prevIndex;
    
    if (currentIndex == -1) {
        // No view currently focused, focus the last one
        prevIndex = static_cast<int>(focusableViews_.size()) - 1;
    } else {
        // Move to previous view, wrapping around
        prevIndex = currentIndex - 1;
        if (prevIndex < 0) {
            prevIndex = static_cast<int>(focusableViews_.size()) - 1;
        }
    }
    
    focusedKey_ = focusableViews_[prevIndex].key;
    
    std::cout << "[FOCUS] Moving focus: index " << currentIndex << " -> " << prevIndex 
              << " (key: '" << focusedKey_ << "', total: " << focusableViews_.size() << " views)\n";
    
    // Request redraw to show focus change
    if (window_) {
        Application::instance().requestRedraw();
    }
}

View* FocusManager::getFocusedView() const {
    int focusedIndex = findViewIndexByKey(focusedKey_);
    if (focusedIndex >= 0 && focusedIndex < static_cast<int>(focusableViews_.size())) {
        return focusableViews_[focusedIndex].view;
    }
    return nullptr;
}

void FocusManager::clearFocus() {
    std::cout << "[FOCUS] Clearing focus (was on key '" << focusedKey_ << "')\n";
    focusedKey_.clear();
}

bool FocusManager::focusViewAtPoint(const Point& point) {
    // Search in reverse order (top-most views first)
    for (int i = static_cast<int>(focusableViews_.size()) - 1; i >= 0; --i) {
        if (focusableViews_[i].bounds.contains(point)) {
            std::cout << "[FOCUS] Found focusable view at click point ("
                      << point.x << ", " << point.y << ") - key '" 
                      << focusableViews_[i].key << "'\n";
            focusedKey_ = focusableViews_[i].key;
            return true;
        }
    }
    
    std::cout << "[FOCUS] No focusable view at point (" << point.x << ", " << point.y << ")\n";
    return false;
}

int FocusManager::findViewIndexByKey(const std::string& key) const {
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

std::string FocusManager::generateAutoKey(const View* view, int registrationIndex) const {
    // Generate a key based on view type and registration order
    // This provides stability as long as the UI structure doesn't change dramatically
    std::ostringstream oss;
    oss << view->getTypeName() << "_" << registrationIndex;
    return oss.str();
}

View* FocusManager::findViewByKey(LayoutNode& root, const std::string& key) {
    if (key.empty()) {
        return nullptr;
    }
    
    // Check if the root view has the key we're looking for
    if (root.view.getFocusKey() == key) {
        return &root.view;
    }
    
    // Recursively search children
    for (auto& child : root.children) {
        if (View* found = findViewByKey(child, key)) {
            return found;
        }
    }
    
    return nullptr;
}

bool FocusManager::dispatchKeyDownToFocused(LayoutNode& root, const KeyEvent& event) {
    (void)root; // Not used - we look up from focusableViews_ instead
    
    if (focusedKey_.empty()) {
        return false;
    }
    
    // Find the focused view by looking it up in the registration list
    // (not by searching the layout tree, since auto-generated keys aren't on View objects)
    int focusedIndex = findViewIndexByKey(focusedKey_);
    if (focusedIndex < 0 || focusedIndex >= static_cast<int>(focusableViews_.size())) {
        std::cout << "[FOCUS] Focused view '" << focusedKey_ << "' not found in current frame\n";
        return false;
    }
    
    View* focusedView = focusableViews_[focusedIndex].view;
    if (!focusedView) {
        std::cout << "[FOCUS] Focused view pointer is null\n";
        return false;
    }
    
    bool handled = focusedView->handleKeyDown(event);
    if (handled) {
        std::cout << "[FOCUS] Key down handled by focused view '" << focusedKey_ << "'\n";
    } else {
        std::cout << "[FOCUS] Key down not handled by focused view '" << focusedKey_ << "'\n";
    }
    return handled;
}

bool FocusManager::dispatchKeyUpToFocused(LayoutNode& root, const KeyEvent& event) {
    (void)root; // Not used
    
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
        std::cout << "[FOCUS] Key up handled by focused view '" << focusedKey_ << "'\n";
    }
    return handled;
}

bool FocusManager::dispatchTextInputToFocused(LayoutNode& root, const TextInputEvent& event) {
    (void)root; // Not used
    
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
        std::cout << "[FOCUS] Text input '" << event.text << "' handled by focused view '" << focusedKey_ << "'\n";
    }
    return handled;
}

} // namespace flux
