#pragma once

#include <Flux/Core/Types.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <string>
#include <memory>
#include <vector>
#include <cstdint>

namespace flux {

class PlatformRenderer {
public:
    virtual ~PlatformRenderer() = default;

    virtual bool initialize(int width, int height, float dpiScaleX = 1.0f, float dpiScaleY = 1.0f) = 0;
    virtual void cleanup() = 0;

    virtual RenderContext* renderContext() = 0;

    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;

    virtual void resize(int width, int height) = 0;

    virtual void swapBuffers() = 0;

    /// Read back the current framebuffer as RGBA8 pixels (top-left origin).
    /// Returns false if readback is unsupported or fails.
    virtual bool readPixels(int x, int y, int w, int h, std::vector<uint8_t>& out) = 0;
};

} // namespace flux
