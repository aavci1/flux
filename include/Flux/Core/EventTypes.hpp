#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Core/KeyEvent.hpp>

namespace flux {

struct EventBase {
    enum class Phase { Capture, Target, Bubble };
    Phase phase = Phase::Target;
    bool handled = false;

    void stopPropagation() { handled = true; }
};

struct PointerEvent : EventBase {
    enum class Kind { Move, Down, Up, Scroll, Enter, Leave };
    Kind kind = Kind::Move;
    Point windowPosition;
    Point localPosition;
    int button = 0;
    float scrollDeltaX = 0;
    float scrollDeltaY = 0;
    KeyModifier modifiers = KeyModifier::None;
};

} // namespace flux
