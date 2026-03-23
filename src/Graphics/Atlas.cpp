#include <Flux/Graphics/Atlas.hpp>
#include <algorithm>
#include <stdexcept>

namespace flux {

Atlas::Atlas(gpu::Device* device, const AtlasDesc& desc)
    : device_(device)
    , pageWidth_(desc.pageWidth)
    , pageHeight_(desc.pageHeight)
    , maxPages_(desc.maxPages)
    , format_(desc.format)
    , bpp_(gpu::bytesPerPixel(desc.format))
{
    if (desc.format == gpu::PixelFormat::Depth32F) {
        throw std::invalid_argument("Atlas: Depth32F is not supported as atlas backing");
    }
    if (pageWidth_ == 0 || pageHeight_ == 0 || maxPages_ == 0) {
        throw std::invalid_argument("Atlas: page dimensions and maxPages must be non-zero");
    }
    rowStrideBytes_ = pageWidth_ * bpp_;
    addPage();
}

uint8_t Atlas::addPage() {
    Page page;
    page.data.resize(static_cast<size_t>(pageWidth_) * pageHeight_ * bpp_, 0);
    gpu::TextureDesc td;
    td.width = pageWidth_;
    td.height = pageHeight_;
    td.format = format_;
    page.texture = device_->createTexture(td);
    auto idx = static_cast<uint8_t>(pages_.size());
    pages_.push_back(std::move(page));
    return idx;
}

gpu::Texture* Atlas::texture(uint8_t page) const {
    if (page >= pages_.size()) return nullptr;
    return pages_[page].texture.get();
}

uint8_t* Atlas::rowData(uint8_t pageIndex, uint32_t row) {
    if (pageIndex >= pages_.size()) return nullptr;
    return pages_[pageIndex].data.data() + static_cast<size_t>(row) * rowStrideBytes_;
}

std::optional<Atlas::AllocResult> Atlas::allocate(uint32_t w, uint32_t h, uint32_t pad) {
    if (w == 0 || h == 0) return std::nullopt;

    uint8_t pageIdx = static_cast<uint8_t>(pages_.size() - 1);
    Page* page = &pages_[pageIdx];

    if (page->cursorX + w + pad > pageWidth_) {
        page->cursorX = 0;
        page->cursorY += page->rowHeight + pad;
        page->rowHeight = 0;
    }
    if (page->cursorY + h + pad > pageHeight_) {
        if (pages_.size() >= maxPages_) return std::nullopt;
        pageIdx = addPage();
        page = &pages_[pageIdx];
    }

    const uint32_t ox = page->cursorX;
    const uint32_t oy = page->cursorY;

    expandDirtyRect(pageIdx, ox, oy, w, h);

    const float invW = 1.0f / static_cast<float>(pageWidth_);
    const float invH = 1.0f / static_cast<float>(pageHeight_);

    AllocResult out{};
    out.pageIndex = pageIdx;
    out.x = ox;
    out.y = oy;
    out.u0 = static_cast<float>(ox) * invW;
    out.v0 = static_cast<float>(oy) * invH;
    out.u1 = static_cast<float>(ox + w) * invW;
    out.v1 = static_cast<float>(oy + h) * invH;

    page->cursorX += w + pad;
    page->rowHeight = std::max(page->rowHeight, h);

    return out;
}

bool Atlas::dirty() const {
    for (const auto& p : pages_) {
        if (p.dirty) return true;
    }
    return false;
}

void Atlas::uploadIfDirty() {
    lastGpuUploadBytes_ = 0;
    for (auto& page : pages_) {
        if (!page.dirty || !page.texture) continue;

        if (!page.dirtyRectValid) {
            page.texture->write(page.data.data(), 0, 0, pageWidth_, pageHeight_, rowStrideBytes_);
            lastGpuUploadBytes_ += static_cast<uint64_t>(pageWidth_) * pageHeight_ * bpp_;
        } else {
            const uint32_t w = page.dirtyX1 - page.dirtyX0;
            const uint32_t h = page.dirtyY1 - page.dirtyY0;
            if (w > 0 && h > 0) {
                const uint8_t* src =
                    &page.data[static_cast<size_t>(page.dirtyY0) * rowStrideBytes_
                               + static_cast<size_t>(page.dirtyX0) * bpp_];
                page.texture->write(src, page.dirtyX0, page.dirtyY0, w, h, rowStrideBytes_);
                lastGpuUploadBytes_ += static_cast<uint64_t>(w) * h * bpp_;
            }
        }
        page.dirty = false;
        page.dirtyRectValid = false;
    }
}

void Atlas::expandDirtyRect(uint8_t pageIndex, uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    if (pageIndex >= pages_.size() || w == 0 || h == 0) return;
    auto& page = pages_[pageIndex];
    const uint32_t x1 = x + w;
    const uint32_t y1 = y + h;
    if (!page.dirtyRectValid) {
        page.dirtyX0 = x;
        page.dirtyY0 = y;
        page.dirtyX1 = x1;
        page.dirtyY1 = y1;
        page.dirtyRectValid = true;
    } else {
        page.dirtyX0 = std::min(page.dirtyX0, x);
        page.dirtyY0 = std::min(page.dirtyY0, y);
        page.dirtyX1 = std::max(page.dirtyX1, x1);
        page.dirtyY1 = std::max(page.dirtyY1, y1);
    }
    page.dirty = true;
}

void Atlas::markAllPagesDirty() {
    for (auto& page : pages_) {
        page.dirty = true;
        page.dirtyRectValid = true;
        page.dirtyX0 = 0;
        page.dirtyY0 = 0;
        page.dirtyX1 = pageWidth_;
        page.dirtyY1 = pageHeight_;
    }
}

} // namespace flux
