#pragma once

#include <Flux/Core/KeyEvent.hpp>

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>

namespace flux::term {

/**
 * View-level actions triggered by key bindings (zoom, scroll).
 * The terminal view performs these; the session only reports them.
 */
enum class TerminalViewAction {
    ZoomIn,
    ZoomOut,
    ZoomReset,
    ScrollPageUp,
    ScrollPageDown,
    ScrollHome,
    ScrollEnd,
};

/** Binding value: either bytes to send to the PTY or a view action. */
using TerminalBindingValue = std::variant<std::string, TerminalViewAction>;

/** (Key + modifiers) used as binding key. Modifiers must match exactly. */
struct TerminalBindingKey {
    Key key{Key::Unknown};
    KeyModifier modifiers{KeyModifier::None};

    bool operator==(const TerminalBindingKey& o) const {
        return key == o.key && modifiers == o.modifiers;
    }
};

struct TerminalBindingKeyHash {
    std::size_t operator()(const TerminalBindingKey& k) const {
        return std::hash<std::uint32_t>()(static_cast<std::uint32_t>(k.key)) ^
               (std::hash<std::uint32_t>()(static_cast<std::uint32_t>(k.modifiers)) << 1);
    }
};

/**
 * Extensible key binding table for the terminal.
 * Maps (Key, KeyModifier) to either bytes to send to the PTY or a view action.
 * Use defaultBindings() for standard xterm-style behavior; then add/remove as needed.
 */
class TerminalKeyBindings {
public:
    TerminalKeyBindings() = default;

    /// Send the given bytes to the PTY when this key+modifiers are pressed.
    void bindSendBytes(Key key, KeyModifier modifiers, std::string bytes);

    /// Trigger a view action (zoom, scroll) when this key+modifiers are pressed.
    void bindViewAction(Key key, KeyModifier modifiers, TerminalViewAction action);

    /// Remove the binding for this key+modifiers, if any.
    void unbind(Key key, KeyModifier modifiers);

    /// Remove all bindings.
    void clear();

    /// Look up binding for this event. Returns the value if a binding exists.
    [[nodiscard]] std::optional<TerminalBindingValue> lookup(const KeyEvent& event) const;

    /// Build default xterm-style bindings (arrows, Enter, Tab, Ctrl+letter, zoom, scroll).
    static TerminalKeyBindings defaultBindings();

private:
    std::unordered_map<TerminalBindingKey, TerminalBindingValue, TerminalBindingKeyHash> bindings_;
};

} // namespace flux::term
