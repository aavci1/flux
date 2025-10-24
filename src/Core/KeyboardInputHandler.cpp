#include <Flux/Core/KeyboardInputHandler.hpp>
#include <Flux/Core/FocusState.hpp>
#include <Flux/Core/LayoutTree.hpp>
#include <iostream>

namespace flux {

KeyboardInputHandler::KeyboardInputHandler()
    : currentModifiers_(KeyModifier::None) {
}

void KeyboardInputHandler::handleKeyDown(int key) {
    // Update modifier state
    updateModifiers(key, true);
    
    // Create KeyEvent
    KeyEvent event;
    event.key = keyFromRawCode(key);
    event.modifiers = currentModifiers_;
    event.rawKeyCode = key;
    event.isRepeat = false;
    
    std::cout << "[INPUT] Key down: " << keyName(event.key)
              << " (raw: " << key << ", mods: "
              << (event.hasCtrl() ? "Ctrl " : "")
              << (event.hasShift() ? "Shift " : "")
              << (event.hasAlt() ? "Alt " : "")
              << ")\n";
    
    // Queue the event to be processed during the next render frame
    pendingKeyDownEvents_.push_back(event);
}

void KeyboardInputHandler::handleKeyUp(int key) {
    // Update modifier state
    updateModifiers(key, false);
    
    // Create KeyEvent
    KeyEvent event;
    event.key = keyFromRawCode(key);
    event.modifiers = currentModifiers_;
    event.rawKeyCode = key;
    event.isRepeat = false;
    
    std::cout << "[INPUT] Key up: " << keyName(event.key) << " (raw: " << key << ")\n";
    
    // Queue the event
    pendingKeyUpEvents_.push_back(event);
}

void KeyboardInputHandler::handleTextInput(const std::string& text) {
    std::cout << "[INPUT] Text input: \"" << text << "\"\n";
    
    TextInputEvent event(text);
    pendingTextInputEvents_.push_back(event);
}

void KeyboardInputHandler::processPendingEvents(LayoutNode& layoutTree, FocusState& focusState) {
    // Process pending key down events
    for (const auto& event : pendingKeyDownEvents_) {
        focusState.dispatchKeyDownToFocused(layoutTree, event);
    }
    pendingKeyDownEvents_.clear();
    
    // Process pending key up events
    for (const auto& event : pendingKeyUpEvents_) {
        focusState.dispatchKeyUpToFocused(layoutTree, event);
    }
    pendingKeyUpEvents_.clear();
    
    // Process pending text input events
    for (const auto& event : pendingTextInputEvents_) {
        focusState.dispatchTextInputToFocused(layoutTree, event);
    }
    pendingTextInputEvents_.clear();
}

void KeyboardInputHandler::clearPendingEvents() {
    pendingKeyDownEvents_.clear();
    pendingKeyUpEvents_.clear();
    pendingTextInputEvents_.clear();
}

void KeyboardInputHandler::updateModifiers(int key, bool pressed) {
    uint32_t mod = static_cast<uint32_t>(currentModifiers_);
    
    switch (key) {
        case static_cast<int>(Key::LeftShift):
        case static_cast<int>(Key::RightShift):
            if (pressed) {
                mod |= static_cast<uint32_t>(KeyModifier::Shift);
            } else {
                mod &= ~static_cast<uint32_t>(KeyModifier::Shift);
            }
            break;
            
        case static_cast<int>(Key::LeftCtrl):
        case static_cast<int>(Key::RightCtrl):
            if (pressed) {
                mod |= static_cast<uint32_t>(KeyModifier::Ctrl);
            } else {
                mod &= ~static_cast<uint32_t>(KeyModifier::Ctrl);
            }
            break;
            
        case static_cast<int>(Key::LeftAlt):
        case static_cast<int>(Key::RightAlt):
            if (pressed) {
                mod |= static_cast<uint32_t>(KeyModifier::Alt);
            } else {
                mod &= ~static_cast<uint32_t>(KeyModifier::Alt);
            }
            break;
            
        case static_cast<int>(Key::LeftSuper):
        case static_cast<int>(Key::RightSuper):
            if (pressed) {
                mod |= static_cast<uint32_t>(KeyModifier::Super);
            } else {
                mod &= ~static_cast<uint32_t>(KeyModifier::Super);
            }
            break;
    }
    
    currentModifiers_ = static_cast<KeyModifier>(mod);
}

} // namespace flux

