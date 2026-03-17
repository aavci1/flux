#pragma once

#include <Flux/Core/Window.hpp>
#include <Flux/Platform/PlatformWindow.hpp>
#include <vector>
#include <mutex>
#include <cstring>
#include <OpenGL/gl.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace llm_studio {

class ScreenCapture {
public:
    void capture(flux::Window& window) {
        auto* pw = static_cast<flux::PlatformWindow*>(window.platformWindow());
        if (!pw) return;

        float dpiX = pw->dpiScaleX();
        float dpiY = pw->dpiScaleY();
        flux::Size logicalSize = window.getSize();

        int w = static_cast<int>(logicalSize.width * dpiX);
        int h = static_cast<int>(logicalSize.height * dpiY);
        if (w <= 0 || h <= 0) return;

        std::vector<uint8_t> pixels(w * h * 4);
        glReadPixels(0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

        // Flip vertically (OpenGL origin is bottom-left)
        int stride = w * 4;
        std::vector<uint8_t> row(stride);
        for (int y = 0; y < h / 2; y++) {
            uint8_t* top = pixels.data() + y * stride;
            uint8_t* bot = pixels.data() + (h - 1 - y) * stride;
            std::memcpy(row.data(), top, stride);
            std::memcpy(top, bot, stride);
            std::memcpy(bot, row.data(), stride);
        }

        // Set full alpha
        for (int i = 3; i < w * h * 4; i += 4) {
            pixels[i] = 255;
        }

        // Encode to PNG in memory
        std::vector<uint8_t> pngBuf;
        stbi_write_png_to_func(
            [](void* ctx, void* data, int size) {
                auto* buf = static_cast<std::vector<uint8_t>*>(ctx);
                auto* bytes = static_cast<uint8_t*>(data);
                buf->insert(buf->end(), bytes, bytes + size);
            },
            &pngBuf, w, h, 4, pixels.data(), stride
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

} // namespace llm_studio
