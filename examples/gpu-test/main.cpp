#include <Flux/GPU/Device.hpp>
#include <SDL3/SDL.h>
#include <cstdio>
#include <cmath>
#include <cstring>
#include <fstream>
#include <vector>

static constexpr const char* kTriangleVertexMSL = R"(
#include <metal_stdlib>
using namespace metal;
struct VertexOut { float4 position [[position]]; float4 color; };
vertex VertexOut vertexMain(uint vid [[vertex_id]]) {
    constexpr float2 positions[3] = { float2(0,-0.5), float2(-0.5,0.5), float2(0.5,0.5) };
    constexpr float4 colors[3] = { float4(1,0.2,0.2,1), float4(0.2,1,0.2,1), float4(0.2,0.2,1,1) };
    VertexOut out; out.position = float4(positions[vid], 0, 1); out.color = colors[vid]; return out;
}
)";
static constexpr const char* kTriangleFragmentMSL = R"(
#include <metal_stdlib>
using namespace metal;
struct VertexOut { float4 position [[position]]; float4 color; };
fragment float4 fragmentMain(VertexOut in [[stage_in]]) { return in.color; }
)";

static std::vector<uint8_t> readFile(const char* path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) return {};
    auto sz = f.tellg();
    std::vector<uint8_t> buf(static_cast<size_t>(sz));
    f.seekg(0);
    f.read(reinterpret_cast<char*>(buf.data()), sz);
    return buf;
}

int main(int argc, char* argv[]) {
    bool useVulkan = false;
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "--vulkan") == 0) useVulkan = true;
    }

    auto backend = useVulkan ? flux::gpu::Backend::Vulkan : flux::gpu::Backend::Metal;
    const char* backendName = useVulkan ? "Vulkan" : "Metal";

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    Uint64 flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY;
    if (useVulkan) flags |= SDL_WINDOW_VULKAN;
    else           flags |= SDL_WINDOW_METAL;

    char title[64];
    std::snprintf(title, sizeof(title), "Flux GPU Test - %s", backendName);

    SDL_Window* window = SDL_CreateWindow(title, 800, 600, flags);
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    try {
        auto device = flux::gpu::createDevice(backend, window);
        std::fprintf(stderr, "%s device created\n", backendName);

        flux::gpu::ShaderSource vertSrc, fragSrc;
        vertSrc.msl = kTriangleVertexMSL;
        fragSrc.msl = kTriangleFragmentMSL;

        auto vertSpv = readFile("generated/shaders/test_triangle.vert.glsl.spv");
        auto fragSpv = readFile("generated/shaders/test_triangle.frag.glsl.spv");
        if (!vertSpv.empty()) vertSrc.spirv = vertSpv;
        if (!fragSpv.empty()) fragSrc.spirv = fragSpv;

        flux::gpu::RenderPipelineDesc pipeDesc;
        pipeDesc.vertexShader = vertSrc;
        pipeDesc.fragmentShader = fragSrc;
        pipeDesc.vertexFunction = "vertexMain";
        pipeDesc.fragmentFunction = "fragmentMain";
        pipeDesc.colorFormat = device->swapchainFormat();
        pipeDesc.blendEnabled = false;

        auto pipeline = device->createRenderPipeline(pipeDesc);
        std::fprintf(stderr, "Pipeline created\n");

        bool running = true;
        uint32_t frameCount = 0;

        while (running) {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_EVENT_QUIT) running = false;
                if (event.type == SDL_EVENT_WINDOW_RESIZED) {
                    int w, h;
                    SDL_GetWindowSizeInPixels(window, &w, &h);
                    device->resize(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
                }
            }

            float t = static_cast<float>(frameCount) * 0.01f;
            float r = 0.05f + 0.05f * std::sin(t);
            float g = 0.05f + 0.05f * std::sin(t + 2.0f);
            float b = 0.15f + 0.10f * std::sin(t + 4.0f);

            if (device->beginFrame()) {
                flux::gpu::RenderPassDesc passDesc;
                passDesc.clearColor = {r, g, b, 1.0f};
                auto* encoder = device->beginRenderPass(passDesc);
                if (encoder) {
                    encoder->setPipeline(pipeline.get());
                    encoder->draw(3);
                }
                device->endRenderPass();
                device->endFrame();
            }
            frameCount++;
        }
        std::fprintf(stderr, "Rendered %u frames\n", frameCount);

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
