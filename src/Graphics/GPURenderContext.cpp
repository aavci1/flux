#include <Flux/Graphics/GPURenderContext.hpp>
#include <Flux/Graphics/FontProvider.hpp>
#include <Flux/Graphics/GPURendererBackend.hpp>
#include <algorithm>
#include <cmath>
#include <cstring>

namespace flux {

GPURenderContext::GPURenderContext(FontProvider* fontProvider, ImageCache* imageCache,
                                   GPURendererBackend* backend,
                                   int width, int height, float dpiScaleX, float dpiScaleY)
    : fontProvider_(fontProvider), imageCache_(imageCache), gpuBackend_(backend),
      width_(width), height_(height), dpiScaleX_(dpiScaleX), dpiScaleY_(dpiScaleY),
      currentFill_(FillStyle::none()), currentStroke_(StrokeStyle::none()) {}

void GPURenderContext::beginFrame() {
    commandBufferPeak_ = std::max(commandBufferPeak_, ownedBuffer_.size());
    ownedBuffer_.clear();
    ownedBuffer_.reserve(commandBufferPeak_);
    cmdBuf_ = &ownedBuffer_;
    recordingBuffer_ = &ownedBuffer_;
}

void GPURenderContext::clear(const Color& color) {
    if (cmdBuf_) cmdBuf_->pushClear(color);
}

void GPURenderContext::present() {
    RenderCommandBuffer* buf = cmdBuf_ ? cmdBuf_ : &ownedBuffer_;
    if (gpuBackend_ && !buf->empty()) {
        gpuBackend_->execute(*buf);
    }
    cmdBuf_ = nullptr;
    recordingBuffer_ = nullptr;
}

void GPURenderContext::resize(int width, int height) {
    width_ = width;
    height_ = height;
}

void GPURenderContext::updateDPIScale(float dpiScaleX, float dpiScaleY) {
    dpiScaleX_ = dpiScaleX;
    dpiScaleY_ = dpiScaleY;
}

void GPURenderContext::save() {
    transformStack_.push_back(transform_);
    if (cmdBuf_) cmdBuf_->pushSave();
}

void GPURenderContext::restore() {
    if (!transformStack_.empty()) {
        transform_ = transformStack_.back();
        transformStack_.pop_back();
    }
    if (cmdBuf_) cmdBuf_->pushRestore();
}

void GPURenderContext::reset() {
    transform_ = TransformState{};
    transformStack_.clear();
    measureFifo_.clear();
    measureCacheIt_.clear();
}

void GPURenderContext::translate(float x, float y) {
    transform_.matrix[4] += transform_.matrix[0] * x + transform_.matrix[2] * y;
    transform_.matrix[5] += transform_.matrix[1] * x + transform_.matrix[3] * y;
    transform_.tx += x;
    transform_.ty += y;
    if (cmdBuf_) cmdBuf_->pushTranslate(x, y);
}

void GPURenderContext::rotate(float angle) {
    const float co = std::cos(angle);
    const float si = std::sin(angle);
    const float m00 = transform_.matrix[0];
    const float m01 = transform_.matrix[2];
    const float m10 = transform_.matrix[1];
    const float m11 = transform_.matrix[3];
    transform_.matrix[0] = m00 * co + m01 * si;
    transform_.matrix[2] = -m00 * si + m01 * co;
    transform_.matrix[1] = m10 * co + m11 * si;
    transform_.matrix[3] = -m10 * si + m11 * co;
    if (cmdBuf_) cmdBuf_->pushRotate(angle);
}

void GPURenderContext::scale(float sx, float sy) {
    transform_.sx *= sx;
    transform_.sy *= sy;
    transform_.matrix[0] *= sx;
    transform_.matrix[2] *= sy;
    transform_.matrix[1] *= sx;
    transform_.matrix[3] *= sy;
    if (cmdBuf_) cmdBuf_->pushScale(sx, sy);
}

void GPURenderContext::skewX(float) {}
void GPURenderContext::skewY(float) {}

void GPURenderContext::setTransform(float a, float b, float c, float d, float e, float f) {
    transform_.matrix[0] = a; transform_.matrix[1] = b;
    transform_.matrix[2] = c; transform_.matrix[3] = d;
    transform_.matrix[4] = e; transform_.matrix[5] = f;
    transform_.tx = e; transform_.ty = f;
    transform_.sx = a; transform_.sy = d;
}

void GPURenderContext::resetTransform() {
    transform_ = TransformState{};
}

void GPURenderContext::getTransform(float* matrix) {
    std::memcpy(matrix, transform_.matrix, sizeof(transform_.matrix));
}

void GPURenderContext::setCompositeOperation(CompositeOperation) {}

void GPURenderContext::setOpacity(float alpha) {
    if (cmdBuf_) cmdBuf_->pushSetOpacity(alpha);
}

void GPURenderContext::setShapeAntiAlias(bool) {}

void GPURenderContext::setStrokeColor(const Color& color) {
    currentStroke_.color = color;
}

void GPURenderContext::setStrokeWidth(float width) {
    currentStroke_.width = width;
}

void GPURenderContext::setLineCap(LineCap cap) {
    currentStroke_.cap = cap;
}

void GPURenderContext::setLineJoin(LineJoin join) {
    currentStroke_.join = join;
}

void GPURenderContext::setMiterLimit(float limit) {
    currentStroke_.miterLimit = limit;
}

void GPURenderContext::setDashPattern(const std::vector<float>& pattern, float offset) {
    currentStroke_.dashPattern = pattern;
    currentStroke_.dashOffset = offset;
}

void GPURenderContext::setStrokeStyle(const StrokeStyle& style) {
    currentStroke_ = style;
    if (cmdBuf_) cmdBuf_->pushSetStrokeStyle(style);
}

void GPURenderContext::setFillColor(const Color& color) {
    currentFill_.color = color;
    currentFill_.type = FillStyle::Type::Solid;
}

void GPURenderContext::setPathWinding(PathWinding winding) {
    currentFill_.winding = winding;
}

void GPURenderContext::setFillStyle(const FillStyle& style) {
    currentFill_ = style;
    if (cmdBuf_) cmdBuf_->pushSetFillStyle(style);
}

void GPURenderContext::drawPath(const Path& path) {
    if (path.isEmpty()) return;
    if (cmdBuf_) cmdBuf_->pushDrawPath(path);
}

void GPURenderContext::drawCircle(const Point& center, float radius) {
    if (cmdBuf_) cmdBuf_->pushDrawCircle(center, radius);
}

void GPURenderContext::drawLine(const Point& start, const Point& end) {
    if (cmdBuf_) cmdBuf_->pushDrawLine(start, end);
}

void GPURenderContext::drawRect(const Rect& rect, const CornerRadius& cornerRadius) {
    if (cmdBuf_) cmdBuf_->pushDrawRect(rect, cornerRadius);
}

void GPURenderContext::drawEllipse(const Point& center, float radiusX, float radiusY) {
    Path p;
    p.ellipse(center, radiusX, radiusY);
    if (cmdBuf_) cmdBuf_->pushDrawPath(std::move(p));
}

void GPURenderContext::drawArc(const Point& center, float radius,
                                float startAngle, float endAngle, bool clockwise) {
    Path p;
    p.arc(center, radius, startAngle, endAngle, clockwise);
    if (cmdBuf_) cmdBuf_->pushDrawPath(std::move(p));
}

void GPURenderContext::setFont(const std::string& name, FontWeight weight) {
    currentTextStyle_.fontName = name;
    currentTextStyle_.weight = weight;
}

void GPURenderContext::setFontSize(float size) {
    currentTextStyle_.size = size;
}

void GPURenderContext::setFontBlur(float) {}

void GPURenderContext::setLetterSpacing(float spacing) {
    currentTextStyle_.letterSpacing = spacing;
}

void GPURenderContext::setLineHeight(float height) {
    currentTextStyle_.lineHeight = height;
}

void GPURenderContext::setTextStyle(const TextStyle& style) {
    currentTextStyle_ = style;
    if (cmdBuf_) cmdBuf_->pushSetTextStyle(style);
}

void GPURenderContext::drawText(const std::string& text, const Point& position,
                                HorizontalAlignment hAlign, VerticalAlignment vAlign) {
    if (cmdBuf_) {
        uint32_t sid = cmdBuf_->internString(std::string(text));
        cmdBuf_->pushDrawText(sid, position, hAlign, vAlign);
    }
}

void GPURenderContext::drawTextBox(const std::string& text, const Point& position,
                                    float maxWidth, HorizontalAlignment hAlign) {
    if (cmdBuf_) {
        uint32_t sid = cmdBuf_->internString(std::string(text));
        cmdBuf_->pushDrawTextBox(sid, position, maxWidth, hAlign);
    }
}

Size GPURenderContext::measureText(const std::string& text, const TextStyle& style) {
    TextMeasureKey key{text, style.fontName, style.size};
    auto it = measureCacheIt_.find(key);
    if (it != measureCacheIt_.end()) {
        return it->second->second;
    }

    if (!fontProvider_) return {0, 0};

    auto fontIndex = fontProvider_->ensureFontLoaded(style.fontName, style.weight);
    if (!fontIndex) {
        return {0, 0};
    }
    Size sz = fontProvider_->measureText(text, style.size, *fontIndex);

    if (measureFifo_.size() >= kMeasureCacheMaxEntries) {
        auto oldest = measureFifo_.begin();
        measureCacheIt_.erase(oldest->first);
        measureFifo_.pop_front();
    }
    measureFifo_.push_back({std::move(key), sz});
    auto lit = std::prev(measureFifo_.end());
    measureCacheIt_[lit->first] = lit;
    return sz;
}

Size GPURenderContext::measureTextBox(const std::string& text, const TextStyle& style, float maxWidth) {
    if (!fontProvider_) return {0, 0};
    auto fontIndex = fontProvider_->ensureFontLoaded(style.fontName, style.weight);
    if (!fontIndex) {
        return {0, 0};
    }
    return fontProvider_->measureTextBox(text, style.size, maxWidth, *fontIndex);
}

Rect GPURenderContext::getTextBounds(const std::string& text, const Point& position, const TextStyle& style) {
    Size sz = measureText(text, style);
    return {position.x, position.y - sz.height, sz.width, sz.height};
}

int GPURenderContext::createImage(const std::string& filename) {
    if (!imageCache_) return 0;
    return imageCache_->loadFromFile(filename);
}

int GPURenderContext::createImageMem(const unsigned char*, int) {
    return 0;
}

int GPURenderContext::createImageRGBA(int width, int height, const unsigned char* data) {
    if (!imageCache_) return 0;
    return imageCache_->loadFromMemory(data, width, height, 4);
}

void GPURenderContext::updateImage(int, const unsigned char*) {}

Size GPURenderContext::getImageSize(int) {
    return {0, 0};
}

void GPURenderContext::deleteImage(int) {}

void GPURenderContext::drawImage(int imageId, const Rect& rect, ImageFit fit,
                                  const CornerRadius& cornerRadius, float alpha) {
    if (cmdBuf_) cmdBuf_->pushDrawImage(imageId, rect, fit, cornerRadius, alpha);
}

void GPURenderContext::drawImage(const std::string& path, const Rect& rect, ImageFit fit,
                                  const CornerRadius& cornerRadius, float alpha) {
    if (cmdBuf_) {
        uint32_t pid = cmdBuf_->internString(std::string(path));
        cmdBuf_->pushDrawImagePath(pid, rect, fit, cornerRadius, alpha);
    }
}

void GPURenderContext::clipPath(const Path& path) {
    if (cmdBuf_) cmdBuf_->pushClipPath(path);
}

void GPURenderContext::resetClip() {}

Point GPURenderContext::transformPoint(const Point& point) {
    const float* m = transform_.matrix;
    return {point.x * m[0] + point.y * m[2] + m[4],
            point.x * m[1] + point.y * m[3] + m[5]};
}

Rect GPURenderContext::transformRect(const Rect& rect) {
    Point p0 = transformPoint({rect.x, rect.y});
    Point p1 = transformPoint({rect.x + rect.width, rect.y});
    Point p2 = transformPoint({rect.x, rect.y + rect.height});
    Point p3 = transformPoint({rect.x + rect.width, rect.y + rect.height});
    float minX = std::min({p0.x, p1.x, p2.x, p3.x});
    float maxX = std::max({p0.x, p1.x, p2.x, p3.x});
    float minY = std::min({p0.y, p1.y, p2.y, p3.y});
    float maxY = std::max({p0.y, p1.y, p2.y, p3.y});
    return {minX, minY, maxX - minX, maxY - minY};
}

float GPURenderContext::degToRad(float degrees) { return degrees * 0.017453293f; }
float GPURenderContext::radToDeg(float radians) { return radians * 57.29577951f; }

void GPURenderContext::setCurrentFocusKey(const std::string& focusKey) {
    currentViewFocusKey_ = focusKey;
}

std::string GPURenderContext::getFocusedKey() const {
    return globalFocusedKey_;
}

bool GPURenderContext::isCurrentViewFocused() const {
    return !currentViewFocusKey_.empty() && currentViewFocusKey_ == globalFocusedKey_;
}

void GPURenderContext::setHoveredBounds(const Rect& bounds) {
    hoveredBounds_ = bounds;
    hasHovered_ = true;
}

void GPURenderContext::setCurrentViewGlobalBounds(const Rect& bounds) {
    currentViewBounds_ = bounds;
}

bool GPURenderContext::isCurrentViewHovered() const {
    if (!hasHovered_) return false;
    return currentViewBounds_.x == hoveredBounds_.x &&
           currentViewBounds_.y == hoveredBounds_.y &&
           currentViewBounds_.width == hoveredBounds_.width &&
           currentViewBounds_.height == hoveredBounds_.height;
}

void GPURenderContext::setPressedBounds(const Rect& bounds) {
    pressedBounds_ = bounds;
    hasPressed_ = true;
}

void GPURenderContext::clearPressedBounds() {
    hasPressed_ = false;
}

bool GPURenderContext::isCurrentViewPressed() const {
    if (!hasPressed_) return false;
    return currentViewBounds_.x == pressedBounds_.x &&
           currentViewBounds_.y == pressedBounds_.y &&
           currentViewBounds_.width == pressedBounds_.width &&
           currentViewBounds_.height == pressedBounds_.height;
}

} // namespace flux
