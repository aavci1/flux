#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Graphics/Renderer.hpp>

namespace flux {

/**
 * @brief Handles mouse input events
 * 
 * Responsible for:
 * - Processing mouse movement, button clicks
 * - Dispatching events to the renderer for hit testing
 */
class MouseInputHandler {
public:
    MouseInputHandler() = default;
    ~MouseInputHandler() = default;

    /**
     * @brief Handle mouse movement
     */
    void handleMouseMove(float x, float y, const Rect& windowBounds, ImmediateModeRenderer* renderer);
    
    /**
     * @brief Handle mouse button down
     */
    void handleMouseDown(int button, float x, float y, const Rect& windowBounds, ImmediateModeRenderer* renderer);
    
    /**
     * @brief Handle mouse button up
     */
    void handleMouseUp(int button, float x, float y, const Rect& windowBounds, ImmediateModeRenderer* renderer);
    
    /**
     * @brief Handle mouse scroll/wheel events
     */
    void handleMouseScroll(float x, float y, float deltaX, float deltaY, const Rect& windowBounds, ImmediateModeRenderer* renderer);

private:
    void dispatchEvent(const Event& event, const Rect& windowBounds, ImmediateModeRenderer* renderer);
};

} // namespace flux

