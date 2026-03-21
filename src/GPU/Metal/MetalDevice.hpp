#pragma once

#include <Flux/GPU/Device.hpp>

#ifdef __OBJC__
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <dispatch/dispatch.h>
#endif

namespace flux::gpu {

#ifdef __OBJC__

class MetalBuffer : public Buffer {
public:
    MetalBuffer(id<MTLDevice> device, const BufferDesc& desc);
    void write(const void* data, size_t size, size_t offset = 0) override;
    size_t size() const override;
    id<MTLBuffer> native() const { return buffer_; }

private:
    id<MTLBuffer> buffer_;
    size_t size_;
};

class MetalTexture : public Texture {
public:
    MetalTexture(id<MTLDevice> device, const TextureDesc& desc);
    MetalTexture(id<MTLTexture> texture, uint32_t w, uint32_t h);
    void write(const void* data, uint32_t x, uint32_t y, uint32_t w, uint32_t h,
               uint32_t srcBytesPerRow) override;
    uint32_t width() const override;
    uint32_t height() const override;
    id<MTLTexture> native() const { return texture_; }
    PixelFormat format() const { return format_; }

    static MTLPixelFormat toMTL(PixelFormat fmt);
    static uint32_t bytesPerPixel(PixelFormat fmt);

private:
    id<MTLTexture> texture_;
    uint32_t width_;
    uint32_t height_;
    PixelFormat format_;
};

class MetalRenderPipeline : public RenderPipeline {
public:
    MetalRenderPipeline(id<MTLDevice> device, const RenderPipelineDesc& desc);
    id<MTLRenderPipelineState> native() const { return pipelineState_; }

private:
    id<MTLRenderPipelineState> pipelineState_;

    static MTLVertexFormat toMTLVertexFormat(VertexFormat fmt);
};

class MetalRenderPassEncoder : public RenderPassEncoder {
public:
    MetalRenderPassEncoder(id<MTLRenderCommandEncoder> encoder);
    void setPipeline(RenderPipeline* pipeline) override;
    void setVertexBuffer(uint32_t slot, Buffer* buffer, size_t offset = 0) override;
    void setIndexBuffer(Buffer* buffer) override;
    void setFragmentTexture(uint32_t slot, Texture* texture) override;
    void setScissorRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h) override;
    void draw(uint32_t vertexCount, uint32_t instanceCount = 1,
              uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1,
                     uint32_t firstIndex = 0, uint32_t firstInstance = 0) override;
    void end() override;

private:
    id<MTLRenderCommandEncoder> encoder_;
    id<MTLBuffer> indexBuffer_;
};

class MetalDevice : public Device {
public:
    /// `nsView` is an `NSView*` with `CAMetalLayer` as its layer (e.g. `+[layerClass]`).
    explicit MetalDevice(void* nsView);
    ~MetalDevice() override;

    std::unique_ptr<Buffer> createBuffer(const BufferDesc& desc) override;
    std::unique_ptr<Texture> createTexture(const TextureDesc& desc) override;
    std::unique_ptr<RenderPipeline> createRenderPipeline(const RenderPipelineDesc& desc) override;

    bool beginFrame() override;
    RenderPassEncoder* beginRenderPass(const RenderPassDesc& desc) override;
    void endRenderPass() override;
    void endFrame() override;
    uint32_t currentFrameIndex() const override { return frameIndex_; }

    void resize(uint32_t width, uint32_t height) override;
    PixelFormat swapchainFormat() const override;

    bool readPixels(int x, int y, int w, int h, std::vector<uint8_t>& out) override;
    void setReadbackEnabled(bool enabled) override;

private:
    id<MTLDevice> device_;
    id<MTLCommandQueue> commandQueue_;
    CAMetalLayer* layer_;

    id<CAMetalDrawable> currentDrawable_;
    id<MTLCommandBuffer> currentCommandBuffer_;
    std::unique_ptr<MetalRenderPassEncoder> currentEncoder_;
    dispatch_semaphore_t frameSemaphore_;
    uint32_t frameIndex_ = 0;

    id<MTLTexture> readbackTexture_;
    uint32_t readbackWidth_ = 0;
    uint32_t readbackHeight_ = 0;
    bool readbackEnabled_ = false;
};

#endif // __OBJC__

} // namespace flux::gpu
