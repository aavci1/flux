#pragma once

#include <Flux/Graphics/RenderCommandBuffer.hpp>
#include <unordered_map>
#include <list>
#include <string>

struct NVGcontext;

namespace flux {

class NanoVGBackend : public RenderBackend {
public:
    explicit NanoVGBackend(NVGcontext* ctx) : nvg_(ctx) {}

    void execute(const RenderCommandBuffer& buffer) override;

    std::unordered_map<std::string, int>& fontCache() { return fontCache_; }

private:
    NVGcontext* nvg_;
    std::unordered_map<std::string, int> fontCache_;

    static constexpr size_t kMaxImageCacheEntries = 256;
    struct ImageEntry { std::string path; int nvgId; };
    std::list<ImageEntry> imageLru_;
    std::unordered_map<std::string, std::list<ImageEntry>::iterator> imageIndex_;

    int getOrCreateImage(const std::string& path);

    FillStyle currentFill_;
    StrokeStyle currentStroke_;
    TextStyle currentText_;

    void applyFill();
    void applyStroke();
    int resolveFont(const std::string& name, FontWeight weight);

    void drawPath(const Path& path);
    void drawImage(int imageId, const Rect& rect, ImageFit fit,
                   const CornerRadius& cr, float alpha);
};

} // namespace flux
