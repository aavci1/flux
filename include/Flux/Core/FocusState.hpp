#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Core/View.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <vector>
#include <string>

namespace flux {

// Forward declarations
struct LayoutNode;

/**
 * @brief Manages keyboard focus state for views
 * 
 * Responsible for:
 * - Tracking which view currently has focus
 * - Registering focusable views during layout
 * - Focus navigation (Tab/Shift+Tab)
 * - Dispatching keyboard events to focused view
 */
class FocusState {
public:
    FocusState();
    ~FocusState() = default;

    /**
     * @brief Register a view as focusable
     */
    void registerFocusableView(View* view, const Rect& bounds);
    
    /**
     * @brief Clear all registered focusable views
     */
    void clearFocusableViews();
    
    /**
     * @brief Move focus to next focusable view
     */
    void focusNext();
    
    /**
     * @brief Move focus to previous focusable view
     */
    void focusPrevious();
    
    /**
     * @brief Get currently focused view
     */
    View* getFocusedView() const;
    
    /**
     * @brief Clear focus (no view will have focus)
     */
    void clearFocus();
    
    /**
     * @brief Focus the view at given point
     */
    bool focusViewAtPoint(const Point& point);
    
    /**
     * @brief Get number of focusable views
     */
    size_t getFocusableViewCount() const { return focusableViews_.size(); }
    
    /**
     * @brief Get the key of currently focused view
     */
    std::string getFocusedKey() const { return focusedKey_; }
    
    /**
     * @brief Find a view in layout tree by focus key
     */
    static View* findViewByKey(LayoutNode& root, const std::string& key);
    
    /**
     * @brief Dispatch key down event to focused view
     */
    bool dispatchKeyDownToFocused(LayoutNode& root, const KeyEvent& event);
    
    /**
     * @brief Dispatch key up event to focused view
     */
    bool dispatchKeyUpToFocused(LayoutNode& root, const KeyEvent& event);
    
    /**
     * @brief Dispatch text input to focused view
     */
    bool dispatchTextInputToFocused(LayoutNode& root, const TextInputEvent& event);

private:
    struct FocusableViewInfo {
        View* view;
        Rect bounds;
        std::string key;
    };
    
    std::string focusedKey_;
    std::vector<FocusableViewInfo> focusableViews_;
    
    int findViewIndexByKey(const std::string& key) const;
    std::string generateAutoKey(const View* view, int registrationIndex) const;
};

} // namespace flux

