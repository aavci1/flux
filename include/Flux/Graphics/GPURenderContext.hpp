#pragma once

#include <Flux/Graphics/RenderContext.hpp>
#include <Flux/Graphics/RenderCommandBuffer.hpp>
#include <Flux/Graphics/ImageCache.hpp>
#include <string>
#include <unordered_map>
#include <memory>

namespace flux {

class FontProvider;
class GPURendererBackend;

class GPURenderContext : public RenderContext {
public:
    GPURenderContext(FontProvider* fontProvider, ImageCache* imageCache,
                     GPURendererBackend* backend,
                     int width, int height,
                     float dpiScaleX = 1.0f, float dpiScaleY = 1.0f);

    void setCommandBuffer(RenderCommandBuffer* buf) { cmdBuf_ = buf; }
    RenderCommandBuffer* commandBuffer() const { return cmdBuf_; }

    void beginFrame() override;
    void clear(const Color& color) override;
    void present() override;
    void resize(int width, int height) override;
    void updateDPIScale(float dpiScaleX, float dpiScaleY);

    void save() override;
    void restore() override;
    void reset() override;

    void translate(float x, float y) override;
    void rotate(float angle) override;
    void scale(float sx, float sy) override;
    void skewX(float angle) override;
    void skewY(float angle) override;
    void setTransform(float a, float b, float c, float d, float e, float f) override;
    void resetTransform() override;
    void getTransform(float* matrix) override;

    void setCompositeOperation(CompositeOperation op) override;
    void setOpacity(float alpha) override;
    void setShapeAntiAlias(bool enabled) override;

    void setStrokeColor(const Color& color) override;
    void setStrokeWidth(float width) override;
    void setLineCap(LineCap cap) override;
    void setLineJoin(LineJoin join) override;
    void setMiterLimit(float limit) override;
    void setDashPattern(const std::vector<float>& pattern, float offset = 0.0f) override;
    void setStrokeStyle(const StrokeStyle& style) override;

    void setFillColor(const Color& color) override;
    void setPathWinding(PathWinding winding) override;
    void setFillStyle(const FillStyle& style) override;

    void drawPath(const Path& path) override;
    void drawCircle(const Point& center, float radius) override;
    void drawLine(const Point& start, const Point& end) override;
    void drawRect(const Rect& rect, const CornerRadius& cornerRadius = CornerRadius()) override;
    void drawEllipse(const Point& center, float radiusX, float radiusY) override;
    void drawArc(const Point& center, float radius, float startAngle, float endAngle, bool clockwise = false) override;

    void setFont(const std::string& name, FontWeight weight = FontWeight::regular) override;
    void setFontSize(float size) override;
    void setFontBlur(float blur) override;
    void setLetterSpacing(float spacing) override;
    void setLineHeight(float height) override;
    void setTextStyle(const TextStyle& style) override;

    void drawText(const std::string& text, const Point& position, HorizontalAlignment hAlign, VerticalAlignment vAlign) override;
    void drawTextBox(const std::string& text, const Point& position, float maxWidth, HorizontalAlignment hAlign) override;
    Size measureText(const std::string& text, const TextStyle& style) override;
    Size measureTextBox(const std::string& text, const TextStyle& style, float maxWidth) override;
    Rect getTextBounds(const std::string& text, const Point& position, const TextStyle& style) override;

    int createImage(const std::string& filename) override;
    int createImageMem(const unsigned char* data, int dataSize) override;
    int createImageRGBA(int width, int height, const unsigned char* data) override;
    void updateImage(int imageId, const unsigned char* data) override;
    Size getImageSize(int imageId) override;
    void deleteImage(int imageId) override;

    void drawImage(int imageId, const Rect& rect, ImageFit fit = ImageFit::Fill,
                   const CornerRadius& cornerRadius = CornerRadius(), float alpha = 1.0f) override;
    void drawImage(const std::string& path, const Rect& rect, ImageFit fit = ImageFit::Fill,
                   const CornerRadius& cornerRadius = CornerRadius(), float alpha = 1.0f) override;

    void clipPath(const Path& path) override;
    void resetClip() override;

    Point transformPoint(const Point& point) override;
    Rect transformRect(const Rect& rect) override;
    float degToRad(float degrees) override;
    float radToDeg(float radians) override;

    void setCurrentFocusKey(const std::string& focusKey) override;
    std::string getFocusedKey() const override;
    bool isCurrentViewFocused() const override;

    void setHoveredBounds(const Rect& bounds) override;
    void setCurrentViewGlobalBounds(const Rect& bounds) override;
    bool isCurrentViewHovered() const override;
    void setPressedBounds(const Rect& bounds) override;
    void clearPressedBounds() override;
    bool isCurrentViewPressed() const override;

private:
    FontProvider* fontProvider_;
    ImageCache* imageCache_;
    GPURendererBackend* gpuBackend_;
    RenderCommandBuffer* cmdBuf_ = nullptr;
    RenderCommandBuffer ownedBuffer_;
    int width_, height_;
    float dpiScaleX_, dpiScaleY_;

    FillStyle currentFill_;
    StrokeStyle currentStroke_;
    TextStyle currentTextStyle_;

    struct TransformState {
        float tx = 0, ty = 0;
        float sx = 1, sy = 1;
        float matrix[6] = {1, 0, 0, 1, 0, 0};
    };
    TransformState transform_;
    std::vector<TransformState> transformStack_;

    struct TextMeasureKey {
        std::string text;
        std::string font;
        float size;
        bool operator==(const TextMeasureKey& o) const {
            return text == o.text && font == o.font && size == o.size;
        }
    };
    struct TextMeasureKeyHash {
        size_t operator()(const TextMeasureKey& k) const {
            size_t h = std::hash<std::string>()(k.text);
            h ^= std::hash<std::string>()(k.font) + 0x9e3779b9 + (h << 6) + (h >> 2);
            h ^= std::hash<float>()(k.size) + 0x9e3779b9 + (h << 6) + (h >> 2);
            return h;
        }
    };
    std::unordered_map<TextMeasureKey, Size, TextMeasureKeyHash> measureCache_;
    uint32_t frameCount_ = 0;
};

} // namespace flux
