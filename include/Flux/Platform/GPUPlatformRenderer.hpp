#pragma once

#include <Flux/Platform/PlatformRenderer.hpp>
#include <Flux/Graphics/GPURenderContext.hpp>
#include <Flux/Graphics/GPURendererBackend.hpp>
#include <Flux/GPU/Device.hpp>
#include <Flux/Platform/NativeGraphicsSurface.hpp>
#include <memory>

namespace flux {

class GPUPlatformRenderer : public PlatformRenderer {
public:
    explicit GPUPlatformRenderer(gpu::Backend backend);
    ~GPUPlatformRenderer() override;

    bool initialize(int width, int height, float dpiScaleX = 1.0f, float dpiScaleY = 1.0f) override;
    void cleanup() override;

    RenderContext* renderContext() override;

    void beginFrame() override;
    void endFrame() override;

    void resize(int width, int height) override;
    void updateDPIScale(float dpiScaleX, float dpiScaleY);

    void swapBuffers() override;

    bool readPixels(int x, int y, int w, int h, std::vector<uint8_t>& out) override;

    void setGraphicsSurface(gpu::NativeGraphicsSurface surface) { surface_ = surface; }
    gpu::Device* device() const { return device_.get(); }

private:
    gpu::Backend backend_;
    gpu::NativeGraphicsSurface surface_{};
    std::unique_ptr<gpu::Device> device_;
    std::unique_ptr<GPURendererBackend> gpuBackend_;
    std::unique_ptr<GPURenderContext> renderCtx_;
    RenderCommandBuffer commandBuffer_;
    int width_ = 0, height_ = 0;
    float dpiScaleX_ = 1.0f, dpiScaleY_ = 1.0f;
};

} // namespace flux
