#include <Flux/Core/MouseInputHandler.hpp>
#include <iostream>

namespace flux {

void MouseInputHandler::handleMouseMove(float x, float y, const Rect& windowBounds, ImmediateModeRenderer* renderer) {
    Event event;
    event.type = Event::MouseMove;
    event.mouseMove.x = x;
    event.mouseMove.y = y;
    dispatchEvent(event, windowBounds, renderer);
}

void MouseInputHandler::handleMouseDown(int button, float x, float y, const Rect& windowBounds, ImmediateModeRenderer* renderer) {
    Event event;
    event.type = Event::MouseDown;
    event.mouseButton.x = x;
    event.mouseButton.y = y;
    event.mouseButton.button = button;
    dispatchEvent(event, windowBounds, renderer);
}

void MouseInputHandler::handleMouseUp(int button, float x, float y, const Rect& windowBounds, ImmediateModeRenderer* renderer) {
    Event event;
    event.type = Event::MouseUp;
    event.mouseButton.x = x;
    event.mouseButton.y = y;
    event.mouseButton.button = button;
    dispatchEvent(event, windowBounds, renderer);
}

void MouseInputHandler::dispatchEvent(const Event& event, const Rect& windowBounds, ImmediateModeRenderer* renderer) {
    if (renderer) {
        renderer->handleEvent(event, windowBounds);
    }
}

} // namespace flux

