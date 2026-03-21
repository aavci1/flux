#pragma once

#include <Flux/Graphics/RenderContext.hpp>
#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

namespace flux {

enum class CmdOp : uint32_t {
    Save, Restore,
    Translate, Rotate, Scale,
    SetOpacity, SetFillStyle, SetStrokeStyle, SetTextStyle,
    DrawRect, DrawCircle, DrawLine, DrawPath,
    DrawText, DrawTextBox,
    DrawImage, DrawImagePath,
    ClipPath,
    Clear,
    BeginElement,
    EndElement
};

class RenderCommandBuffer {
public:
    // =========================================================================
    // Sequential reader for consumers (CommandCompiler, etc.)
    // =========================================================================

    class Reader {
    public:
        Reader(const uint32_t* begin, const uint32_t* end)
            : begin_(begin), pos_(begin), end_(end) {}

        bool hasNext() const { return pos_ < end_; }

        CmdOp nextOp() { return static_cast<CmdOp>(*pos_++); }

        float readFloat() {
            float v;
            std::memcpy(&v, pos_++, sizeof(float));
            return v;
        }

        uint32_t readUint32() { return *pos_++; }

        int32_t readInt32() { return static_cast<int32_t>(*pos_++); }

        uint32_t wordOffset() const { return static_cast<uint32_t>(pos_ - begin_); }

        void seekTo(uint32_t absoluteWordOffset) { pos_ = begin_ + absoluteWordOffset; }

    private:
        const uint32_t* begin_;
        const uint32_t* pos_;
        const uint32_t* end_;
    };

    // =========================================================================
    // Command writing (producers: GPURenderContext)
    // =========================================================================

    void pushSave()    { writeOp(CmdOp::Save); }
    void pushRestore() { writeOp(CmdOp::Restore); }

    void pushTranslate(float x, float y) {
        writeOp(CmdOp::Translate); writeF(x); writeF(y);
    }
    void pushRotate(float angle) {
        writeOp(CmdOp::Rotate); writeF(angle);
    }
    void pushScale(float sx, float sy) {
        writeOp(CmdOp::Scale); writeF(sx); writeF(sy);
    }
    void pushSetOpacity(float opacity) {
        writeOp(CmdOp::SetOpacity); writeF(opacity);
    }

    void pushSetFillStyle(const FillStyle& s) {
        uint32_t id = static_cast<uint32_t>(fillPool_.size());
        fillPool_.push_back(s);
        writeOp(CmdOp::SetFillStyle); writeU(id);
    }
    void pushSetStrokeStyle(const StrokeStyle& s) {
        uint32_t id = static_cast<uint32_t>(strokePool_.size());
        strokePool_.push_back(s);
        writeOp(CmdOp::SetStrokeStyle); writeU(id);
    }
    void pushSetTextStyle(const TextStyle& s) {
        uint32_t id = static_cast<uint32_t>(textStylePool_.size());
        textStylePool_.push_back(s);
        writeOp(CmdOp::SetTextStyle); writeU(id);
    }

    void pushDrawRect(const Rect& b, const CornerRadius& cr) {
        writeOp(CmdOp::DrawRect);
        writeF(b.x); writeF(b.y); writeF(b.width); writeF(b.height);
        writeF(cr.topLeft); writeF(cr.topRight); writeF(cr.bottomRight); writeF(cr.bottomLeft);
    }
    void pushDrawCircle(const Point& c, float radius) {
        writeOp(CmdOp::DrawCircle);
        writeF(c.x); writeF(c.y); writeF(radius);
    }
    void pushDrawLine(const Point& from, const Point& to) {
        writeOp(CmdOp::DrawLine);
        writeF(from.x); writeF(from.y); writeF(to.x); writeF(to.y);
    }
    void pushDrawPath(Path path) {
        uint32_t id = static_cast<uint32_t>(pathPool_.size());
        pathPool_.push_back(std::move(path));
        writeOp(CmdOp::DrawPath); writeU(id);
    }

    void pushDrawText(uint32_t strId, const Point& pos,
                      HorizontalAlignment hAlign, VerticalAlignment vAlign) {
        writeOp(CmdOp::DrawText);
        writeU(strId); writeF(pos.x); writeF(pos.y);
        writeU(static_cast<uint32_t>(hAlign));
        writeU(static_cast<uint32_t>(vAlign));
    }
    void pushDrawTextBox(uint32_t strId, const Point& pos,
                         float maxWidth, HorizontalAlignment hAlign) {
        writeOp(CmdOp::DrawTextBox);
        writeU(strId); writeF(pos.x); writeF(pos.y); writeF(maxWidth);
        writeU(static_cast<uint32_t>(hAlign));
    }

    void pushDrawImage(int imageId, const Rect& rect, ImageFit fit,
                       const CornerRadius& cr, float alpha) {
        writeOp(CmdOp::DrawImage);
        writeI(imageId);
        writeF(rect.x); writeF(rect.y); writeF(rect.width); writeF(rect.height);
        writeU(static_cast<uint32_t>(fit));
        writeF(cr.topLeft); writeF(cr.topRight); writeF(cr.bottomRight); writeF(cr.bottomLeft);
        writeF(alpha);
    }
    void pushDrawImagePath(uint32_t pathStrId, const Rect& rect, ImageFit fit,
                           const CornerRadius& cr, float alpha) {
        writeOp(CmdOp::DrawImagePath);
        writeU(pathStrId);
        writeF(rect.x); writeF(rect.y); writeF(rect.width); writeF(rect.height);
        writeU(static_cast<uint32_t>(fit));
        writeF(cr.topLeft); writeF(cr.topRight); writeF(cr.bottomRight); writeF(cr.bottomLeft);
        writeF(alpha);
    }

    void pushClipPath(Path path) {
        uint32_t id = static_cast<uint32_t>(pathPool_.size());
        pathPool_.push_back(std::move(path));
        writeOp(CmdOp::ClipPath); writeU(id);
    }
    void pushClear(const Color& c) {
        writeOp(CmdOp::Clear);
        writeF(c.r); writeF(c.g); writeF(c.b); writeF(c.a);
    }

    // Element boundary markers for incremental compilation.
    // Returns the word offset of the skip-distance placeholder.
    uint32_t pushBeginElement(uintptr_t elementId, uint64_t subtreeVersion) {
        uint32_t patchOffset = static_cast<uint32_t>(stream_.size());
        writeOp(CmdOp::BeginElement);
        writeU(static_cast<uint32_t>(elementId));
        writeU(static_cast<uint32_t>(elementId >> 32));
        writeU(static_cast<uint32_t>(subtreeVersion));
        writeU(static_cast<uint32_t>(subtreeVersion >> 32));
        writeU(0);  // placeholder: absolute word offset of EndElement
        return patchOffset;
    }
    void pushEndElement(uint32_t beginPatchOffset) {
        // Patch the skip-distance in the matching BeginElement (field at offset+5)
        stream_[beginPatchOffset + 5] = static_cast<uint32_t>(stream_.size());
        writeOp(CmdOp::EndElement);
    }

    // =========================================================================
    // String pool (deduplicated within the frame)
    // =========================================================================

    uint32_t internString(std::string s) {
        auto it = stringLookup_.find(s);
        if (it != stringLookup_.end()) return it->second;
        uint32_t id = static_cast<uint32_t>(stringPool_.size());
        stringPool_.push_back(std::move(s));
        stringLookup_[stringPool_.back()] = id;
        return id;
    }
    const std::string& str(uint32_t id) const { return stringPool_.at(id); }

    // =========================================================================
    // Pool accessors (for consumers that need the full objects)
    // =========================================================================

    const Path&        path(uint32_t id)      const { return pathPool_.at(id); }
    const FillStyle&   fillStyle(uint32_t id)  const { return fillPool_.at(id); }
    const StrokeStyle& strokeStyle(uint32_t id) const { return strokePool_.at(id); }
    const TextStyle&   textStyle(uint32_t id)  const { return textStylePool_.at(id); }

    // =========================================================================
    // Iteration
    // =========================================================================

    Reader reader() const {
        return Reader(stream_.data(), stream_.data() + stream_.size());
    }

    // =========================================================================
    // Housekeeping
    // =========================================================================

    size_t size() const { return stream_.size(); }
    bool   empty() const { return stream_.empty(); }

    void clear() {
        stream_.clear();
        pathPool_.clear();
        fillPool_.clear();
        strokePool_.clear();
        textStylePool_.clear();
        stringPool_.clear();
        stringLookup_.clear();
    }

    void reserve(size_t nWords) { stream_.reserve(nWords); }

private:
    std::vector<uint32_t> stream_;

    std::vector<Path>        pathPool_;
    std::vector<FillStyle>   fillPool_;
    std::vector<StrokeStyle> strokePool_;
    std::vector<TextStyle>   textStylePool_;

    std::vector<std::string>                    stringPool_;
    std::unordered_map<std::string, uint32_t>   stringLookup_;

    void writeOp(CmdOp op) { stream_.push_back(static_cast<uint32_t>(op)); }
    void writeF(float v)   { uint32_t u; std::memcpy(&u, &v, sizeof(float)); stream_.push_back(u); }
    void writeU(uint32_t v){ stream_.push_back(v); }
    void writeI(int32_t v) { stream_.push_back(static_cast<uint32_t>(v)); }
};

class RenderBackend {
public:
    virtual ~RenderBackend() = default;
    virtual void execute(const RenderCommandBuffer& buffer) = 0;
};

} // namespace flux
