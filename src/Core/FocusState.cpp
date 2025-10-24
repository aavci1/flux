#include <Flux/Core/FocusState.hpp>
#include <Flux/Core/LayoutTree.hpp>
#include <Flux/Core/Application.hpp>
#include <iostream>
#include <sstream>
#include <algorithm>

namespace flux {

FocusState::FocusState()
    : focusedKey_("") {
    std::cout << "[FOCUS] Focus management initialized (key-based tracking)\n";
}

void FocusState::registerFocusableView(View* view, const Rect& bounds) {
    if (!view) {
        std::cout << "[FOCUS] Attempted to register null view\n";
        return;
    }

    if (bounds.width <= 0 || bounds.height <= 0) {
        std::cout << "[FOCUS] Skipping view with invalid bounds\n";
        return;
    }

    std::string key = view->getFocusKey();
    if (key.empty()) {
        key = generateAutoKey(view, static_cast<int>(focusableViews_.size()));
    }
    
    std::cout << "[FOCUS] Registered focusable view #" << focusableViews_.size() 
              << " (" << view->getTypeName() << ") with key '" << key << "' at ("
              << bounds.x << ", " << bounds.y << ", "
              << bounds.width << "x" << bounds.height << ")\n";
    
    focusableViews_.push_back({view, bounds, key});
}

void FocusState::clearFocusableViews() {
    focusableViews_.clear();
}

void FocusState::focusNext() {
    if (focusableViews_.empty()) {
        std::cout << "[FOCUS] No focusable views available\n";
        return;
    }

    int currentIndex = findViewIndexByKey(focusedKey_);
    int nextIndex;
    
    if (currentIndex == -1) {
        nextIndex = 0;
    } else {
        nextIndex = (currentIndex + 1) % static_cast<int>(focusableViews_.size());
    }
    
    focusedKey_ = focusableViews_[nextIndex].key;
    
    std::cout << "[FOCUS] Moving focus: index " << currentIndex << " -> " << nextIndex 
              << " (key: '" << focusedKey_ << "', total: " << focusableViews_.size() << " views)\n";
    
    Application::instance().requestRedraw();
}

void FocusState::focusPrevious() {
    if (focusableViews_.empty()) {
        std::cout << "[FOCUS] No focusable views available\n";
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
    
    focusedKey_ = focusableViews_[prevIndex].key;
    
    std::cout << "[FOCUS] Moving focus: index " << currentIndex << " -> " << prevIndex 
              << " (key: '" << focusedKey_ << "', total: " << focusableViews_.size() << " views)\n";
    
    Application::instance().requestRedraw();
}

View* FocusState::getFocusedView() const {
    int focusedIndex = findViewIndexByKey(focusedKey_);
    if (focusedIndex >= 0 && focusedIndex < static_cast<int>(focusableViews_.size())) {
        return focusableViews_[focusedIndex].view;
    }
    return nullptr;
}

void FocusState::clearFocus() {
    std::cout << "[FOCUS] Clearing focus (was on key '" << focusedKey_ << "')\n";
    focusedKey_.clear();
}

bool FocusState::focusViewAtPoint(const Point& point) {
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
        std::cout << "[FOCUS] Key up handled by focused view '" << focusedKey_ << "'\n";
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
        std::cout << "[FOCUS] Text input '" << event.text << "' handled by focused view '" << focusedKey_ << "'\n";
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

} // namespace flux

