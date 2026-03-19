#include <Flux/Platform/NanoVGRenderer.hpp>
#include <Flux/Graphics/NanoVGRenderContext.hpp>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <nanovg_gl.h>
#include <Flux/Core/Log.hpp>
#include <cstring>
#include <vector>

namespace flux {

NanoVGRenderer::NanoVGRenderer()
    : nvgContext_(nullptr), width_(0), height_(0), dpiScaleX_(1.0f), dpiScaleY_(1.0f) {
}

NanoVGRenderer::~NanoVGRenderer() {
    cleanup();
}

bool NanoVGRenderer::initialize(int width, int height, float dpiScaleX, float dpiScaleY) {
    width_ = width;
    height_ = height;
    dpiScaleX_ = dpiScaleX;
    dpiScaleY_ = dpiScaleY;

    nvgContext_ = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES);

    if (!nvgContext_) {
        FLUX_LOG_ERROR("[NanoVGRenderer] Failed to initialize NanoVG");
        return false;
    }

    renderContext_ = std::make_unique<NanoVGRenderContext>(nvgContext_, width_, height_, dpiScaleX_, dpiScaleY_);

    FLUX_LOG_INFO("[NanoVGRenderer] Initialized with size %dx%d DPI scale %fx%f", width_, height_, dpiScaleX_, dpiScaleY_);

    return true;
}

void NanoVGRenderer::cleanup() {
    renderContext_.reset();

    if (nvgContext_) {
        nvgDeleteGL2(nvgContext_);
        nvgContext_ = nullptr;
    }
}

RenderContext* NanoVGRenderer::renderContext() {
    return renderContext_.get();
}

void NanoVGRenderer::beginFrame() {
    if (nvgContext_) {
        nvgBeginFrame(nvgContext_, width_, height_, dpiScaleX_);
    }
}

void NanoVGRenderer::endFrame() {
    if (nvgContext_) {
        nvgEndFrame(nvgContext_);
    }
}

void NanoVGRenderer::resize(int width, int height) {
    width_ = width;
    height_ = height;

    if (renderContext_) {
        renderContext_->resize(width, height);
    }

    FLUX_LOG_DEBUG("[NanoVGRenderer] Resized to %dx%d DPI scale %fx%f", width_, height_, dpiScaleX_, dpiScaleY_);
}

void NanoVGRenderer::updateDPIScale(float dpiScaleX, float dpiScaleY) {
    dpiScaleX_ = dpiScaleX;
    dpiScaleY_ = dpiScaleY;

    FLUX_LOG_DEBUG("[NanoVGRenderer] Updated DPI scale to %fx%f", dpiScaleX_, dpiScaleY_);
}

void NanoVGRenderer::swapBuffers() {
}

bool NanoVGRenderer::readPixels(int x, int y, int w, int h, std::vector<uint8_t>& out) {
    if (w <= 0 || h <= 0) return false;
    out.resize(static_cast<size_t>(w) * h * 4);
    glReadPixels(x, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, out.data());

    // glReadPixels returns bottom-up; flip to top-down
    int stride = w * 4;
    std::vector<uint8_t> row(stride);
    for (int iy = 0; iy < h / 2; iy++) {
        uint8_t* top = out.data() + iy * stride;
        uint8_t* bot = out.data() + (h - 1 - iy) * stride;
        std::memcpy(row.data(), top, stride);
        std::memcpy(top, bot, stride);
        std::memcpy(bot, row.data(), stride);
    }
    return true;
}

} // namespace flux
