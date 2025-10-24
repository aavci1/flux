#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <vector>
#include <memory>
#include <functional>
#include <string>

namespace flux {

// Forward declarations
class Window;
struct LayoutNode;

/**
 * @brief Manages keyboard focus for views within a window
 * 
 * The FocusManager tracks which view currently has keyboard focus and provides
 * methods for navigating focus between views using Tab/Shift+Tab.
 * 
 * In immediate mode, views are recreated each frame. Focus is tracked using:
 * 1. Explicit focus keys (if provided via the focusKey property)
 * 2. Auto-generated keys based on registration order (for views without explicit keys)
 * 
 * This approach allows focus to persist across frames even when views are
 * conditionally rendered or reordered.
 */
class FocusManager {
public:
    explicit FocusManager(Window* window);
    ~FocusManager() = default;

    /**
     * @brief Register a view as focusable
     * @param view The view to register
     * @param bounds The bounds of the view (for hit testing)
     */
    void registerFocusableView(View* view, const Rect& bounds);

    /**
     * @brief Clear all registered focusable views
     * 
     * This should be called at the start of each frame before rebuilding
     * the view tree, since views are recreated each frame in immediate mode.
     */
    void clearFocusableViews();

    /**
     * @brief Move focus to the next focusable view (Tab key)
     */
    void focusNext();

    /**
     * @brief Move focus to the previous focusable view (Shift+Tab key)
     */
    void focusPrevious();

    /**
     * @brief Get the currently focused view
     * @return Pointer to focused view, or nullptr if no view has focus
     */
    View* getFocusedView() const;

    /**
     * @brief Clear keyboard focus (no view will have focus)
     */
    void clearFocus();

    /**
     * @brief Find and focus the view at the given point
     * @param point The point to test (in window coordinates)
     * @return true if a focusable view was found and focused
     */
    bool focusViewAtPoint(const Point& point);

    /**
     * @brief Get the number of registered focusable views
     */
    size_t getFocusableViewCount() const { return focusableViews_.size(); }

    /**
     * @brief Get the key of the currently focused view
     * @return Focus key, or empty string if no view has focus
     */
    std::string getFocusedKey() const { return focusedKey_; }
    
    /**
     * @brief Find a view in a layout tree by its focus key
     * @param root The root layout node to search
     * @param key The focus key to search for
     * @return Pointer to the view, or nullptr if not found
     */
    static View* findViewByKey(LayoutNode& root, const std::string& key);
    
    /**
     * @brief Dispatch a key down event to the focused view in the layout tree
     * @param root The root layout node
     * @param event The key event
     * @return true if the event was handled
     */
    bool dispatchKeyDownToFocused(LayoutNode& root, const KeyEvent& event);
    
    /**
     * @brief Dispatch a key up event to the focused view in the layout tree
     * @param root The root layout node
     * @param event The key event
     * @return true if the event was handled
     */
    bool dispatchKeyUpToFocused(LayoutNode& root, const KeyEvent& event);
    
    /**
     * @brief Dispatch a text input event to the focused view in the layout tree
     * @param root The root layout node
     * @param event The text input event
     * @return true if the event was handled
     */
    bool dispatchTextInputToFocused(LayoutNode& root, const TextInputEvent& event);

private:
    struct FocusableViewInfo {
        View* view;
        Rect bounds;
        std::string key;  // Unique key for this view (explicit or auto-generated)
    };

    Window* window_;
    std::string focusedKey_;  // Track focus by key for stability across frames
    std::vector<FocusableViewInfo> focusableViews_;
    
    // Helper to find the index of a view by its key
    int findViewIndexByKey(const std::string& key) const;
    
    // Helper to generate an auto key for a view without an explicit focusKey
    std::string generateAutoKey(const View* view, int registrationIndex) const;
};

} // namespace flux

