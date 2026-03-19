#pragma once

#include <Flux/Core/Window.hpp>
#include <Flux/Platform/PlatformWindow.hpp>
#include <Flux/Platform/PlatformRenderer.hpp>
#include <vector>
#include <mutex>
#include <cstring>
#include "stb_image_write.h"

namespace flux {

class ScreenCapture {
public:
    void capture(Window& window) {
        auto* pw = static_cast<PlatformWindow*>(window.platformWindow());
        if (!pw) return;

        float dpiX = pw->dpiScaleX();
        float dpiY = pw->dpiScaleY();
        Size logicalSize = window.getSize();

        int w = static_cast<int>(logicalSize.width * dpiX);
        int h = static_cast<int>(logicalSize.height * dpiY);
        if (w <= 0 || h <= 0) return;

        auto* renderer = pw->platformRenderer();
        if (!renderer) return;

        std::vector<uint8_t> pixels;
        if (!renderer->readPixels(0, 0, w, h, pixels)) return;

        // Force alpha to 255 for screenshot output
        for (size_t i = 3; i < pixels.size(); i += 4) {
            pixels[i] = 255;
        }

        std::vector<uint8_t> pngBuf;
        stbi_write_png_to_func(
            [](void* ctx, void* data, int size) {
                auto* buf = static_cast<std::vector<uint8_t>*>(ctx);
                auto* bytes = static_cast<uint8_t*>(data);
                buf->insert(buf->end(), bytes, bytes + size);
            },
            &pngBuf, w, h, 4, pixels.data(), w * 4
        );

        std::lock_guard<std::mutex> lock(mutex_);
        pngData_ = std::move(pngBuf);
        width_ = w;
        height_ = h;
    }

    std::vector<uint8_t> getPng() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return pngData_;
    }

    int width() const { std::lock_guard<std::mutex> lock(mutex_); return width_; }
    int height() const { std::lock_guard<std::mutex> lock(mutex_); return height_; }

private:
    mutable std::mutex mutex_;
    std::vector<uint8_t> pngData_;
    int width_ = 0;
    int height_ = 0;
};

} // namespace flux
