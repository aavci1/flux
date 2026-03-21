#pragma once

#include <Flux/Graphics/RenderCommandBuffer.hpp>
#include <unordered_map>
#include <string>

struct NVGcontext;

namespace flux {

class NanoVGBackend : public RenderBackend {
public:
    explicit NanoVGBackend(NVGcontext* ctx) : nvg_(ctx) {}

    void execute(const RenderCommandBuffer& buffer) override;

    std::unordered_map<std::string, int>& fontCache() { return fontCache_; }
    std::unordered_map<std::string, int>& imageCache() { return imageCache_; }

private:
    NVGcontext* nvg_;
    std::unordered_map<std::string, int> fontCache_;
    std::unordered_map<std::string, int> imageCache_;

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
