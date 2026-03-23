#pragma once

#include <Flux/GPU/Device.hpp>
#include <Flux/GPU/Types.hpp>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

namespace flux {

struct AtlasDesc {
    uint32_t pageWidth = 1024;
    uint32_t pageHeight = 1024;
    uint32_t maxPages = 8;
    gpu::PixelFormat format = gpu::PixelFormat::R8;
};

/// Shelf-packed GPU texture atlas with optional CPU staging (batched uploads via dirty rects).
class Atlas {
public:
    Atlas(gpu::Device* device, const AtlasDesc& desc);

    [[nodiscard]] uint32_t pageWidth() const { return pageWidth_; }
    [[nodiscard]] uint32_t pageHeight() const { return pageHeight_; }
    [[nodiscard]] gpu::PixelFormat format() const { return format_; }
    [[nodiscard]] uint32_t rowStrideBytes() const { return rowStrideBytes_; }

    [[nodiscard]] uint8_t pageCount() const { return static_cast<uint8_t>(pages_.size()); }
    [[nodiscard]] uint32_t maxPages() const { return maxPages_; }

    [[nodiscard]] gpu::Texture* texture(uint8_t page = 0) const;

    /// UVs are normalized to [0,1] using page width/height (non-square pages use independent scales).
    struct AllocResult {
        uint8_t pageIndex = 0;
        uint32_t x = 0;
        uint32_t y = 0;
        float u0 = 0;
        float v0 = 0;
        float u1 = 0;
        float v1 = 0;
    };

    /// Reserve a w×h rectangle on the current shelf; may add pages up to maxPages.
    [[nodiscard]] std::optional<AllocResult> allocate(uint32_t w, uint32_t h, uint32_t pad = 1);

    /// Pointer to the first byte of a row in the CPU staging buffer (tightly packed, rowStrideBytes() pitch).
    [[nodiscard]] uint8_t* rowData(uint8_t pageIndex, uint32_t row);

    void expandDirtyRect(uint8_t pageIndex, uint32_t x, uint32_t y, uint32_t w, uint32_t h);

    [[nodiscard]] bool dirty() const;
    void uploadIfDirty();

    void markAllPagesDirty();

    [[nodiscard]] uint64_t lastGpuUploadBytes() const { return lastGpuUploadBytes_; }

private:
    struct Page {
        std::vector<uint8_t> data;
        std::unique_ptr<gpu::Texture> texture;
        uint32_t cursorX = 0;
        uint32_t cursorY = 0;
        uint32_t rowHeight = 0;
        bool dirty = false;
        bool dirtyRectValid = false;
        uint32_t dirtyX0 = 0;
        uint32_t dirtyY0 = 0;
        uint32_t dirtyX1 = 0;
        uint32_t dirtyY1 = 0;
    };

    gpu::Device* device_;
    uint32_t pageWidth_;
    uint32_t pageHeight_;
    uint32_t maxPages_;
    gpu::PixelFormat format_;
    uint32_t bpp_;
    uint32_t rowStrideBytes_;

    std::vector<Page> pages_;
    uint64_t lastGpuUploadBytes_ = 0;

    uint8_t addPage();
};

} // namespace flux
