#include "TerminalKeyBindings.hpp"

namespace flux::term {

namespace {

using enum Key;
using enum KeyModifier;

std::string ctrlLetter(char c) {
    if (c >= 'a' && c <= 'z') {
        return std::string(1, static_cast<char>(c - 'a' + 1));
    }
    if (c >= 'A' && c <= 'Z') {
        return std::string(1, static_cast<char>(c - 'A' + 1));
    }
    return "";
}

std::optional<char> keyToLetter(Key k) {
    switch (k) {
    case A: return 'a'; case B: return 'b'; case C: return 'c'; case D: return 'd';
    case E: return 'e'; case F: return 'f'; case G: return 'g'; case H: return 'h';
    case I: return 'i'; case J: return 'j'; case K: return 'k'; case L: return 'l';
    case M: return 'm'; case N: return 'n'; case O: return 'o'; case P: return 'p';
    case Q: return 'q'; case R: return 'r'; case S: return 's'; case T: return 't';
    case U: return 'u'; case V: return 'v'; case W: return 'w'; case X: return 'x';
    case Y: return 'y'; case Z: return 'z';
    default: return std::nullopt;
    }
}

} // namespace

void TerminalKeyBindings::bindSendBytes(Key key, KeyModifier modifiers, std::string bytes) {
    if (!bytes.empty()) {
        bindings_[TerminalBindingKey{key, modifiers}] = std::move(bytes);
    }
}

void TerminalKeyBindings::bindViewAction(Key key, KeyModifier modifiers, TerminalViewAction action) {
    bindings_[TerminalBindingKey{key, modifiers}] = action;
}

void TerminalKeyBindings::unbind(Key key, KeyModifier modifiers) {
    bindings_.erase(TerminalBindingKey{key, modifiers});
}

void TerminalKeyBindings::clear() {
    bindings_.clear();
}

std::optional<TerminalBindingValue> TerminalKeyBindings::lookup(const KeyEvent& event) const {
    auto it = bindings_.find(TerminalBindingKey{event.key, event.modifiers});
    if (it == bindings_.end()) {
        return std::nullopt;
    }
    return it->second;
}

TerminalKeyBindings TerminalKeyBindings::defaultBindings() {
    TerminalKeyBindings b;

    // --- View actions (zoom, scroll) ---
    KeyModifier zoomMods = Ctrl | Super;  // Ctrl or Cmd
    b.bindViewAction(Equal, Ctrl, TerminalViewAction::ZoomIn);
    b.bindViewAction(Equal, Super, TerminalViewAction::ZoomIn);
    b.bindViewAction(Minus, Ctrl, TerminalViewAction::ZoomOut);
    b.bindViewAction(Minus, Super, TerminalViewAction::ZoomOut);
    b.bindViewAction(Num0, Ctrl, TerminalViewAction::ZoomReset);
    b.bindViewAction(Num0, Super, TerminalViewAction::ZoomReset);

    b.bindViewAction(PageUp, Shift, TerminalViewAction::ScrollPageUp);
    b.bindViewAction(PageDown, Shift, TerminalViewAction::ScrollPageDown);
    b.bindViewAction(Home, Shift, TerminalViewAction::ScrollHome);
    b.bindViewAction(End, Shift, TerminalViewAction::ScrollEnd);

    // --- PTY: simple keys (no modifiers) ---
    b.bindSendBytes(Enter, None, "\r");
    b.bindSendBytes(Tab, None, "\t");
    b.bindSendBytes(Backspace, None, "\x7f");
    b.bindSendBytes(Escape, None, "\x1b");
    b.bindSendBytes(Up, None, "\x1b[A");
    b.bindSendBytes(Down, None, "\x1b[B");
    b.bindSendBytes(Right, None, "\x1b[C");
    b.bindSendBytes(Left, None, "\x1b[D");
    b.bindSendBytes(Home, None, "\x1b[H");
    b.bindSendBytes(End, None, "\x1b[F");
    b.bindSendBytes(PageUp, None, "\x1b[5~");
    b.bindSendBytes(PageDown, None, "\x1b[6~");
    b.bindSendBytes(Delete, None, "\x1b[3~");
    b.bindSendBytes(Insert, None, "\x1b[2~");

    // --- PTY: Alt + arrows ---
    b.bindSendBytes(Up, Alt, "\x1b[1;3A");
    b.bindSendBytes(Down, Alt, "\x1b[1;3B");
    b.bindSendBytes(Right, Alt, "\x1b[1;3C");
    b.bindSendBytes(Left, Alt, "\x1b[1;3D");

    // --- PTY: Ctrl + arrows ---
    b.bindSendBytes(Up, Ctrl, "\x1b[1;5A");
    b.bindSendBytes(Down, Ctrl, "\x1b[1;5B");
    b.bindSendBytes(Right, Ctrl, "\x1b[1;5C");
    b.bindSendBytes(Left, Ctrl, "\x1b[1;5D");

    // --- PTY: Ctrl + letters A–Z (Key enum is not contiguous) ---
    b.bindSendBytes(A, Ctrl, ctrlLetter('a'));
    b.bindSendBytes(B, Ctrl, ctrlLetter('b'));
    b.bindSendBytes(C, Ctrl, ctrlLetter('c'));
    b.bindSendBytes(D, Ctrl, ctrlLetter('d'));
    b.bindSendBytes(E, Ctrl, ctrlLetter('e'));
    b.bindSendBytes(F, Ctrl, ctrlLetter('f'));
    b.bindSendBytes(G, Ctrl, ctrlLetter('g'));
    b.bindSendBytes(H, Ctrl, ctrlLetter('h'));
    b.bindSendBytes(I, Ctrl, ctrlLetter('i'));
    b.bindSendBytes(J, Ctrl, ctrlLetter('j'));
    b.bindSendBytes(K, Ctrl, ctrlLetter('k'));
    b.bindSendBytes(L, Ctrl, ctrlLetter('l'));
    b.bindSendBytes(M, Ctrl, ctrlLetter('m'));
    b.bindSendBytes(N, Ctrl, ctrlLetter('n'));
    b.bindSendBytes(O, Ctrl, ctrlLetter('o'));
    b.bindSendBytes(P, Ctrl, ctrlLetter('p'));
    b.bindSendBytes(Q, Ctrl, ctrlLetter('q'));
    b.bindSendBytes(R, Ctrl, ctrlLetter('r'));
    b.bindSendBytes(S, Ctrl, ctrlLetter('s'));
    b.bindSendBytes(T, Ctrl, ctrlLetter('t'));
    b.bindSendBytes(U, Ctrl, ctrlLetter('u'));
    b.bindSendBytes(V, Ctrl, ctrlLetter('v'));
    b.bindSendBytes(W, Ctrl, ctrlLetter('w'));
    b.bindSendBytes(X, Ctrl, ctrlLetter('x'));
    b.bindSendBytes(Y, Ctrl, ctrlLetter('y'));
    b.bindSendBytes(Z, Ctrl, ctrlLetter('z'));

    // --- PTY: Ctrl + symbols (Equal/Minus/Num0 reserved for view zoom above) ---
    b.bindSendBytes(Space, Ctrl, "\x00");
    b.bindSendBytes(LeftBracket, Ctrl, "\x1b");
    b.bindSendBytes(Backslash, Ctrl, "\x1c");
    b.bindSendBytes(RightBracket, Ctrl, "\x1d");

    return b;
}

} // namespace flux::term
