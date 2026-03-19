#include <Flux/Graphics/GPURendererBackend.hpp>
#include <cstring>
#include <algorithm>
#include <fstream>
#include <vector>
#include <stdexcept>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#elif defined(__linux__)
#include <unistd.h>
#include <climits>
#endif

namespace flux {

static std::string getExeDir() {
#ifdef __APPLE__
    char buf[4096];
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) == 0) {
        std::string p(buf);
        auto pos = p.rfind('/');
        if (pos != std::string::npos) return p.substr(0, pos + 1);
    }
#elif defined(__linux__)
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len > 0) {
        buf[len] = '\0';
        std::string p(buf);
        auto pos = p.rfind('/');
        if (pos != std::string::npos) return p.substr(0, pos + 1);
    }
#endif
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

    static std::string vertMSLStr = readFileStr("sdf_quad.vert.glsl.metal");
    static std::string rectMSLStr = readFileStr("rect.frag.glsl.metal");
    static std::string circleMSLStr = readFileStr("circle.frag.glsl.metal");
    static std::string lineMSLStr = readFileStr("line.frag.glsl.metal");

    static auto vertSPV = readFileBin("sdf_quad.vert.glsl.spv");
    static auto rectSPV = readFileBin("rect.frag.glsl.spv");
    static auto circleSPV = readFileBin("circle.frag.glsl.spv");
    static auto lineSPV = readFileBin("line.frag.glsl.spv");

    // Per-vertex layout: float2 position
    gpu::VertexBufferLayout vertLayout;
    vertLayout.stride = sizeof(float) * 2;
    vertLayout.perInstance = false;
    vertLayout.attributes = {{0, 0, gpu::VertexFormat::Float2}};

    // Per-instance layout: SDFQuadInstance (80 bytes)
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
    };

    auto makeShaderSrc = [](const std::string& msl, const std::vector<uint8_t>& spv) {
        gpu::ShaderSource s;
        s.msl = msl;
        if (!spv.empty()) s.spirv = spv;
        return s;
    };

    auto makePipeline = [&](const std::string& fragMSL, const std::vector<uint8_t>& fragSPV) {
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
    linePipeline_ = makePipeline(lineMSLStr, lineSPV);

    // Glyph pipeline
    static std::string glyphVertMSL = readFileStr("glyph.vert.glsl.metal");
    static std::string glyphFragMSL = readFileStr("glyph.frag.glsl.metal");
    static auto glyphVertSPV = readFileBin("glyph.vert.glsl.spv");
    static auto glyphFragSPV = readFileBin("glyph.frag.glsl.spv");

    gpu::VertexBufferLayout glyphInstLayout;
    glyphInstLayout.stride = sizeof(GlyphInstance);
    glyphInstLayout.perInstance = true;
    glyphInstLayout.attributes = {
        {1, 0,  gpu::VertexFormat::Float4},  // screenRect
        {2, 16, gpu::VertexFormat::Float4},  // uvRect
        {3, 32, gpu::VertexFormat::Float4},  // color
        {4, 48, gpu::VertexFormat::Float2},  // viewport
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
    static std::string pathVertMSL = readFileStr("path.vert.glsl.metal");
    static std::string pathFragMSL = readFileStr("path.frag.glsl.metal");
    static auto pathVertSPV = readFileBin("path.vert.glsl.spv");
    static auto pathFragSPV = readFileBin("path.frag.glsl.spv");

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
    static std::string imageVertMSL = readFileStr("image.vert.glsl.metal");
    static std::string imageFragMSL = readFileStr("image.frag.glsl.metal");
    static auto imageVertSPV = readFileBin("image.vert.glsl.spv");
    static auto imageFragSPV = readFileBin("image.frag.glsl.spv");

    {
        gpu::VertexBufferLayout imgInstLayout;
        imgInstLayout.stride = sizeof(ImageInstance);
        imgInstLayout.perInstance = true;
        imgInstLayout.attributes = {
            {1, 0,  gpu::VertexFormat::Float4},   // screenRect
            {2, 16, gpu::VertexFormat::Float4},   // uvRect
            {3, 32, gpu::VertexFormat::Float4},   // tint
            {4, 48, gpu::VertexFormat::Float2},   // viewport
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

static std::unique_ptr<gpu::Buffer> ensureBuffer(gpu::Device* device,
                                                  std::unique_ptr<gpu::Buffer>& buf,
                                                  size_t& capacity, size_t needed) {
    if (needed == 0) return nullptr;
    size_t bytes = needed * sizeof(SDFQuadInstance);
    if (!buf || capacity < needed) {
        capacity = std::max(needed, size_t(64));
        gpu::BufferDesc desc;
        desc.size = capacity * sizeof(SDFQuadInstance);
        desc.usage = gpu::BufferUsage::Vertex;
        buf = device->createBuffer(desc);
    }
    return nullptr;
}

void GPURendererBackend::execute(const RenderCommandBuffer& buffer) {
    if (viewportWidth_ <= 0 || viewportHeight_ <= 0) return;

    ensurePipelines();

    auto batches = compiler_.compile(buffer, viewportWidth_, viewportHeight_);
    uploadAndDraw(batches);
}

void GPURendererBackend::uploadAndDraw(const CompiledBatches& batches) {
    // Ensure instance buffers
    ensureBuffer(device_, rectInstanceBuffer_, rectBufferCapacity_, batches.rects.size());
    ensureBuffer(device_, circleInstanceBuffer_, circleBufferCapacity_, batches.circles.size());
    ensureBuffer(device_, lineInstanceBuffer_, lineBufferCapacity_, batches.lines.size());

    // Upload instance data
    if (!batches.rects.empty())
        rectInstanceBuffer_->write(batches.rects.data(),
                                   batches.rects.size() * sizeof(SDFQuadInstance));
    if (!batches.circles.empty())
        circleInstanceBuffer_->write(batches.circles.data(),
                                     batches.circles.size() * sizeof(SDFQuadInstance));
    if (!batches.lines.empty())
        lineInstanceBuffer_->write(batches.lines.data(),
                                   batches.lines.size() * sizeof(SDFQuadInstance));

    if (!device_->beginFrame()) return;

    gpu::RenderPassDesc passDesc;
    passDesc.clearColor = batches.clearColor;
    auto* enc = device_->beginRenderPass(passDesc);
    if (!enc) { device_->endFrame(); return; }

    if (batches.hasScissor) {
        enc->setScissorRect(
            static_cast<uint32_t>(batches.scissor.x),
            static_cast<uint32_t>(batches.scissor.y),
            static_cast<uint32_t>(batches.scissor.width),
            static_cast<uint32_t>(batches.scissor.height));
    }

    auto drawBatch = [&](gpu::RenderPipeline* pipeline, gpu::Buffer* instBuf, uint32_t count) {
        if (count == 0) return;
        enc->setPipeline(pipeline);
        enc->setVertexBuffer(0, quadVB_.get());
        enc->setVertexBuffer(1, instBuf);
        enc->draw(6, count);
    };

    drawBatch(rectPipeline_.get(), rectInstanceBuffer_.get(),
              static_cast<uint32_t>(batches.rects.size()));
    drawBatch(circlePipeline_.get(), circleInstanceBuffer_.get(),
              static_cast<uint32_t>(batches.circles.size()));
    drawBatch(linePipeline_.get(), lineInstanceBuffer_.get(),
              static_cast<uint32_t>(batches.lines.size()));

    // Glyph rendering
    if (!batches.glyphs.empty() && glyphAtlas_) {
        glyphAtlas_->uploadIfDirty();

        ensureBuffer(device_, glyphInstanceBuffer_, glyphBufferCapacity_, batches.glyphs.size());
        glyphInstanceBuffer_->write(batches.glyphs.data(),
                                     batches.glyphs.size() * sizeof(GlyphInstance));

        enc->setPipeline(glyphPipeline_.get());
        enc->setVertexBuffer(0, quadVB_.get());
        enc->setVertexBuffer(1, glyphInstanceBuffer_.get());
        if (glyphAtlas_->texture())
            enc->setFragmentTexture(0, glyphAtlas_->texture());
        enc->draw(6, static_cast<uint32_t>(batches.glyphs.size()));
    }

    // Image rendering — one draw per unique texture
    if (!batches.imageDraws.empty()) {
        for (const auto& draw : batches.imageDraws) {
            gpu::Texture* tex = nullptr;
            if (!draw.path.empty())
                tex = imageCache_->getOrLoad(draw.path);
            else if (draw.imageId > 0)
                tex = imageCache_->getById(draw.imageId);
            if (!tex) continue;

            size_t needed = 1;
            if (!imageInstanceBuffer_ || imageBufferCapacity_ < needed) {
                imageBufferCapacity_ = std::max(needed, size_t(16));
                gpu::BufferDesc desc;
                desc.size = imageBufferCapacity_ * sizeof(ImageInstance);
                desc.usage = gpu::BufferUsage::Vertex;
                imageInstanceBuffer_ = device_->createBuffer(desc);
            }
            imageInstanceBuffer_->write(&draw.instance, sizeof(ImageInstance));

            enc->setPipeline(imagePipeline_.get());
            enc->setVertexBuffer(0, quadVB_.get());
            enc->setVertexBuffer(1, imageInstanceBuffer_.get());
            enc->setFragmentTexture(0, tex);
            enc->draw(6, 1);
        }
    }

    // Path rendering (non-instanced triangles)
    if (!batches.pathVertices.empty()) {
        size_t pathCount = batches.pathVertices.size();
        size_t pathBytes = pathCount * sizeof(PathVertex);
        if (!pathVertexBuffer_ || pathBufferCapacity_ < pathCount) {
            pathBufferCapacity_ = std::max(pathCount, size_t(256));
            gpu::BufferDesc desc;
            desc.size = pathBufferCapacity_ * sizeof(PathVertex);
            desc.usage = gpu::BufferUsage::Vertex;
            pathVertexBuffer_ = device_->createBuffer(desc);
        }
        pathVertexBuffer_->write(batches.pathVertices.data(), pathBytes);

        enc->setPipeline(pathPipeline_.get());
        enc->setVertexBuffer(0, pathVertexBuffer_.get());
        enc->draw(static_cast<uint32_t>(pathCount), 1);
    }

    device_->endRenderPass();
    device_->endFrame();
}

} // namespace flux
