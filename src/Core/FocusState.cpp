#include <Flux/Core/FocusState.hpp>
#include <Flux/Core/Element.hpp>
#include <Flux/Core/LayoutTree.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/Log.hpp>
#include <Flux/Core/View.hpp>
#include <sstream>
#include <algorithm>
#include <vector>

namespace flux {

FocusState::FocusState()
    : focusedKey_("") {
    FLUX_LOG_DEBUG("[FOCUS] Focus management initialized (key-based tracking)");
}

std::string FocusState::registerFocusableElement(Element* element, const Rect& bounds) {
    if (!element || !element->description) {
        FLUX_LOG_WARN("[FOCUS] Attempted to register null element");
        return "";
    }

    if (bounds.width <= 0 || bounds.height <= 0) {
        FLUX_LOG_WARN("[FOCUS] Skipping element with invalid bounds");
        return "";
    }

    View* view = element->description.get();
    std::string key = view->getFocusKey();
    if (key.empty()) {
        key = generateAutoKey(element, static_cast<int>(focusableViews_.size()));
    }
    
    FLUX_LOG_DEBUG("[FOCUS] Registered focusable view #%zu (%s) with key '%s' at (%.0f, %.0f, %.0fx%.0f)",
                   focusableViews_.size(), view->getTypeName().c_str(), key.c_str(),
                   bounds.x, bounds.y, bounds.width, bounds.height);

    keyIndex_[key] = focusableViews_.size();
    focusableViews_.push_back({element, bounds, key});
    return key;
}

void FocusState::clearFocusableViews() {
    focusableViews_.clear();
    keyIndex_.clear();
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
    Element* elem = getFocusedElement();
    if (elem && elem->description) {
        return elem->description.get();
    }
    return nullptr;
}

Element* FocusState::getFocusedElement() const {
    int focusedIndex = findViewIndexByKey(focusedKey_);
    if (focusedIndex >= 0 && focusedIndex < static_cast<int>(focusableViews_.size())) {
        return focusableViews_[focusedIndex].element;
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

static std::vector<Element*> buildAncestorPath(Element* target) {
    std::vector<Element*> path;
    for (Element* e = target; e != nullptr; e = e->parent) {
        path.push_back(e);
    }
    std::reverse(path.begin(), path.end());
    return path;
}

bool FocusState::dispatchKeyDownToFocused(LayoutNode& root, const KeyEvent& event) {
    (void)root;

    if (focusedKey_.empty()) return false;

    Element* focusedElem = getFocusedElement();
    if (!focusedElem || !focusedElem->description) {
        FLUX_LOG_WARN("[FOCUS] Focused element '%s' not found in current frame", focusedKey_.c_str());
        return false;
    }

    auto path = buildAncestorPath(focusedElem);

    // Capture: root → target (exclusive)
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        if (path[i]->description && path[i]->description->handleKeyDown(event)) {
            FLUX_LOG_DEBUG("[FOCUS] Key down captured by ancestor '%s'", path[i]->typeName.c_str());
            return true;
        }
    }

    // Target
    bool handled = focusedElem->description->handleKeyDown(event);
    if (handled) {
        FLUX_LOG_DEBUG("[FOCUS] Key down handled by focused view '%s'", focusedKey_.c_str());
        return true;
    }

    // Bubble: target → root (skip target)
    for (int i = static_cast<int>(path.size()) - 2; i >= 0; --i) {
        if (path[i]->description && path[i]->description->handleKeyDown(event)) {
            FLUX_LOG_DEBUG("[FOCUS] Key down bubbled to '%s'", path[i]->typeName.c_str());
            return true;
        }
    }

    FLUX_LOG_DEBUG("[FOCUS] Key down not handled by focused view '%s'", focusedKey_.c_str());
    return false;
}

bool FocusState::dispatchKeyUpToFocused(LayoutNode& root, const KeyEvent& event) {
    (void)root;

    if (focusedKey_.empty()) return false;

    Element* focusedElem = getFocusedElement();
    if (!focusedElem || !focusedElem->description) return false;

    auto path = buildAncestorPath(focusedElem);

    for (size_t i = 0; i + 1 < path.size(); ++i) {
        if (path[i]->description && path[i]->description->handleKeyUp(event)) {
            return true;
        }
    }

    if (focusedElem->description->handleKeyUp(event)) {
        FLUX_LOG_DEBUG("[FOCUS] Key up handled by focused view '%s'", focusedKey_.c_str());
        return true;
    }

    for (int i = static_cast<int>(path.size()) - 2; i >= 0; --i) {
        if (path[i]->description && path[i]->description->handleKeyUp(event)) {
            return true;
        }
    }
    return false;
}

bool FocusState::dispatchTextInputToFocused(LayoutNode& root, const TextInputEvent& event) {
    (void)root;

    if (focusedKey_.empty()) return false;

    Element* focusedElem = getFocusedElement();
    if (!focusedElem || !focusedElem->description) return false;

    auto path = buildAncestorPath(focusedElem);

    for (size_t i = 0; i + 1 < path.size(); ++i) {
        if (path[i]->description && path[i]->description->handleTextInput(event)) {
            return true;
        }
    }

    if (focusedElem->description->handleTextInput(event)) {
        FLUX_LOG_DEBUG("[FOCUS] Text input '%s' handled by focused view '%s'",
                       event.text.c_str(), focusedKey_.c_str());
        return true;
    }

    for (int i = static_cast<int>(path.size()) - 2; i >= 0; --i) {
        if (path[i]->description && path[i]->description->handleTextInput(event)) {
            return true;
        }
    }
    return false;
}

int FocusState::findViewIndexByKey(const std::string& key) const {
    if (key.empty()) return -1;
    auto it = keyIndex_.find(key);
    if (it != keyIndex_.end()) return static_cast<int>(it->second);
    return -1;
}

std::string FocusState::generateAutoKey(const Element* element, int registrationIndex) const {
    std::ostringstream oss;
    if (element && element->description) {
        oss << element->description->getTypeName();
    } else {
        oss << "unknown";
    }
    oss << "_" << registrationIndex;
    return oss.str();
}

void FocusState::notifyFocusChange(const std::string& newKey) {
    if (newKey == focusedKey_) return;
    std::string oldKey = focusedKey_;
    focusedKey_ = newKey;
    pendingBlurKey_ = oldKey;
    pendingFocusKey_ = newKey;
}

void FocusState::dispatchPendingFocusNotifications() {
    if (!pendingBlurKey_.empty()) {
        int idx = findViewIndexByKey(pendingBlurKey_);
        if (idx >= 0 && idx < static_cast<int>(focusableViews_.size())) {
            Element* elem = focusableViews_[idx].element;
            if (elem && elem->description) {
                elem->description->notifyFocusLost();
            }
        }
        pendingBlurKey_.clear();
    }
    if (!pendingFocusKey_.empty()) {
        int idx = findViewIndexByKey(pendingFocusKey_);
        if (idx >= 0 && idx < static_cast<int>(focusableViews_.size())) {
            Element* elem = focusableViews_[idx].element;
            if (elem && elem->description) {
                elem->description->notifyFocusGained();
            }
        }
        pendingFocusKey_.clear();
    }
}

} // namespace flux

