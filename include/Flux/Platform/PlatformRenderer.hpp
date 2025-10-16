#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <string>
#include <memory>

namespace flux {

/**
 * Abstract base class for platform-specific renderer implementations.
 * This allows the RenderContext to work with any rendering system
 * without being tightly coupled to specific implementations.
 */
class PlatformRenderer {
public:
    virtual ~PlatformRenderer() = default;

    // Renderer initialization
    virtual bool initialize(int width, int height, float dpiScaleX = 1.0f, float dpiScaleY = 1.0f) = 0;
    virtual void cleanup() = 0;

    // Render context access
    virtual RenderContext* renderContext() = 0;

    // Frame management
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;

    // Resize handling
    virtual void resize(int width, int height) = 0;

    // Platform-specific methods
    virtual void swapBuffers() = 0;
};

} // namespace flux
