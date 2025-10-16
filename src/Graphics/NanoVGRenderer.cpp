#include <Flux/Platform/NanoVGRenderer.hpp>
#include <Flux/Graphics/NanoVGRenderContext.hpp>
#define NANOVG_GL2_IMPLEMENTATION
#include <nanovg_gl.h>
#include <iostream>
#include <stdexcept>

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

    // Initialize NanoVG with OpenGL backend
    nvgContext_ = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
    if (!nvgContext_) {
        std::cerr << "[NanoVGRenderer] Failed to initialize NanoVG\n";
        return false;
    }

    // Create render context
    renderContext_ = std::make_unique<NanoVGRenderContext>(nvgContext_, width_, height_, dpiScaleX_, dpiScaleY_);

    std::cout << "[NanoVGRenderer] Initialized with size " << width_ << "x" << height_
              << " DPI scale " << dpiScaleX_ << "x" << dpiScaleY_ << "\n";

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

    std::cout << "[NanoVGRenderer] Resized to " << width_ << "x" << height_
              << " DPI scale " << dpiScaleX_ << "x" << dpiScaleY_ << "\n";
}

void NanoVGRenderer::updateDPIScale(float dpiScaleX, float dpiScaleY) {
    dpiScaleX_ = dpiScaleX;
    dpiScaleY_ = dpiScaleY;

    std::cout << "[NanoVGRenderer] Updated DPI scale to " << dpiScaleX_ << "x" << dpiScaleY_ << "\n";
}

void NanoVGRenderer::swapBuffers() {
    // Buffer swapping is handled by GLFW
}

} // namespace flux
