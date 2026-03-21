#pragma once

#include <Flux/Graphics/RenderCommandBuffer.hpp>
#include <Flux/Graphics/CommandCompiler.hpp>
#include <Flux/Graphics/GlyphAtlas.hpp>
#include <Flux/Graphics/ImageCache.hpp>
#include <Flux/GPU/Device.hpp>
#include <memory>
#include <vector>

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

    static constexpr uint32_t kFrames = gpu::Device::kMaxFramesInFlight;

    struct FrameBuffers {
        std::unique_ptr<gpu::Buffer> rect;
        std::unique_ptr<gpu::Buffer> circle;
        std::unique_ptr<gpu::Buffer> line;
        std::unique_ptr<gpu::Buffer> glyph;
        std::unique_ptr<gpu::Buffer> path;
        std::unique_ptr<gpu::Buffer> image;
        size_t rectCap = 0, circleCap = 0, lineCap = 0;
        size_t glyphCap = 0, pathCap = 0, imageCap = 0;
    };
    FrameBuffers frameBuffers_[kFrames];

    CompiledBatches compiledBatches_;

    std::unique_ptr<GlyphAtlas> glyphAtlas_;
    std::unique_ptr<ImageCache> imageCache_;

    /// Reused to batch consecutive image draws that share a texture.
    std::vector<ImageInstance> imageBatchScratch_;

    bool pipelinesReady_ = false;
};

} // namespace flux
