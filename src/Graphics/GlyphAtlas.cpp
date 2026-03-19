#include <Flux/Graphics/GlyphAtlas.hpp>
#include <Flux/Core/FontDiscovery.hpp>
#include <cstring>
#include <algorithm>
#include <sstream>

namespace flux {

GlyphAtlas::GlyphAtlas(gpu::Device* device, uint32_t atlasSize)
    : device_(device), atlasSize_(atlasSize)
{
    FT_Init_FreeType(&ftLib_);

    atlasData_.resize(atlasSize_ * atlasSize_, 0);

    gpu::TextureDesc td;
    td.width = atlasSize_;
    td.height = atlasSize_;
    td.format = gpu::PixelFormat::R8;
    texture_ = device_->createTexture(td);
}

GlyphAtlas::~GlyphAtlas() {
    for (auto& [idx, face] : faces_) FT_Done_Face(face);
    if (ftLib_) FT_Done_FreeType(ftLib_);
}

bool GlyphAtlas::loadFont(const std::string& path, uint16_t fontIndex) {
    FT_Face face;
    if (FT_New_Face(ftLib_, path.c_str(), 0, &face) != 0) return false;
    faces_[fontIndex] = face;
    return true;
}

bool GlyphAtlas::loadFontByName(const std::string& name, FontWeight weight, uint16_t fontIndex) {
    auto path = FontDiscovery::findFontPath(name, weight);
    if (!path.has_value()) return false;
    return loadFont(path.value(), fontIndex);
}

bool GlyphAtlas::rasterizeGlyph(const GlyphKey& key, GlyphInfo& out) {
    auto it = faces_.find(key.fontIndex);
    if (it == faces_.end()) return false;

    FT_Face face = it->second;
    FT_Set_Pixel_Sizes(face, 0, key.fontSize);

    if (FT_Load_Char(face, key.codepoint, FT_LOAD_RENDER) != 0) return false;

    FT_GlyphSlot g = face->glyph;
    uint32_t gw = g->bitmap.width;
    uint32_t gh = g->bitmap.rows;

    if (gw == 0 || gh == 0) {
        out.u0 = out.v0 = out.u1 = out.v1 = 0;
        out.width = 0;
        out.height = 0;
        out.bearingX = static_cast<float>(g->bitmap_left);
        out.bearingY = static_cast<float>(g->bitmap_top);
        out.advance = static_cast<float>(g->advance.x >> 6);
        return true;
    }

    uint32_t pad = 1;
    if (cursorX_ + gw + pad > atlasSize_) {
        cursorX_ = 0;
        cursorY_ += rowHeight_ + pad;
        rowHeight_ = 0;
    }
    if (cursorY_ + gh + pad > atlasSize_) return false;

    for (uint32_t row = 0; row < gh; row++) {
        uint32_t dstY = cursorY_ + row;
        std::memcpy(&atlasData_[dstY * atlasSize_ + cursorX_],
                     &g->bitmap.buffer[row * g->bitmap.pitch], gw);
    }

    float inv = 1.0f / static_cast<float>(atlasSize_);
    out.u0 = static_cast<float>(cursorX_) * inv;
    out.v0 = static_cast<float>(cursorY_) * inv;
    out.u1 = static_cast<float>(cursorX_ + gw) * inv;
    out.v1 = static_cast<float>(cursorY_ + gh) * inv;
    out.width = static_cast<float>(gw);
    out.height = static_cast<float>(gh);
    out.bearingX = static_cast<float>(g->bitmap_left);
    out.bearingY = static_cast<float>(g->bitmap_top);
    out.advance = static_cast<float>(g->advance.x >> 6);

    cursorX_ += gw + pad;
    rowHeight_ = std::max(rowHeight_, gh);
    dirty_ = true;

    return true;
}

const GlyphInfo* GlyphAtlas::getGlyph(uint32_t codepoint, uint16_t fontSize, uint16_t fontIndex) {
    GlyphKey key{codepoint, fontSize, fontIndex};
    auto it = cache_.find(key);
    if (it != cache_.end()) return &it->second;

    GlyphInfo info{};
    if (!rasterizeGlyph(key, info)) return nullptr;
    auto [inserted, _] = cache_.emplace(key, info);
    return &inserted->second;
}

void GlyphAtlas::uploadIfDirty() {
    if (!dirty_) return;
    texture_->write(atlasData_.data(), 0, 0, atlasSize_, atlasSize_);
    dirty_ = false;
}

Size GlyphAtlas::measureText(const std::string& text, float fontSize, uint16_t fontIndex) {
    uint16_t fsz = static_cast<uint16_t>(fontSize);
    float width = 0, maxH = 0;

    for (size_t i = 0; i < text.size(); ) {
        uint32_t cp = static_cast<uint8_t>(text[i]);
        size_t bytes = 1;
        if (cp >= 0xC0 && cp < 0xE0 && i + 1 < text.size()) {
            cp = ((cp & 0x1F) << 6) | (text[i+1] & 0x3F);
            bytes = 2;
        } else if (cp >= 0xE0 && cp < 0xF0 && i + 2 < text.size()) {
            cp = ((cp & 0x0F) << 12) | ((text[i+1] & 0x3F) << 6) | (text[i+2] & 0x3F);
            bytes = 3;
        } else if (cp >= 0xF0 && i + 3 < text.size()) {
            cp = ((cp & 0x07) << 18) | ((text[i+1] & 0x3F) << 12) |
                 ((text[i+2] & 0x3F) << 6) | (text[i+3] & 0x3F);
            bytes = 4;
        }
        i += bytes;

        auto* g = getGlyph(cp, fsz, fontIndex);
        if (g) {
            width += g->advance;
            maxH = std::max(maxH, g->height);
        }
    }
    return {width, std::max(maxH, fontSize)};
}

Size GlyphAtlas::measureTextBox(const std::string& text, float fontSize, float maxWidth, uint16_t fontIndex) {
    auto lines = wrapText(text, fontSize, maxWidth, fontIndex);
    float totalH = 0;
    float maxW = 0;
    for (const auto& line : lines) {
        auto sz = measureText(line, fontSize, fontIndex);
        maxW = std::max(maxW, sz.width);
        totalH += fontSize * 1.2f;
    }
    return {std::min(maxW, maxWidth), totalH};
}

std::vector<std::string> GlyphAtlas::wrapText(const std::string& text, float fontSize,
                                                float maxWidth, uint16_t fontIndex) {
    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string word;
    std::string currentLine;
    float lineWidth = 0;
    uint16_t fsz = static_cast<uint16_t>(fontSize);

    auto wordWidth = [&](const std::string& w) -> float {
        float ww = 0;
        for (char c : w) {
            auto* g = getGlyph(static_cast<uint8_t>(c), fsz, fontIndex);
            if (g) ww += g->advance;
        }
        return ww;
    };

    auto spaceW = wordWidth(" ");

    while (stream >> word) {
        float ww = wordWidth(word);
        if (!currentLine.empty() && lineWidth + spaceW + ww > maxWidth) {
            lines.push_back(currentLine);
            currentLine = word;
            lineWidth = ww;
        } else {
            if (!currentLine.empty()) {
                currentLine += " ";
                lineWidth += spaceW;
            }
            currentLine += word;
            lineWidth += ww;
        }
    }
    if (!currentLine.empty()) lines.push_back(currentLine);
    if (lines.empty()) lines.push_back("");
    return lines;
}

std::vector<GlyphInstance> GlyphAtlas::layoutText(const std::string& text, float x, float y,
                                                    float fontSize, const Color& color,
                                                    float vpW, float vpH, uint16_t fontIndex) {
    std::vector<GlyphInstance> out;
    uint16_t fsz = static_cast<uint16_t>(fontSize);
    float penX = x;

    for (size_t i = 0; i < text.size(); ) {
        uint32_t cp = static_cast<uint8_t>(text[i]);
        size_t bytes = 1;
        if (cp >= 0xC0 && cp < 0xE0 && i + 1 < text.size()) {
            cp = ((cp & 0x1F) << 6) | (text[i+1] & 0x3F); bytes = 2;
        } else if (cp >= 0xE0 && cp < 0xF0 && i + 2 < text.size()) {
            cp = ((cp & 0x0F) << 12) | ((text[i+1] & 0x3F) << 6) | (text[i+2] & 0x3F); bytes = 3;
        } else if (cp >= 0xF0 && i + 3 < text.size()) {
            cp = ((cp & 0x07) << 18) | ((text[i+1] & 0x3F) << 12) |
                 ((text[i+2] & 0x3F) << 6) | (text[i+3] & 0x3F); bytes = 4;
        }
        i += bytes;

        auto* g = getGlyph(cp, fsz, fontIndex);
        if (!g || (g->width == 0 && g->height == 0)) {
            if (g) penX += g->advance;
            continue;
        }

        GlyphInstance inst{};
        inst.screenRect[0] = penX + g->bearingX;
        inst.screenRect[1] = y - g->bearingY;
        inst.screenRect[2] = g->width;
        inst.screenRect[3] = g->height;
        inst.uvRect[0] = g->u0;
        inst.uvRect[1] = g->v0;
        inst.uvRect[2] = g->u1;
        inst.uvRect[3] = g->v1;
        inst.color[0] = color.r;
        inst.color[1] = color.g;
        inst.color[2] = color.b;
        inst.color[3] = color.a;
        inst.viewport[0] = vpW;
        inst.viewport[1] = vpH;

        out.push_back(inst);
        penX += g->advance;
    }

    return out;
}

std::vector<GlyphInstance> GlyphAtlas::layoutTextBox(const std::string& text, float x, float y,
                                                       float fontSize, float maxWidth,
                                                       const Color& color,
                                                       float vpW, float vpH,
                                                       HorizontalAlignment hAlign,
                                                       uint16_t fontIndex) {
    auto lines = wrapText(text, fontSize, maxWidth, fontIndex);
    std::vector<GlyphInstance> out;
    float lineH = fontSize * 1.2f;
    float penY = y + fontSize;

    for (const auto& line : lines) {
        float lineW = measureText(line, fontSize, fontIndex).width;
        float startX = x;
        if (hAlign == HorizontalAlignment::center) startX = x + (maxWidth - lineW) * 0.5f;
        else if (hAlign == HorizontalAlignment::trailing) startX = x + maxWidth - lineW;

        auto glyphs = layoutText(line, startX, penY, fontSize, color, vpW, vpH, fontIndex);
        out.insert(out.end(), glyphs.begin(), glyphs.end());
        penY += lineH;
    }
    return out;
}

} // namespace flux
