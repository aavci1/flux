#pragma once

#include <cstdint>
#include <string>

namespace flux {

/**
 * @brief Key codes based on Linux input event codes
 * 
 * These correspond to the key codes from <linux/input-event-codes.h>
 */
enum class Key : uint32_t {
    Unknown = 0,
    
    // Alphabet keys
    A = 30, B = 48, C = 46, D = 32, E = 18, F = 33, G = 34, H = 35,
    I = 23, J = 36, K = 37, L = 38, M = 50, N = 49, O = 24, P = 25,
    Q = 16, R = 19, S = 31, T = 20, U = 22, V = 47, W = 17, X = 45,
    Y = 21, Z = 44,
    
    // Number keys (top row)
    Num0 = 11, Num1 = 2, Num2 = 3, Num3 = 4, Num4 = 5,
    Num5 = 6, Num6 = 7, Num7 = 8, Num8 = 9, Num9 = 10,
    
    // Function keys
    F1 = 59, F2 = 60, F3 = 61, F4 = 62, F5 = 63, F6 = 64,
    F7 = 65, F8 = 66, F9 = 67, F10 = 68, F11 = 87, F12 = 88,
    
    // Navigation keys
    Escape = 1,
    Tab = 15,
    Backspace = 14,
    Enter = 28,
    Space = 57,
    
    Insert = 110,
    Delete = 111,
    Home = 102,
    End = 107,
    PageUp = 104,
    PageDown = 109,
    
    Left = 105,
    Right = 106,
    Up = 103,
    Down = 108,
    
    // Modifier keys
    LeftShift = 42,
    RightShift = 54,
    LeftCtrl = 29,
    RightCtrl = 97,
    LeftAlt = 56,
    RightAlt = 100,
    LeftSuper = 125,  // Windows/Super/Command key
    RightSuper = 126,
    
    // Special keys
    CapsLock = 58,
    NumLock = 69,
    ScrollLock = 70,
    
    // Symbol keys
    Minus = 12,
    Equal = 13,
    LeftBracket = 26,
    RightBracket = 27,
    Semicolon = 39,
    Apostrophe = 40,
    Grave = 41,        // `
    Backslash = 43,
    Comma = 51,
    Period = 52,
    Slash = 53,
};

/**
 * @brief Keyboard modifiers (bit flags)
 */
enum class KeyModifier : uint32_t {
    None = 0,
    Shift = 1 << 0,
    Ctrl = 1 << 1,
    Alt = 1 << 2,
    Super = 1 << 3,  // Windows/Command key
};

/**
 * @brief Bitwise OR for modifiers
 */
inline KeyModifier operator|(KeyModifier a, KeyModifier b) {
    return static_cast<KeyModifier>(
        static_cast<uint32_t>(a) | static_cast<uint32_t>(b)
    );
}

/**
 * @brief Bitwise AND for modifiers
 */
inline KeyModifier operator&(KeyModifier a, KeyModifier b) {
    return static_cast<KeyModifier>(
        static_cast<uint32_t>(a) & static_cast<uint32_t>(b)
    );
}

/**
 * @brief Check if modifier is set
 */
inline bool hasModifier(KeyModifier mods, KeyModifier check) {
    return (static_cast<uint32_t>(mods) & static_cast<uint32_t>(check)) != 0;
}

/**
 * @brief Keyboard event structure
 */
struct KeyEvent {
    Key key;
    KeyModifier modifiers;
    uint32_t rawKeyCode;  // Raw key code from platform
    bool isRepeat;        // True if this is a key repeat event
    
    KeyEvent()
        : key(Key::Unknown)
        , modifiers(KeyModifier::None)
        , rawKeyCode(0)
        , isRepeat(false) {}
    
    KeyEvent(Key k, KeyModifier mods = KeyModifier::None, uint32_t raw = 0, bool repeat = false)
        : key(k)
        , modifiers(mods)
        , rawKeyCode(raw)
        , isRepeat(repeat) {}
    
    // Helper methods
    bool hasShift() const { return hasModifier(modifiers, KeyModifier::Shift); }
    bool hasCtrl() const { return hasModifier(modifiers, KeyModifier::Ctrl); }
    bool hasAlt() const { return hasModifier(modifiers, KeyModifier::Alt); }
    bool hasSuper() const { return hasModifier(modifiers, KeyModifier::Super); }
};

/**
 * @brief Text input event (for composed characters, IME, etc.)
 */
struct TextInputEvent {
    std::string text;  // UTF-8 encoded text
    
    TextInputEvent() = default;
    explicit TextInputEvent(const std::string& t) : text(t) {}
};

/**
 * @brief Convert raw key code to Key enum
 */
inline Key keyFromRawCode(uint32_t rawCode) {
    // Direct mapping for now (assumes Linux input event codes)
    return static_cast<Key>(rawCode);
}

/**
 * @brief Get human-readable name for a key
 */
inline std::string keyName(Key key) {
    switch (key) {
        case Key::A: return "A";
        case Key::B: return "B";
        case Key::C: return "C";
        case Key::D: return "D";
        case Key::E: return "E";
        case Key::F: return "F";
        case Key::G: return "G";
        case Key::H: return "H";
        case Key::I: return "I";
        case Key::J: return "J";
        case Key::K: return "K";
        case Key::L: return "L";
        case Key::M: return "M";
        case Key::N: return "N";
        case Key::O: return "O";
        case Key::P: return "P";
        case Key::Q: return "Q";
        case Key::R: return "R";
        case Key::S: return "S";
        case Key::T: return "T";
        case Key::U: return "U";
        case Key::V: return "V";
        case Key::W: return "W";
        case Key::X: return "X";
        case Key::Y: return "Y";
        case Key::Z: return "Z";
        
        case Key::Num0: return "0";
        case Key::Num1: return "1";
        case Key::Num2: return "2";
        case Key::Num3: return "3";
        case Key::Num4: return "4";
        case Key::Num5: return "5";
        case Key::Num6: return "6";
        case Key::Num7: return "7";
        case Key::Num8: return "8";
        case Key::Num9: return "9";
        
        case Key::Escape: return "Escape";
        case Key::Tab: return "Tab";
        case Key::Backspace: return "Backspace";
        case Key::Enter: return "Enter";
        case Key::Space: return "Space";
        
        case Key::Left: return "Left";
        case Key::Right: return "Right";
        case Key::Up: return "Up";
        case Key::Down: return "Down";
        
        case Key::Home: return "Home";
        case Key::End: return "End";
        case Key::PageUp: return "PageUp";
        case Key::PageDown: return "PageDown";
        
        case Key::Delete: return "Delete";
        case Key::Insert: return "Insert";
        
        default: return "Unknown";
    }
}

} // namespace flux

