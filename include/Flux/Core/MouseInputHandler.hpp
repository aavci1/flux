#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Graphics/Renderer.hpp>

namespace flux {

class MouseInputHandler {
public:
    MouseInputHandler() = default;
    ~MouseInputHandler() = default;

    void handleMouseMove(float x, float y, const Rect& windowBounds, ImmediateModeRenderer* renderer);
    void handleMouseDown(int button, float x, float y, const Rect& windowBounds, ImmediateModeRenderer* renderer);
    void handleMouseUp(int button, float x, float y, const Rect& windowBounds, ImmediateModeRenderer* renderer);
    void handleMouseScroll(float x, float y, float deltaX, float deltaY, const Rect& windowBounds, ImmediateModeRenderer* renderer);
};

} // namespace flux

