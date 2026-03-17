#pragma once

#include <Flux/Platform/PlatformRenderer.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <nanovg.h>
#include <stdexcept>

namespace flux {

class NanoVGRenderer : public PlatformRenderer {
private:
    NVGcontext* nvgContext_;
    std::unique_ptr<RenderContext> renderContext_;
    int width_;
    int height_;
    float dpiScaleX_;
    float dpiScaleY_;

public:
    NanoVGRenderer();
    ~NanoVGRenderer() override;

    bool initialize(int width, int height, float dpiScaleX = 1.0f, float dpiScaleY = 1.0f) override;
    void cleanup() override;

    RenderContext* renderContext() override;

    void beginFrame() override;
    void endFrame() override;

    void resize(int width, int height) override;
    void updateDPIScale(float dpiScaleX, float dpiScaleY);

    void swapBuffers() override;

    NVGcontext* nvgContext() const { return nvgContext_; }
};

} // namespace flux
