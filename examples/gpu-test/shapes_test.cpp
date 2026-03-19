#include <Flux/Graphics/GPURendererBackend.hpp>
#include <Flux/Graphics/RenderCommandBuffer.hpp>
#include <Flux/GPU/Device.hpp>
#include <SDL3/SDL.h>
#include <cstdio>
#include <cstring>
#include <cmath>

int main(int argc, char* argv[]) {
    bool useVulkan = false;
    for (int i = 1; i < argc; i++)
        if (std::strcmp(argv[i], "--vulkan") == 0) useVulkan = true;

    auto backend = useVulkan ? flux::gpu::Backend::Vulkan : flux::gpu::Backend::Metal;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    Uint64 flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    flags |= useVulkan ? SDL_WINDOW_VULKAN : SDL_WINDOW_METAL;

    SDL_Window* window = SDL_CreateWindow("SDF Shapes Test", 800, 600, flags);
    if (!window) { std::fprintf(stderr, "Window failed\n"); return 1; }

    try {
        auto device = flux::gpu::createDevice(backend, window);
        flux::GPURendererBackend gpuBackend(device.get());

        // Load a system font for text rendering
        auto* atlas = gpuBackend.glyphAtlas();
        if (atlas) {
            if (!atlas->loadFontByName("Helvetica", flux::FontWeight::regular, 0)) {
                atlas->loadFontByName("Arial", flux::FontWeight::regular, 0);
            }
        }

        int pw, ph;
        SDL_GetWindowSizeInPixels(window, &pw, &ph);
        gpuBackend.setViewportSize(static_cast<float>(pw), static_cast<float>(ph));

        bool running = true;
        uint32_t frame = 0;

        while (running) {
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_EVENT_QUIT) running = false;
                if (e.type == SDL_EVENT_WINDOW_RESIZED) {
                    SDL_GetWindowSizeInPixels(window, &pw, &ph);
                    device->resize(static_cast<uint32_t>(pw), static_cast<uint32_t>(ph));
                    gpuBackend.setViewportSize(static_cast<float>(pw), static_cast<float>(ph));
                }
            }

            float t = frame * 0.02f;

            flux::RenderCommandBuffer buf;
            buf.push(flux::CmdClear{{0.12f, 0.12f, 0.18f, 1.0f}});

            // Blue rounded rectangle
            buf.push(flux::CmdSetFillStyle{flux::FillStyle::solid({0.2f, 0.4f, 0.9f, 1.0f})});
            buf.push(flux::CmdSetStrokeStyle{flux::StrokeStyle::none()});
            buf.push(flux::CmdDrawRect{{50, 50, 200, 120}, flux::CornerRadius(12)});

            // Red stroked rectangle
            buf.push(flux::CmdSetFillStyle{flux::FillStyle::solid({0.9f, 0.2f, 0.3f, 0.8f})});
            buf.push(flux::CmdSetStrokeStyle{flux::StrokeStyle::solid({1, 1, 1, 1}, 2)});
            buf.push(flux::CmdDrawRect{{300, 50, 180, 120}, flux::CornerRadius(20)});

            // Animated circle
            float cx = 400 + std::sin(t) * 100;
            float cy = 350 + std::cos(t * 0.7f) * 80;
            buf.push(flux::CmdSetFillStyle{flux::FillStyle::solid({0.2f, 0.9f, 0.4f, 0.9f})});
            buf.push(flux::CmdSetStrokeStyle{flux::StrokeStyle::solid({1, 1, 0, 1}, 3)});
            buf.push(flux::CmdDrawCircle{{cx, cy}, 60});

            // Small circles
            for (int i = 0; i < 12; i++) {
                float a = static_cast<float>(i) * 0.524f + t;
                float px = 150 + std::cos(a) * 80;
                float py = 350 + std::sin(a) * 80;
                float hue = static_cast<float>(i) / 12.0f;
                buf.push(flux::CmdSetFillStyle{flux::FillStyle::solid({hue, 1.0f - hue, 0.5f, 0.8f})});
                buf.push(flux::CmdSetStrokeStyle{flux::StrokeStyle::none()});
                buf.push(flux::CmdDrawCircle{{px, py}, 15});
            }

            // Lines
            for (int i = 0; i < 8; i++) {
                float a = static_cast<float>(i) * 0.785f + t * 0.5f;
                float x1 = 650 + std::cos(a) * 30;
                float y1 = 150 + std::sin(a) * 30;
                float x2 = 650 + std::cos(a) * 90;
                float y2 = 150 + std::sin(a) * 90;
                buf.push(flux::CmdSetStrokeStyle{flux::StrokeStyle::solid({1, 0.8f, 0.2f, 0.9f}, 2)});
                buf.push(flux::CmdDrawLine{{x1, y1}, {x2, y2}});
            }

            // Text rendering
            buf.push(flux::CmdSetTextStyle{flux::TextStyle::regular("default", 24)});
            buf.push(flux::CmdSetFillStyle{flux::FillStyle::solid({1, 1, 1, 1})});
            buf.push(flux::CmdDrawText{"Hello GPU Renderer!", {400, 30}, flux::HorizontalAlignment::center, flux::VerticalAlignment::center});

            buf.push(flux::CmdSetTextStyle{flux::TextStyle::regular("default", 16)});
            buf.push(flux::CmdSetFillStyle{flux::FillStyle::solid({0.8f, 0.8f, 0.9f, 1})});
            buf.push(flux::CmdDrawText{"SDF shapes + FreeType glyphs + instanced rendering", {400, 560}, flux::HorizontalAlignment::center, flux::VerticalAlignment::center});

            buf.push(flux::CmdSetFillStyle{flux::FillStyle::solid({1, 0.9f, 0.3f, 1})});
            buf.push(flux::CmdDrawTextBox{"This text wraps inside a box using the FreeType glyph atlas.", {520, 250}, 250, flux::HorizontalAlignment::leading});

            // Path rendering: a star shape
            {
                flux::Path star;
                float starCx = 650, starCy = 350;
                for (int i = 0; i < 10; i++) {
                    float a = static_cast<float>(i) * 0.6283185f - 1.5707963f;
                    float r = (i % 2 == 0) ? 50.0f : 25.0f;
                    float px = starCx + std::cos(a) * r;
                    float py = starCy + std::sin(a) * r;
                    if (i == 0) star.moveTo({px, py});
                    else star.lineTo({px, py});
                }
                star.close();
                buf.push(flux::CmdSetFillStyle{flux::FillStyle::solid({1.0f, 0.85f, 0.0f, 0.9f})});
                buf.push(flux::CmdSetStrokeStyle{flux::StrokeStyle::solid({1, 1, 1, 1}, 2)});
                buf.push(flux::CmdDrawPath{star});
            }

            // Many rectangles for batching test
            buf.push(flux::CmdSetStrokeStyle{flux::StrokeStyle::none()});
            for (int i = 0; i < 20; i++) {
                float x = 50 + static_cast<float>(i) * 35;
                float alpha = 0.3f + 0.7f * static_cast<float>(i) / 20.0f;
                buf.push(flux::CmdSetFillStyle{flux::FillStyle::solid({0.6f, 0.3f, 0.8f, alpha})});
                buf.push(flux::CmdDrawRect{{x, 500, 30, 30}, flux::CornerRadius(4)});
            }

            gpuBackend.execute(buf);
            frame++;
        }

        std::fprintf(stderr, "Rendered %u frames\n", frame);
    } catch (const std::exception& ex) {
        std::fprintf(stderr, "Error: %s\n", ex.what());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
