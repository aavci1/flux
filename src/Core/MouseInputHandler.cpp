#include <Flux/Core/MouseInputHandler.hpp>

namespace flux {

void MouseInputHandler::handleMouseMove(float x, float y, const Rect& windowBounds, ImmediateModeRenderer* renderer) {
    if (!renderer) return;
    PointerEvent e;
    e.kind = PointerEvent::Kind::Move;
    e.windowPosition = {x, y};
    e.localPosition = {x, y};
    renderer->handleEvent(e, windowBounds);
}

void MouseInputHandler::handleMouseDown(int button, float x, float y, const Rect& windowBounds, ImmediateModeRenderer* renderer) {
    if (!renderer) return;
    PointerEvent e;
    e.kind = PointerEvent::Kind::Down;
    e.windowPosition = {x, y};
    e.localPosition = {x, y};
    e.button = button;
    renderer->handleEvent(e, windowBounds);
}

void MouseInputHandler::handleMouseUp(int button, float x, float y, const Rect& windowBounds, ImmediateModeRenderer* renderer) {
    if (!renderer) return;
    PointerEvent e;
    e.kind = PointerEvent::Kind::Up;
    e.windowPosition = {x, y};
    e.localPosition = {x, y};
    e.button = button;
    renderer->handleEvent(e, windowBounds);
}

void MouseInputHandler::handleMouseScroll(float x, float y, float deltaX, float deltaY, const Rect& windowBounds, ImmediateModeRenderer* renderer) {
    if (!renderer) return;
    PointerEvent e;
    e.kind = PointerEvent::Kind::Scroll;
    e.windowPosition = {x, y};
    e.localPosition = {x, y};
    e.scrollDeltaX = deltaX;
    e.scrollDeltaY = deltaY;
    renderer->handleEvent(e, windowBounds);
}

} // namespace flux

