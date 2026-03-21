#include <Flux/Graphics/GPURendererBackend.hpp>
#include <Flux/Platform/PathUtil.hpp>
#include <Flux/Platform/PlatformRegistry.hpp>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <span>
#include <string_view>
#include <vector>
#include <stdexcept>

#if defined(FLUX_HAS_EMBEDDED_SHADERS)
#include "FluxEmbeddedShaders.hpp"
#endif

namespace flux {

static std::string getExeDir() {
    if (auto* util = PlatformRegistry::instance().pathUtil()) {
        return util->executableDirectory();
    }
    return {};
}

static const std::string& shaderDir() {
    static std::string dir = getExeDir() + "generated/shaders/";
    return dir;
}

static std::string readFileStr(const std::string& name) {
    // Try exe-relative path first, then CWD-relative fallback
    for (const auto& base : {shaderDir(), std::string("generated/shaders/")}) {
        std::ifstream f(base + name);
        if (f) return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
    }
    return {};
}

static std::vector<uint8_t> readFileBin(const std::string& name) {
    for (const auto& base : {shaderDir(), std::string("generated/shaders/")}) {
        std::ifstream f(base + name, std::ios::binary | std::ios::ate);
        if (!f) continue;
        auto sz = f.tellg();
        std::vector<uint8_t> buf(static_cast<size_t>(sz));
        f.seekg(0);
        f.read(reinterpret_cast<char*>(buf.data()), sz);
        return buf;
    }
    return {};
}

// Unit quad: 6 vertices (2 triangles), each is float2
static constexpr float kQuadVertices[] = {
    -1, -1,   1, -1,   1,  1,
    -1, -1,   1,  1,  -1,  1,
};

GPURendererBackend::GPURendererBackend(gpu::Device* device)
    : device_(device)
{
    glyphAtlas_ = std::make_unique<GlyphAtlas>(device_);
    imageCache_ = std::make_unique<ImageCache>(device_);
    compiler_.setGlyphAtlas(glyphAtlas_.get());
}

void GPURendererBackend::setViewportSize(float width, float height) {
    viewportWidth_ = width;
    viewportHeight_ = height;
}

void GPURendererBackend::setDPIScale(float scaleX, float scaleY) {
    dpiScaleX_ = scaleX;
    dpiScaleY_ = scaleY;
}

void GPURendererBackend::ensureQuadVertexBuffer() {
    if (quadVB_) return;
    gpu::BufferDesc desc;
    desc.size = sizeof(kQuadVertices);
    desc.usage = gpu::BufferUsage::Vertex;
    quadVB_ = device_->createBuffer(desc);
    quadVB_->write(kQuadVertices, sizeof(kQuadVertices));
}

void GPURendererBackend::ensurePipelines() {
    if (pipelinesReady_) return;
    ensureQuadVertexBuffer();

#if defined(FLUX_HAS_EMBEDDED_SHADERS)
    const std::string_view vertMSLStr = flux::gpu::embedded::msl_sdf_quad_vert_glsl();
    const std::string_view lineVertMSLStr = flux::gpu::embedded::msl_sdf_line_vert_glsl();
    const std::string_view rectMSLStr = flux::gpu::embedded::msl_rect_frag_glsl();
    const std::string_view circleMSLStr = flux::gpu::embedded::msl_circle_frag_glsl();
    const std::string_view lineMSLStr = flux::gpu::embedded::msl_line_frag_glsl();

    const auto vertSPV = flux::gpu::embedded::spv_sdf_quad_vert_glsl();
    const auto lineVertSPV = flux::gpu::embedded::spv_sdf_line_vert_glsl();
    const auto rectSPV = flux::gpu::embedded::spv_rect_frag_glsl();
    const auto circleSPV = flux::gpu::embedded::spv_circle_frag_glsl();
    const auto lineSPV = flux::gpu::embedded::spv_line_frag_glsl();
#else
    static const std::string vertMSLStrStored = readFileStr("sdf_quad.vert.glsl.metal");
    static const std::string lineVertMSLStrStored = readFileStr("sdf_line.vert.glsl.metal");
    static const std::string rectMSLStrStored = readFileStr("rect.frag.glsl.metal");
    static const std::string circleMSLStrStored = readFileStr("circle.frag.glsl.metal");
    static const std::string lineMSLStrStored = readFileStr("line.frag.glsl.metal");
    const std::string_view vertMSLStr = vertMSLStrStored;
    const std::string_view lineVertMSLStr = lineVertMSLStrStored;
    const std::string_view rectMSLStr = rectMSLStrStored;
    const std::string_view circleMSLStr = circleMSLStrStored;
    const std::string_view lineMSLStr = lineMSLStrStored;

    static const std::vector<uint8_t> vertSPVStored = readFileBin("sdf_quad.vert.glsl.spv");
    static const std::vector<uint8_t> lineVertSPVStored = readFileBin("sdf_line.vert.glsl.spv");
    static const std::vector<uint8_t> rectSPVStored = readFileBin("rect.frag.glsl.spv");
    static const std::vector<uint8_t> circleSPVStored = readFileBin("circle.frag.glsl.spv");
    static const std::vector<uint8_t> lineSPVStored = readFileBin("line.frag.glsl.spv");
    const std::span<const uint8_t> vertSPV = vertSPVStored;
    const std::span<const uint8_t> lineVertSPV = lineVertSPVStored;
    const std::span<const uint8_t> rectSPV = rectSPVStored;
    const std::span<const uint8_t> circleSPV = circleSPVStored;
    const std::span<const uint8_t> lineSPV = lineSPVStored;
#endif

    // Per-vertex layout: float2 position
    gpu::VertexBufferLayout vertLayout;
    vertLayout.stride = sizeof(float) * 2;
    vertLayout.perInstance = false;
    vertLayout.attributes = {{0, 0, gpu::VertexFormat::Float2}};

    // Per-instance layout: SDFQuadInstance (96 bytes)
    gpu::VertexBufferLayout instLayout;
    instLayout.stride = sizeof(SDFQuadInstance);
    instLayout.perInstance = true;
    instLayout.attributes = {
        {1, 0,  gpu::VertexFormat::Float4},  // rect
        {2, 16, gpu::VertexFormat::Float4},  // corners
        {3, 32, gpu::VertexFormat::Float4},  // fillColor
        {4, 48, gpu::VertexFormat::Float4},  // strokeColor
        {5, 64, gpu::VertexFormat::Float2},  // strokeWidth + opacity
        {6, 72, gpu::VertexFormat::Float2},  // viewport
        {7, 80, gpu::VertexFormat::Float4},  // rotation + pad
    };

    auto makeShaderSrc = [](std::string_view msl, std::span<const uint8_t> spv) {
        gpu::ShaderSource s;
        s.msl = msl;
        if (!spv.empty()) {
            s.spirv = spv;
        }
        return s;
    };

    auto makePipeline = [&](std::string_view fragMSL, std::span<const uint8_t> fragSPV) {
        gpu::RenderPipelineDesc desc;
        desc.vertexShader = makeShaderSrc(vertMSLStr, vertSPV);
        desc.fragmentShader = makeShaderSrc(fragMSL, fragSPV);
        desc.vertexFunction = "main0";
        desc.fragmentFunction = "main0";
        desc.vertexBuffers = {vertLayout, instLayout};
        desc.colorFormat = device_->swapchainFormat();
        desc.blendEnabled = true;
        return device_->createRenderPipeline(desc);
    };

    rectPipeline_ = makePipeline(rectMSLStr, rectSPV);
    circlePipeline_ = makePipeline(circleMSLStr, circleSPV);
    {
        gpu::RenderPipelineDesc lineDesc;
        lineDesc.vertexShader = makeShaderSrc(lineVertMSLStr, lineVertSPV);
        lineDesc.fragmentShader = makeShaderSrc(lineMSLStr, lineSPV);
        lineDesc.vertexFunction = "main0";
        lineDesc.fragmentFunction = "main0";
        lineDesc.vertexBuffers = {vertLayout, instLayout};
        lineDesc.colorFormat = device_->swapchainFormat();
        lineDesc.blendEnabled = true;
        linePipeline_ = device_->createRenderPipeline(lineDesc);
    }

    // Glyph pipeline
#if defined(FLUX_HAS_EMBEDDED_SHADERS)
    const std::string_view glyphVertMSL = flux::gpu::embedded::msl_glyph_vert_glsl();
    const std::string_view glyphFragMSL = flux::gpu::embedded::msl_glyph_frag_glsl();
    const auto glyphVertSPV = flux::gpu::embedded::spv_glyph_vert_glsl();
    const auto glyphFragSPV = flux::gpu::embedded::spv_glyph_frag_glsl();
#else
    static const std::string glyphVertMSLStored = readFileStr("glyph.vert.glsl.metal");
    static const std::string glyphFragMSLStored = readFileStr("glyph.frag.glsl.metal");
    const std::string_view glyphVertMSL = glyphVertMSLStored;
    const std::string_view glyphFragMSL = glyphFragMSLStored;
    static const std::vector<uint8_t> glyphVertSPVStored = readFileBin("glyph.vert.glsl.spv");
    static const std::vector<uint8_t> glyphFragSPVStored = readFileBin("glyph.frag.glsl.spv");
    const std::span<const uint8_t> glyphVertSPV = glyphVertSPVStored;
    const std::span<const uint8_t> glyphFragSPV = glyphFragSPVStored;
#endif

    gpu::VertexBufferLayout glyphInstLayout;
    glyphInstLayout.stride = sizeof(GlyphInstance);
    glyphInstLayout.perInstance = true;
    glyphInstLayout.attributes = {
        {1, 0,  gpu::VertexFormat::Float4},  // screenRect
        {2, 16, gpu::VertexFormat::Float4},  // uvRect
        {3, 32, gpu::VertexFormat::Float4},  // color
        {4, 48, gpu::VertexFormat::Float2},  // viewport
        {5, 56, gpu::VertexFormat::Float4},  // rotation + pad
    };

    {
        gpu::RenderPipelineDesc desc;
        desc.vertexShader = makeShaderSrc(glyphVertMSL, glyphVertSPV);
        desc.fragmentShader = makeShaderSrc(glyphFragMSL, glyphFragSPV);
        desc.vertexFunction = "main0";
        desc.fragmentFunction = "main0";
        desc.vertexBuffers = {vertLayout, glyphInstLayout};
        desc.colorFormat = device_->swapchainFormat();
        desc.blendEnabled = true;
        glyphPipeline_ = device_->createRenderPipeline(desc);
    }

    // Path pipeline — non-instanced, per-vertex color
#if defined(FLUX_HAS_EMBEDDED_SHADERS)
    const std::string_view pathVertMSL = flux::gpu::embedded::msl_path_vert_glsl();
    const std::string_view pathFragMSL = flux::gpu::embedded::msl_path_frag_glsl();
    const auto pathVertSPV = flux::gpu::embedded::spv_path_vert_glsl();
    const auto pathFragSPV = flux::gpu::embedded::spv_path_frag_glsl();
#else
    static const std::string pathVertMSLStored = readFileStr("path.vert.glsl.metal");
    static const std::string pathFragMSLStored = readFileStr("path.frag.glsl.metal");
    const std::string_view pathVertMSL = pathVertMSLStored;
    const std::string_view pathFragMSL = pathFragMSLStored;
    static const std::vector<uint8_t> pathVertSPVStored = readFileBin("path.vert.glsl.spv");
    static const std::vector<uint8_t> pathFragSPVStored = readFileBin("path.frag.glsl.spv");
    const std::span<const uint8_t> pathVertSPV = pathVertSPVStored;
    const std::span<const uint8_t> pathFragSPV = pathFragSPVStored;
#endif

    {
        gpu::VertexBufferLayout pathVertLayout;
        pathVertLayout.stride = sizeof(PathVertex);
        pathVertLayout.perInstance = false;
        pathVertLayout.attributes = {
            {0, 0,  gpu::VertexFormat::Float2},   // pos
            {1, 8,  gpu::VertexFormat::Float4},   // color
            {2, 24, gpu::VertexFormat::Float2},   // viewport
        };

        gpu::RenderPipelineDesc desc;
        desc.vertexShader = makeShaderSrc(pathVertMSL, pathVertSPV);
        desc.fragmentShader = makeShaderSrc(pathFragMSL, pathFragSPV);
        desc.vertexFunction = "main0";
        desc.fragmentFunction = "main0";
        desc.vertexBuffers = {pathVertLayout};
        desc.colorFormat = device_->swapchainFormat();
        desc.blendEnabled = true;
        pathPipeline_ = device_->createRenderPipeline(desc);
    }

    // Image pipeline — instanced textured quads
#if defined(FLUX_HAS_EMBEDDED_SHADERS)
    const std::string_view imageVertMSL = flux::gpu::embedded::msl_image_vert_glsl();
    const std::string_view imageFragMSL = flux::gpu::embedded::msl_image_frag_glsl();
    const auto imageVertSPV = flux::gpu::embedded::spv_image_vert_glsl();
    const auto imageFragSPV = flux::gpu::embedded::spv_image_frag_glsl();
#else
    static const std::string imageVertMSLStored = readFileStr("image.vert.glsl.metal");
    static const std::string imageFragMSLStored = readFileStr("image.frag.glsl.metal");
    const std::string_view imageVertMSL = imageVertMSLStored;
    const std::string_view imageFragMSL = imageFragMSLStored;
    static const std::vector<uint8_t> imageVertSPVStored = readFileBin("image.vert.glsl.spv");
    static const std::vector<uint8_t> imageFragSPVStored = readFileBin("image.frag.glsl.spv");
    const std::span<const uint8_t> imageVertSPV = imageVertSPVStored;
    const std::span<const uint8_t> imageFragSPV = imageFragSPVStored;
#endif

    {
        gpu::VertexBufferLayout imgInstLayout;
        imgInstLayout.stride = sizeof(ImageInstance);
        imgInstLayout.perInstance = true;
        imgInstLayout.attributes = {
            {1, 0,  gpu::VertexFormat::Float4},   // screenRect
            {2, 16, gpu::VertexFormat::Float4},   // uvRect
            {3, 32, gpu::VertexFormat::Float4},   // tint
            {4, 48, gpu::VertexFormat::Float2},   // viewport
            {5, 56, gpu::VertexFormat::Float4},   // rotation + pad
        };

        gpu::RenderPipelineDesc desc;
        desc.vertexShader = makeShaderSrc(imageVertMSL, imageVertSPV);
        desc.fragmentShader = makeShaderSrc(imageFragMSL, imageFragSPV);
        desc.vertexFunction = "main0";
        desc.fragmentFunction = "main0";
        desc.vertexBuffers = {vertLayout, imgInstLayout};
        desc.colorFormat = device_->swapchainFormat();
        desc.blendEnabled = true;
        imagePipeline_ = device_->createRenderPipeline(desc);
    }

    pipelinesReady_ = true;
}

/// Grow GPU vertex buffer for instance data: high-water with slack to reduce realloc spikes.
template<typename T>
static void ensureInstanceBuffer(gpu::Device* device, std::unique_ptr<gpu::Buffer>& buf,
                                 size_t& capacity, size_t needed, size_t minCapacity = 64) {
    if (needed == 0) return;
    if (!buf || capacity < needed) {
        const size_t oldCap = capacity;
        size_t newCap = std::max(needed, minCapacity);
        if (oldCap > 0) {
            newCap = std::max(needed, oldCap + (oldCap >> 1));
        }
        capacity = newCap;
        gpu::BufferDesc desc;
        desc.size = capacity * sizeof(T);
        desc.usage = gpu::BufferUsage::Vertex;
        buf = device->createBuffer(desc);
    }
}

static gpu::Texture* resolveImageTexture(ImageCache* cache, const ImageDrawCmd& d) {
    if (!cache) return nullptr;
    if (!d.path.empty()) return cache->getOrLoad(d.path);
    if (d.imageId > 0) return cache->getById(d.imageId);
    return nullptr;
}

static void ensureImageInstanceBuffer(gpu::Device* device,
                                      std::unique_ptr<gpu::Buffer>& buf,
                                      size_t& capacity, size_t needed) {
    ensureInstanceBuffer<ImageInstance>(device, buf, capacity, needed, 16);
}

void GPURendererBackend::execute(const RenderCommandBuffer& buffer) {
    if (viewportWidth_ <= 0 || viewportHeight_ <= 0) return;

    ensurePipelines();

    compiler_.compile(buffer, viewportWidth_, viewportHeight_,
                      dpiScaleX_, dpiScaleY_, compiledBatches_);
    uploadAndDraw(compiledBatches_);
}

void GPURendererBackend::uploadAndDraw(const CompiledBatches& batches) {
    // Ensure and upload all instance buffers (shared across draw groups)
    ensureInstanceBuffer<SDFQuadInstance>(device_, rectInstanceBuffer_, rectBufferCapacity_,
                                          batches.rects.size());
    ensureInstanceBuffer<SDFQuadInstance>(device_, circleInstanceBuffer_, circleBufferCapacity_,
                                          batches.circles.size());
    ensureInstanceBuffer<SDFQuadInstance>(device_, lineInstanceBuffer_, lineBufferCapacity_,
                                          batches.lines.size());

    if (!batches.rects.empty())
        rectInstanceBuffer_->write(batches.rects.data(),
                                   batches.rects.size() * sizeof(SDFQuadInstance));
    if (!batches.circles.empty())
        circleInstanceBuffer_->write(batches.circles.data(),
                                     batches.circles.size() * sizeof(SDFQuadInstance));
    if (!batches.lines.empty())
        lineInstanceBuffer_->write(batches.lines.data(),
                                   batches.lines.size() * sizeof(SDFQuadInstance));

    if (!batches.glyphs.empty() && glyphAtlas_) {
        glyphAtlas_->uploadIfDirty();
        ensureInstanceBuffer<GlyphInstance>(device_, glyphInstanceBuffer_, glyphBufferCapacity_,
                                            batches.glyphs.size());
        glyphInstanceBuffer_->write(batches.glyphs.data(),
                                     batches.glyphs.size() * sizeof(GlyphInstance));
    }

    if (!batches.pathVertices.empty()) {
        const size_t pathCount = batches.pathVertices.size();
        ensureInstanceBuffer<PathVertex>(device_, pathVertexBuffer_, pathBufferCapacity_, pathCount,
                                         256);
        pathVertexBuffer_->write(batches.pathVertices.data(), pathCount * sizeof(PathVertex));
    }

    if (!device_->beginFrame()) return;

    gpu::RenderPassDesc passDesc;
    passDesc.clearColor = batches.clearColor;
    auto* enc = device_->beginRenderPass(passDesc);
    if (!enc) { device_->endFrame(); return; }

    uint32_t vpW = static_cast<uint32_t>(viewportWidth_);
    uint32_t vpH = static_cast<uint32_t>(viewportHeight_);

    for (const auto& group : batches.groups) {
        // Set scissor for this group
        if (group.scissor.active) {
            enc->setScissorRect(
                static_cast<uint32_t>(group.scissor.x),
                static_cast<uint32_t>(group.scissor.y),
                static_cast<uint32_t>(std::max(0.0f, group.scissor.width)),
                static_cast<uint32_t>(std::max(0.0f, group.scissor.height)));
        } else {
            enc->setScissorRect(0, 0, vpW, vpH);
        }

        // Draw in command order (batch consecutive image ops that share the same texture)
        for (size_t i = 0; i < group.drawOps.size();) {
            const auto& op = group.drawOps[i];
            switch (op.type) {
                case DrawOpType::Rect:
                    enc->setPipeline(rectPipeline_.get());
                    enc->setVertexBuffer(0, quadVB_.get());
                    enc->setVertexBuffer(1, rectInstanceBuffer_.get());
                    enc->draw(6, op.count, 0, group.rectOffset + op.offset);
                    ++i;
                    break;
                case DrawOpType::Circle:
                    enc->setPipeline(circlePipeline_.get());
                    enc->setVertexBuffer(0, quadVB_.get());
                    enc->setVertexBuffer(1, circleInstanceBuffer_.get());
                    enc->draw(6, op.count, 0, group.circleOffset + op.offset);
                    ++i;
                    break;
                case DrawOpType::Line:
                    enc->setPipeline(linePipeline_.get());
                    enc->setVertexBuffer(0, quadVB_.get());
                    enc->setVertexBuffer(1, lineInstanceBuffer_.get());
                    enc->draw(6, op.count, 0, group.lineOffset + op.offset);
                    ++i;
                    break;
                case DrawOpType::Glyph:
                    if (glyphAtlas_ && op.count > 0) {
                        auto* pageTex = glyphAtlas_->texture(op.pageIndex);
                        if (pageTex) {
                            enc->setPipeline(glyphPipeline_.get());
                            enc->setVertexBuffer(0, quadVB_.get());
                            enc->setVertexBuffer(1, glyphInstanceBuffer_.get());
                            enc->setFragmentTexture(0, pageTex);
                            enc->draw(6, op.count, 0, group.glyphOffset + op.offset);
                        }
                    }
                    ++i;
                    break;
                case DrawOpType::Path:
                    if (op.count > 0) {
                        enc->setPipeline(pathPipeline_.get());
                        enc->setVertexBuffer(0, pathVertexBuffer_.get());
                        enc->draw(op.count, 1, group.pathOffset + op.offset, 0);
                    }
                    ++i;
                    break;
                case DrawOpType::Image: {
                    if (op.offset >= group.imageDraws.size()) {
                        ++i;
                        break;
                    }
                    const auto& draw0 = group.imageDraws[op.offset];
                    gpu::Texture* tex = resolveImageTexture(imageCache_.get(), draw0);
                    if (!tex) {
                        ++i;
                        break;
                    }
                    imageBatchScratch_.clear();
                    imageBatchScratch_.push_back(draw0.instance);
                    size_t j = i + 1;
                    while (j < group.drawOps.size() &&
                           group.drawOps[j].type == DrawOpType::Image) {
                        const auto& oj = group.drawOps[j];
                        if (oj.offset >= group.imageDraws.size()) break;
                        const auto& dj = group.imageDraws[oj.offset];
                        gpu::Texture* tj = resolveImageTexture(imageCache_.get(), dj);
                        if (tj != tex) break;
                        imageBatchScratch_.push_back(dj.instance);
                        ++j;
                    }
                    const uint32_t instCount = static_cast<uint32_t>(imageBatchScratch_.size());
                    ensureImageInstanceBuffer(device_, imageInstanceBuffer_, imageBufferCapacity_,
                                              imageBatchScratch_.size());
                    imageInstanceBuffer_->write(imageBatchScratch_.data(),
                                                imageBatchScratch_.size() * sizeof(ImageInstance));
                    enc->setPipeline(imagePipeline_.get());
                    enc->setVertexBuffer(0, quadVB_.get());
                    enc->setVertexBuffer(1, imageInstanceBuffer_.get());
                    enc->setFragmentTexture(0, tex);
                    enc->draw(6, instCount);
                    i = j;
                    break;
                }
            }
        }
    }

    device_->endRenderPass();
    device_->endFrame();
}

} // namespace flux
