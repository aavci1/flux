#pragma once

#include <Flux/Core/KeyEvent.hpp>
#include <vector>

namespace flux {

// Forward declarations
struct LayoutNode;
class FocusState;

/**
 * @brief Handles keyboard input, modifier tracking, and event queuing
 * 
 * Responsible for:
 * - Tracking keyboard modifier state (Ctrl, Shift, Alt, Super)
 * - Queuing keyboard events for frame-synchronized dispatch
 * - Converting raw key codes to KeyEvent objects
 */
class KeyboardInputHandler {
public:
    KeyboardInputHandler();
    ~KeyboardInputHandler() = default;

    /**
     * @brief Handle a raw key down event (modifier state inferred from key presses; see overload).
     */
    void handleKeyDown(int key);

    /**
     * @brief Key down with OS-reported modifiers (required on macOS: Cmd/Opt use flagsChanged, not keyDown).
     */
    void handleKeyDown(int key, KeyModifier platformModifiers);

    /**
     * @brief Handle a raw key up event (modifier state inferred from key presses; see overload).
     */
    void handleKeyUp(int key);

    void handleKeyUp(int key, KeyModifier platformModifiers);
    
    /**
     * @brief Handle text input
     */
    void handleTextInput(const std::string& text);

    /**
     * @brief Queue a key down with explicit modifiers (e.g. synthetic test IPC).
     */
    void enqueueSyntheticKeyDown(KeyEvent event);
    
    /**
     * @brief Get current modifier state
     */
    KeyModifier getModifiers() const { return currentModifiers_; }
    
    /**
     * @brief Process all pending events and dispatch to focused view
     * @param layoutTree The current layout tree
     * @param focusState The focus state manager
     */
    void processPendingEvents(LayoutNode& layoutTree, FocusState& focusState);
    
    /**
     * @brief Get pending key down events (for external processing)
     */
    const std::vector<KeyEvent>& getPendingKeyDown() const { return pendingKeyDownEvents_; }
    
    /**
     * @brief Get pending key up events (for external processing)
     */
    const std::vector<KeyEvent>& getPendingKeyUp() const { return pendingKeyUpEvents_; }
    
    /**
     * @brief Clear all pending events
     */
    void clearPendingEvents();

    /**
     * @brief Clear only key down/up events (keeps text input, e.g. after Paste shortcut)
     */
    void clearPendingKeyEvents();

private:
    KeyModifier currentModifiers_;
    std::vector<KeyEvent> pendingKeyDownEvents_;
    std::vector<KeyEvent> pendingKeyUpEvents_;
    std::vector<TextInputEvent> pendingTextInputEvents_;
    
    void updateModifiers(int key, bool pressed);
};

} // namespace flux

