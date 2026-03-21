#import "MetalDevice.hpp"
#import <AppKit/AppKit.h>
#import <QuartzCore/CAMetalLayer.h>
#include <algorithm>
#include <stdexcept>
#include <cstring>

namespace flux::gpu {

// =============================================================================
// MetalBuffer
// =============================================================================

MetalBuffer::MetalBuffer(id<MTLDevice> device, const BufferDesc& desc)
    : size_(desc.size)
{
    buffer_ = [device newBufferWithLength:size_ options:MTLResourceStorageModeShared];
    if (!buffer_) {
        throw std::runtime_error("Failed to create Metal buffer");
    }
}

void MetalBuffer::write(const void* data, size_t size, size_t offset) {
    std::memcpy(static_cast<uint8_t*>([buffer_ contents]) + offset, data, size);
}

size_t MetalBuffer::size() const { return size_; }

// =============================================================================
// MetalTexture
// =============================================================================

MetalTexture::MetalTexture(id<MTLDevice> device, const TextureDesc& desc)
    : width_(desc.width), height_(desc.height), format_(desc.format)
{
    MTLTextureDescriptor* texDesc = [[MTLTextureDescriptor alloc] init];
    texDesc.width = width_;
    texDesc.height = height_;
    texDesc.pixelFormat = toMTL(desc.format);
    texDesc.storageMode = MTLStorageModePrivate;

    if (desc.renderTarget) {
        texDesc.usage = MTLTextureUsageRenderTarget | MTLTextureUsageShaderRead;
        texDesc.storageMode = MTLStorageModePrivate;
    } else {
        texDesc.usage = MTLTextureUsageShaderRead;
        texDesc.storageMode = MTLStorageModeManaged;
    }

    texture_ = [device newTextureWithDescriptor:texDesc];
    if (!texture_) {
        throw std::runtime_error("Failed to create Metal texture");
    }
}

MetalTexture::MetalTexture(id<MTLTexture> texture, uint32_t w, uint32_t h)
    : texture_(texture), width_(w), height_(h), format_(PixelFormat::BGRA8)
{
}

void MetalTexture::write(const void* data, uint32_t x, uint32_t y, uint32_t w, uint32_t h,
                         uint32_t srcBytesPerRow) {
    MTLRegion region = MTLRegionMake2D(x, y, w, h);
    const uint32_t bpp = bytesPerPixel(format_);
    NSUInteger bpr = srcBytesPerRow ? static_cast<NSUInteger>(srcBytesPerRow)
                                  : static_cast<NSUInteger>(w) * bpp;
    [texture_ replaceRegion:region mipmapLevel:0 withBytes:data bytesPerRow:bpr];
}

uint32_t MetalTexture::width() const { return width_; }
uint32_t MetalTexture::height() const { return height_; }

MTLPixelFormat MetalTexture::toMTL(PixelFormat fmt) {
    switch (fmt) {
        case PixelFormat::RGBA8:    return MTLPixelFormatRGBA8Unorm;
        case PixelFormat::BGRA8:    return MTLPixelFormatBGRA8Unorm;
        case PixelFormat::R8:       return MTLPixelFormatR8Unorm;
        case PixelFormat::Depth32F: return MTLPixelFormatDepth32Float;
    }
    return MTLPixelFormatBGRA8Unorm;
}

uint32_t MetalTexture::bytesPerPixel(PixelFormat fmt) {
    switch (fmt) {
        case PixelFormat::RGBA8:    return 4;
        case PixelFormat::BGRA8:    return 4;
        case PixelFormat::R8:       return 1;
        case PixelFormat::Depth32F: return 4;
    }
    return 4;
}

// =============================================================================
// MetalRenderPipeline
// =============================================================================

MetalRenderPipeline::MetalRenderPipeline(id<MTLDevice> device, const RenderPipelineDesc& desc) {
    NSError* error = nil;

    NSString* vertMSL = [[NSString alloc] initWithBytes:desc.vertexShader.msl.data()
                                                 length:desc.vertexShader.msl.size()
                                               encoding:NSUTF8StringEncoding];
    id<MTLLibrary> vertLib = [device newLibraryWithSource:vertMSL options:nil error:&error];
    if (!vertLib) {
        throw std::runtime_error(
            std::string("Metal vertex shader compilation failed: ") +
            [[error localizedDescription] UTF8String]);
    }

    NSString* fragMSL = [[NSString alloc] initWithBytes:desc.fragmentShader.msl.data()
                                                 length:desc.fragmentShader.msl.size()
                                               encoding:NSUTF8StringEncoding];
    id<MTLLibrary> fragLib = [device newLibraryWithSource:fragMSL options:nil error:&error];
    if (!fragLib) {
        throw std::runtime_error(
            std::string("Metal fragment shader compilation failed: ") +
            [[error localizedDescription] UTF8String]);
    }

    id<MTLFunction> vertFunc = [vertLib newFunctionWithName:
        [NSString stringWithUTF8String:desc.vertexFunction.c_str()]];
    id<MTLFunction> fragFunc = [fragLib newFunctionWithName:
        [NSString stringWithUTF8String:desc.fragmentFunction.c_str()]];

    if (!vertFunc || !fragFunc) {
        throw std::runtime_error("Metal shader function not found");
    }

    MTLRenderPipelineDescriptor* pipeDesc = [[MTLRenderPipelineDescriptor alloc] init];
    pipeDesc.vertexFunction = vertFunc;
    pipeDesc.fragmentFunction = fragFunc;
    pipeDesc.colorAttachments[0].pixelFormat = MetalTexture::toMTL(desc.colorFormat);

    if (desc.blendEnabled) {
        pipeDesc.colorAttachments[0].blendingEnabled = YES;
        pipeDesc.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
        pipeDesc.colorAttachments[0].destinationRGBBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
        pipeDesc.colorAttachments[0].sourceAlphaBlendFactor = MTLBlendFactorOne;
        pipeDesc.colorAttachments[0].destinationAlphaBlendFactor = MTLBlendFactorOneMinusSourceAlpha;
    }

    if (!desc.vertexBuffers.empty()) {
        MTLVertexDescriptor* vertexDesc = [[MTLVertexDescriptor alloc] init];
        uint32_t attrIdx = 0;
        for (uint32_t i = 0; i < desc.vertexBuffers.size(); i++) {
            const auto& layout = desc.vertexBuffers[i];
            vertexDesc.layouts[i].stride = layout.stride;
            vertexDesc.layouts[i].stepFunction = layout.perInstance
                ? MTLVertexStepFunctionPerInstance
                : MTLVertexStepFunctionPerVertex;
            vertexDesc.layouts[i].stepRate = 1;

            for (const auto& attr : layout.attributes) {
                vertexDesc.attributes[attrIdx].bufferIndex = i;
                vertexDesc.attributes[attrIdx].offset = attr.offset;
                vertexDesc.attributes[attrIdx].format = toMTLVertexFormat(attr.format);
                attrIdx++;
            }
        }
        pipeDesc.vertexDescriptor = vertexDesc;
    }

    pipelineState_ = [device newRenderPipelineStateWithDescriptor:pipeDesc error:&error];
    if (!pipelineState_) {
        throw std::runtime_error(
            std::string("Metal pipeline creation failed: ") +
            [[error localizedDescription] UTF8String]);
    }
}

MTLVertexFormat MetalRenderPipeline::toMTLVertexFormat(VertexFormat fmt) {
    switch (fmt) {
        case VertexFormat::Float:      return MTLVertexFormatFloat;
        case VertexFormat::Float2:     return MTLVertexFormatFloat2;
        case VertexFormat::Float3:     return MTLVertexFormatFloat3;
        case VertexFormat::Float4:     return MTLVertexFormatFloat4;
        case VertexFormat::UByte4Norm: return MTLVertexFormatUChar4Normalized;
    }
    return MTLVertexFormatFloat4;
}

// =============================================================================
// MetalRenderPassEncoder
// =============================================================================

MetalRenderPassEncoder::MetalRenderPassEncoder(id<MTLRenderCommandEncoder> encoder)
    : encoder_(encoder)
{
}

void MetalRenderPassEncoder::setPipeline(RenderPipeline* pipeline) {
    auto* mtlPipeline = static_cast<MetalRenderPipeline*>(pipeline);
    [encoder_ setRenderPipelineState:mtlPipeline->native()];
}

void MetalRenderPassEncoder::setVertexBuffer(uint32_t slot, Buffer* buffer, size_t offset) {
    auto* mtlBuffer = static_cast<MetalBuffer*>(buffer);
    [encoder_ setVertexBuffer:mtlBuffer->native() offset:offset atIndex:slot];
}

void MetalRenderPassEncoder::setIndexBuffer(Buffer* buffer) {
    indexBuffer_ = static_cast<MetalBuffer*>(buffer)->native();
}

void MetalRenderPassEncoder::setFragmentTexture(uint32_t slot, Texture* texture) {
    auto* mtlTexture = static_cast<MetalTexture*>(texture);
    [encoder_ setFragmentTexture:mtlTexture->native() atIndex:slot];
}

void MetalRenderPassEncoder::setScissorRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    MTLScissorRect rect = { x, y, w, h };
    [encoder_ setScissorRect:rect];
}

void MetalRenderPassEncoder::draw(uint32_t vertexCount, uint32_t instanceCount,
                                  uint32_t firstVertex, uint32_t firstInstance) {
    [encoder_ drawPrimitives:MTLPrimitiveTypeTriangle
                 vertexStart:firstVertex
                 vertexCount:vertexCount
               instanceCount:instanceCount
                baseInstance:firstInstance];
}

void MetalRenderPassEncoder::drawIndexed(uint32_t indexCount, uint32_t instanceCount,
                                         uint32_t firstIndex, uint32_t firstInstance) {
    [encoder_ drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                         indexCount:indexCount
                          indexType:MTLIndexTypeUInt16
                        indexBuffer:indexBuffer_
                  indexBufferOffset:firstIndex * sizeof(uint16_t)
                      instanceCount:instanceCount
                         baseVertex:0
                       baseInstance:firstInstance];
}

void MetalRenderPassEncoder::end() {
    [encoder_ endEncoding];
}

// =============================================================================
// MetalDevice
// =============================================================================

MetalDevice::MetalDevice(void* nsViewPtr)
    : currentDrawable_(nil)
    , currentCommandBuffer_(nil)
{
    NSView* view = (__bridge NSView*)nsViewPtr;
    if (!view) {
        throw std::runtime_error("MetalDevice: null NSView");
    }
    // Layer may be nil or CALayer until laid out; layer-hosting with CAMetalLayer is reliable.
    if (view.layer == nil || ![view.layer isKindOfClass:[CAMetalLayer class]]) {
        view.wantsLayer = YES;
        CAMetalLayer* metal = [CAMetalLayer layer];
        metal.frame = view.bounds;
        view.layer = metal;
    }
    if (!view.layer || ![view.layer isKindOfClass:[CAMetalLayer class]]) {
        throw std::runtime_error("MetalDevice: could not attach CAMetalLayer to NSView");
    }

    device_ = MTLCreateSystemDefaultDevice();
    if (!device_) {
        throw std::runtime_error("No Metal-capable GPU found");
    }

    commandQueue_ = [device_ newCommandQueue];
    if (!commandQueue_) {
        throw std::runtime_error("Failed to create Metal command queue");
    }

    layer_ = (CAMetalLayer*)view.layer;
    layer_.device = device_;
    layer_.pixelFormat = MTLPixelFormatBGRA8Unorm;
    // Cheaper drawable path when GPU→CPU readback is off (see setReadbackEnabled).
    layer_.framebufferOnly = YES;

    NSSize backing = [view convertSizeToBacking:NSMakeSize(NSWidth(view.bounds), NSHeight(view.bounds))];
    layer_.drawableSize = CGSizeMake(std::max(1.0, backing.width), std::max(1.0, backing.height));

    frameSemaphore_ = dispatch_semaphore_create(1);
}

MetalDevice::~MetalDevice() {
    currentEncoder_.reset();
    currentCommandBuffer_ = nil;
    currentDrawable_ = nil;
}

std::unique_ptr<Buffer> MetalDevice::createBuffer(const BufferDesc& desc) {
    return std::make_unique<MetalBuffer>(device_, desc);
}

std::unique_ptr<Texture> MetalDevice::createTexture(const TextureDesc& desc) {
    return std::make_unique<MetalTexture>(device_, desc);
}

std::unique_ptr<RenderPipeline> MetalDevice::createRenderPipeline(const RenderPipelineDesc& desc) {
    return std::make_unique<MetalRenderPipeline>(device_, desc);
}

bool MetalDevice::beginFrame() {
    dispatch_semaphore_wait(frameSemaphore_, DISPATCH_TIME_FOREVER);

    currentDrawable_ = [layer_ nextDrawable];
    if (!currentDrawable_) {
        dispatch_semaphore_signal(frameSemaphore_);
        return false;
    }

    currentCommandBuffer_ = [commandQueue_ commandBuffer];
    if (!currentCommandBuffer_) {
        dispatch_semaphore_signal(frameSemaphore_);
        return false;
    }
    return true;
}

RenderPassEncoder* MetalDevice::beginRenderPass(const RenderPassDesc& desc) {
    MTLRenderPassDescriptor* passDesc = [MTLRenderPassDescriptor renderPassDescriptor];

    if (desc.colorTarget) {
        auto* mtlTex = static_cast<MetalTexture*>(desc.colorTarget);
        passDesc.colorAttachments[0].texture = mtlTex->native();
    } else {
        passDesc.colorAttachments[0].texture = currentDrawable_.texture;
    }

    switch (desc.loadAction) {
        case LoadAction::Clear:
            passDesc.colorAttachments[0].loadAction = MTLLoadActionClear;
            passDesc.colorAttachments[0].clearColor = MTLClearColorMake(
                desc.clearColor.r, desc.clearColor.g, desc.clearColor.b, desc.clearColor.a);
            break;
        case LoadAction::Load:
            passDesc.colorAttachments[0].loadAction = MTLLoadActionLoad;
            break;
        case LoadAction::DontCare:
            passDesc.colorAttachments[0].loadAction = MTLLoadActionDontCare;
            break;
    }
    passDesc.colorAttachments[0].storeAction = MTLStoreActionStore;

    id<MTLRenderCommandEncoder> encoder =
        [currentCommandBuffer_ renderCommandEncoderWithDescriptor:passDesc];
    if (!encoder) return nullptr;

    currentEncoder_ = std::make_unique<MetalRenderPassEncoder>(encoder);
    return currentEncoder_.get();
}

void MetalDevice::endRenderPass() {
    if (currentEncoder_) {
        currentEncoder_->end();
        currentEncoder_.reset();
    }
}

void MetalDevice::endFrame() {
    if (currentDrawable_ && currentCommandBuffer_) {
        id<MTLTexture> drawableTex = currentDrawable_.texture;
        uint32_t tw = static_cast<uint32_t>(drawableTex.width);
        uint32_t th = static_cast<uint32_t>(drawableTex.height);

        if (readbackEnabled_) {
            if (readbackWidth_ != tw || readbackHeight_ != th) {
                readbackWidth_ = tw;
                readbackHeight_ = th;

                MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];
                desc.width = tw;
                desc.height = th;
                desc.pixelFormat = MTLPixelFormatBGRA8Unorm;
                desc.storageMode = MTLStorageModeManaged;
                desc.usage = MTLTextureUsageShaderRead;
                readbackTexture_ = [device_ newTextureWithDescriptor:desc];
            }

            id<MTLBlitCommandEncoder> blit = [currentCommandBuffer_ blitCommandEncoder];
            [blit copyFromTexture:drawableTex
                      sourceSlice:0
                      sourceLevel:0
                     sourceOrigin:MTLOriginMake(0, 0, 0)
                       sourceSize:MTLSizeMake(tw, th, 1)
                        toTexture:readbackTexture_
                 destinationSlice:0
                 destinationLevel:0
                destinationOrigin:MTLOriginMake(0, 0, 0)];
            [blit synchronizeTexture:readbackTexture_ slice:0 level:0];
            [blit endEncoding];
        }

        __block dispatch_semaphore_t sem = frameSemaphore_;
        [currentCommandBuffer_ addCompletedHandler:^(id<MTLCommandBuffer>) {
            dispatch_semaphore_signal(sem);
        }];
        [currentCommandBuffer_ presentDrawable:currentDrawable_];
        [currentCommandBuffer_ commit];
    } else {
        dispatch_semaphore_signal(frameSemaphore_);
    }
    currentCommandBuffer_ = nil;
    currentDrawable_ = nil;
}

void MetalDevice::resize(uint32_t width, uint32_t height) {
    layer_.drawableSize = CGSizeMake(width, height);
}

PixelFormat MetalDevice::swapchainFormat() const {
    return PixelFormat::BGRA8;
}

void MetalDevice::setReadbackEnabled(bool enabled) {
    readbackEnabled_ = enabled;
    if (!enabled) {
        readbackTexture_ = nil;
        readbackWidth_ = 0;
        readbackHeight_ = 0;
        layer_.framebufferOnly = YES;
    } else {
        layer_.framebufferOnly = NO;
    }
}

bool MetalDevice::readPixels(int x, int y, int w, int h, std::vector<uint8_t>& out) {
    if (!readbackEnabled_ || !readbackTexture_ || w <= 0 || h <= 0) return false;

    // Wait for the last frame to finish so readbackTexture_ is populated
    dispatch_semaphore_wait(frameSemaphore_, DISPATCH_TIME_FOREVER);
    dispatch_semaphore_signal(frameSemaphore_);

    uint32_t tw = static_cast<uint32_t>(readbackTexture_.width);
    uint32_t th = static_cast<uint32_t>(readbackTexture_.height);
    uint32_t rx = static_cast<uint32_t>(std::max(x, 0));
    uint32_t ry = static_cast<uint32_t>(std::max(y, 0));
    uint32_t rw = std::min(static_cast<uint32_t>(w), tw - rx);
    uint32_t rh = std::min(static_cast<uint32_t>(h), th - ry);

    out.resize(static_cast<size_t>(rw) * rh * 4);
    NSUInteger bpr = rw * 4;

    [readbackTexture_ getBytes:out.data()
                   bytesPerRow:bpr
                    fromRegion:MTLRegionMake2D(rx, ry, rw, rh)
                   mipmapLevel:0];

    // BGRA → RGBA conversion
    for (size_t i = 0; i < out.size(); i += 4) {
        std::swap(out[i], out[i + 2]);
    }

    return true;
}

std::unique_ptr<Device> createMetalDevice(void* nsView) {
    return std::make_unique<MetalDevice>(nsView);
}

} // namespace flux::gpu
