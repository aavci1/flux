#pragma once

#include <Flux/GPU/Types.hpp>
#include <Flux/Platform/NativeGraphicsSurface.hpp>
#include <memory>
#include <vector>
#include <cstdint>

namespace flux::gpu {

class Texture;

struct RenderPassDesc {
    Texture* colorTarget = nullptr;
    LoadAction loadAction = LoadAction::Clear;
    ClearColor clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
};

class Buffer {
public:
    virtual ~Buffer() = default;
    virtual void write(const void* data, size_t size, size_t offset = 0) = 0;
    virtual size_t size() const = 0;
};

class Texture {
public:
    virtual ~Texture() = default;
    /// Upload a sub-rectangle. Pixels are R8/RGBA/etc. per format().
    /// When \p srcBytesPerRow is 0, source rows are tightly packed (\p w * bytesPerPixel).
    /// When non-zero, \p data points into a larger buffer with that row pitch (e.g. CPU atlas).
    virtual void write(const void* data, uint32_t x, uint32_t y, uint32_t w, uint32_t h,
                       uint32_t srcBytesPerRow = 0) = 0;
    virtual uint32_t width() const = 0;
    virtual uint32_t height() const = 0;
};

class RenderPipeline {
public:
    virtual ~RenderPipeline() = default;
};

class RenderPassEncoder {
public:
    virtual ~RenderPassEncoder() = default;
    virtual void setPipeline(RenderPipeline* pipeline) = 0;
    virtual void setVertexBuffer(uint32_t slot, Buffer* buffer, size_t offset = 0) = 0;
    virtual void setIndexBuffer(Buffer* buffer) = 0;
    virtual void setFragmentTexture(uint32_t slot, Texture* texture) = 0;
    virtual void setScissorRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h) = 0;
    virtual void draw(uint32_t vertexCount, uint32_t instanceCount = 1,
                      uint32_t firstVertex = 0, uint32_t firstInstance = 0) = 0;
    virtual void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1,
                             uint32_t firstIndex = 0, uint32_t firstInstance = 0) = 0;
    virtual void end() = 0;
};

class Device {
public:
    static constexpr uint32_t kMaxFramesInFlight = 2;

    virtual ~Device() = default;

    virtual uint32_t currentFrameIndex() const { return 0; }

    virtual std::unique_ptr<Buffer> createBuffer(const BufferDesc& desc) = 0;
    virtual std::unique_ptr<Texture> createTexture(const TextureDesc& desc) = 0;
    virtual std::unique_ptr<RenderPipeline> createRenderPipeline(const RenderPipelineDesc& desc) = 0;

    virtual bool beginFrame() = 0;
    virtual RenderPassEncoder* beginRenderPass(const RenderPassDesc& desc) = 0;
    virtual void endRenderPass() = 0;
    virtual void endFrame() = 0;

    virtual void resize(uint32_t width, uint32_t height) = 0;
    virtual PixelFormat swapchainFormat() const = 0;

    /// Read RGBA8 pixels from the last presented frame. Returns false if unsupported.
    virtual bool readPixels(int x, int y, int w, int h, std::vector<uint8_t>& out) {
        (void)x; (void)y; (void)w; (void)h; (void)out;
        return false;
    }

    /// When false (default), Metal may skip per-frame drawable readback and use a cheaper
    /// framebuffer path. Enable for UI test screenshots (`Window::enableTestMode`).
    virtual void setReadbackEnabled(bool enabled) { (void)enabled; }
};

std::unique_ptr<Device> createDevice(Backend backend, NativeGraphicsSurface surface);

} // namespace flux::gpu
