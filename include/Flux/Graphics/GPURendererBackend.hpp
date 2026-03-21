#pragma once

#include <Flux/Graphics/RenderCommandBuffer.hpp>
#include <Flux/Graphics/CommandCompiler.hpp>
#include <Flux/Graphics/GlyphAtlas.hpp>
#include <Flux/Graphics/ImageCache.hpp>
#include <Flux/GPU/Device.hpp>
#include <memory>

namespace flux {

class GPURendererBackend : public RenderBackend {
public:
    explicit GPURendererBackend(gpu::Device* device);
    void execute(const RenderCommandBuffer& buffer) override;

    void setViewportSize(float width, float height);
    void setDPIScale(float scaleX, float scaleY);
    GlyphAtlas* glyphAtlas() { return glyphAtlas_.get(); }
    ImageCache* imageCache() { return imageCache_.get(); }

private:
    void ensurePipelines();
    void ensureQuadVertexBuffer();
    void uploadAndDraw(const CompiledBatches& batches);

    gpu::Device* device_;
    CommandCompiler compiler_;

    float viewportWidth_ = 0;
    float viewportHeight_ = 0;
    float dpiScaleX_ = 1.0f;
    float dpiScaleY_ = 1.0f;

    std::unique_ptr<gpu::RenderPipeline> rectPipeline_;
    std::unique_ptr<gpu::RenderPipeline> circlePipeline_;
    std::unique_ptr<gpu::RenderPipeline> linePipeline_;
    std::unique_ptr<gpu::RenderPipeline> glyphPipeline_;
    std::unique_ptr<gpu::RenderPipeline> pathPipeline_;
    std::unique_ptr<gpu::RenderPipeline> imagePipeline_;

    std::unique_ptr<gpu::Buffer> quadVB_;

    std::unique_ptr<gpu::Buffer> rectInstanceBuffer_;
    std::unique_ptr<gpu::Buffer> circleInstanceBuffer_;
    std::unique_ptr<gpu::Buffer> lineInstanceBuffer_;
    std::unique_ptr<gpu::Buffer> glyphInstanceBuffer_;
    std::unique_ptr<gpu::Buffer> pathVertexBuffer_;
    std::unique_ptr<gpu::Buffer> imageInstanceBuffer_;

    CompiledBatches compiledBatches_;

    size_t rectBufferCapacity_ = 0;
    size_t circleBufferCapacity_ = 0;
    size_t lineBufferCapacity_ = 0;
    size_t glyphBufferCapacity_ = 0;
    size_t pathBufferCapacity_ = 0;
    size_t imageBufferCapacity_ = 0;

    std::unique_ptr<GlyphAtlas> glyphAtlas_;
    std::unique_ptr<ImageCache> imageCache_;

    bool pipelinesReady_ = false;
};

} // namespace flux
