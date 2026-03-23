#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace flux::gpu {

enum class Backend { Metal, Vulkan };

enum class PixelFormat {
    RGBA8,
    BGRA8,
    R8,
    Depth32F
};

/// Bytes per texel for formats that can be staged to a texture (not meaningful for Depth32F as color data).
[[nodiscard]] constexpr uint32_t bytesPerPixel(PixelFormat fmt) noexcept {
    switch (fmt) {
    case PixelFormat::RGBA8: return 4;
    case PixelFormat::BGRA8: return 4;
    case PixelFormat::R8: return 1;
    case PixelFormat::Depth32F: return 4;
    }
    return 0;
}

enum class BufferUsage {
    Vertex,
    Index,
    Uniform
};

enum class LoadAction {
    Clear,
    Load,
    DontCare
};

enum class VertexFormat {
    Float,
    Float2,
    Float3,
    Float4,
    UByte4Norm
};

struct ClearColor {
    float r = 0.0f, g = 0.0f, b = 0.0f, a = 1.0f;
};

struct VertexAttribute {
    uint32_t location;
    uint32_t offset;
    VertexFormat format;
};

struct VertexBufferLayout {
    uint32_t stride;
    bool perInstance = false;
    std::vector<VertexAttribute> attributes;
};

struct BufferDesc {
    size_t size;
    BufferUsage usage;
};

struct TextureDesc {
    uint32_t width;
    uint32_t height;
    PixelFormat format;
    bool renderTarget = false;
};

struct ShaderSource {
    std::span<const uint8_t> spirv;
    std::string_view msl;
};

struct RenderPipelineDesc {
    ShaderSource vertexShader;
    ShaderSource fragmentShader;
    std::string vertexFunction = "vertexMain";
    std::string fragmentFunction = "fragmentMain";
    std::vector<VertexBufferLayout> vertexBuffers;
    PixelFormat colorFormat = PixelFormat::BGRA8;
    bool blendEnabled = true;
};

} // namespace flux::gpu
