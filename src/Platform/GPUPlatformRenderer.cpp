#include <Flux/Platform/GPUPlatformRenderer.hpp>
#include <Flux/Platform/MemoryFootprint.hpp>
#include <Flux/Core/Log.hpp>

namespace flux {

GPUPlatformRenderer::GPUPlatformRenderer(gpu::Backend backend)
    : backend_(backend) {}

GPUPlatformRenderer::~GPUPlatformRenderer() {
    cleanup();
}

bool GPUPlatformRenderer::initialize(int width, int height, float dpiScaleX, float dpiScaleY) {
    width_ = width;
    height_ = height;
    dpiScaleX_ = dpiScaleX;
    dpiScaleY_ = dpiScaleY;

    if (surface_.kind == gpu::NativeGraphicsSurfaceKind::None || !surface_.ptr) {
        FLUX_LOG_ERROR("[GPUPlatformRenderer] graphics surface not set before initialize");
        return false;
    }

    try {
        device_ = gpu::createDevice(backend_, surface_);
        gpuBackend_ = std::make_unique<GPURendererBackend>(device_.get());

        int pw = static_cast<int>(width * dpiScaleX);
        int ph = static_cast<int>(height * dpiScaleY);
        gpuBackend_->setViewportSize(static_cast<float>(pw), static_cast<float>(ph));
        gpuBackend_->setDPIScale(dpiScaleX, dpiScaleY);

        auto* atlas = gpuBackend_->glyphAtlas();
        auto* imgCache = gpuBackend_->imageCache();

        renderCtx_ = std::make_unique<GPURenderContext>(atlas, imgCache, gpuBackend_.get(),
                                                        width, height, dpiScaleX, dpiScaleY);

        if (atlas) {
            if (!atlas->ensureFontLoaded("Helvetica", FontWeight::regular)) {
                if (!atlas->ensureFontLoaded("Arial", FontWeight::regular)) {
                    (void)atlas->ensureFontLoaded("DejaVu Sans", FontWeight::regular);
                }
            }
        }

        FLUX_LOG_INFO("[GPUPlatformRenderer] Initialized %s backend %dx%d (fb %dx%d)",
                      backend_ == gpu::Backend::Metal ? "Metal" : "Vulkan",
                      width, height, pw, ph);
        logMemoryFootprintIfRequested("after GPU init");
        return true;
    } catch (const std::exception& ex) {
        FLUX_LOG_ERROR("[GPUPlatformRenderer] Init failed: %s", ex.what());
        return false;
    }
}

void GPUPlatformRenderer::cleanup() {
    renderCtx_.reset();
    gpuBackend_.reset();
    device_.reset();
}

RenderContext* GPUPlatformRenderer::renderContext() {
    return renderCtx_.get();
}

void GPUPlatformRenderer::beginFrame() {
    if (renderCtx_) renderCtx_->beginFrame();
}

void GPUPlatformRenderer::endFrame() {
    // present() on GPURenderContext feeds the buffer to GPURendererBackend
    if (renderCtx_) renderCtx_->present();
}

void GPUPlatformRenderer::resize(int width, int height) {
    width_ = width;
    height_ = height;
    int pw = static_cast<int>(width * dpiScaleX_);
    int ph = static_cast<int>(height * dpiScaleY_);
    if (device_) device_->resize(static_cast<uint32_t>(pw), static_cast<uint32_t>(ph));
    if (gpuBackend_) {
        gpuBackend_->setViewportSize(static_cast<float>(pw), static_cast<float>(ph));
        gpuBackend_->setDPIScale(dpiScaleX_, dpiScaleY_);
    }
    if (renderCtx_) renderCtx_->resize(width, height);
}

void GPUPlatformRenderer::updateDPIScale(float dpiScaleX, float dpiScaleY) {
    dpiScaleX_ = dpiScaleX;
    dpiScaleY_ = dpiScaleY;
    if (renderCtx_) renderCtx_->updateDPIScale(dpiScaleX, dpiScaleY);
    resize(width_, height_);
}

void GPUPlatformRenderer::swapBuffers() {
}

bool GPUPlatformRenderer::readPixels(int x, int y, int w, int h, std::vector<uint8_t>& out) {
    if (!device_) return false;
    return device_->readPixels(x, y, w, h, out);
}

void GPUPlatformRenderer::setReadbackEnabled(bool enabled) {
    if (device_) {
        device_->setReadbackEnabled(enabled);
    }
}

} // namespace flux
