#include <Flux/Graphics/GlyphAtlas.hpp>
#include <cstring>
#include <algorithm>
#include <sstream>
#include <vector>

namespace flux {

namespace {

template<typename Fn>
void utf8ForEach(const std::string& text, Fn&& fn) {
    for (size_t i = 0; i < text.size();) {
        uint32_t cp = static_cast<uint8_t>(text[i]);
        size_t bytes = 1;
        if (cp >= 0xC0 && cp < 0xE0 && i + 1 < text.size()) {
            cp = ((cp & 0x1F) << 6) | (static_cast<uint8_t>(text[i + 1]) & 0x3F);
            bytes = 2;
        } else if (cp >= 0xE0 && cp < 0xF0 && i + 2 < text.size()) {
            cp = ((cp & 0x0F) << 12) | ((static_cast<uint8_t>(text[i + 1]) & 0x3F) << 6)
                | (static_cast<uint8_t>(text[i + 2]) & 0x3F);
            bytes = 3;
        } else if (cp >= 0xF0 && i + 3 < text.size()) {
            cp = ((cp & 0x07) << 18) | ((static_cast<uint8_t>(text[i + 1]) & 0x3F) << 12)
                | ((static_cast<uint8_t>(text[i + 2]) & 0x3F) << 6)
                | (static_cast<uint8_t>(text[i + 3]) & 0x3F);
            bytes = 4;
        }
        i += bytes;
        fn(cp);
    }
}

} // namespace

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
    FT_Face face = nullptr;
    if (FT_New_Face(ftLib_, path.c_str(), 0, &face) != 0) {
        return false;
    }
    if (auto it = faces_.find(fontIndex); it != faces_.end()) {
        FT_Done_Face(it->second);
        // Only drop glyphs for this slot — other faces keep valid UVs in the shared atlas.
        for (auto cit = cache_.begin(); cit != cache_.end();) {
            if (cit->first.fontIndex == fontIndex) {
                cit = cache_.erase(cit);
            } else {
                ++cit;
            }
        }
    }
    faces_[fontIndex] = face;
    dirty_ = true;
    return true;
}

bool GlyphAtlas::loadFontByName(const std::string& name, FontWeight weight, uint16_t fontIndex) {
    auto path = FontProvider::findFontPath(name, weight);
    if (!path.has_value()) return false;
    return loadFont(path.value(), fontIndex);
}

std::optional<uint16_t> GlyphAtlas::ensureFontLoaded(const std::string& name, FontWeight weight) {
    // Views use makeTextStyle("default", …); "default" is not a real family name on any OS.
    const std::string cacheName = (name.empty() || name == "default") ? std::string("default") : name;
    const std::string key = cacheName + "_" + std::to_string(static_cast<int>(weight));

    if (auto it = fontKeyToIndex_.find(key); it != fontKeyToIndex_.end()) {
        if (faces_.count(it->second) > 0) {
            return it->second;
        }
        fontKeyToIndex_.erase(it);
    }

    // GPU bootstrap often loads Helvetica first; share that slot for the "default" UI font.
    if (cacheName == "default") {
        const std::string helvKey = std::string("Helvetica_") + std::to_string(static_cast<int>(weight));
        if (auto it = fontKeyToIndex_.find(helvKey); it != fontKeyToIndex_.end() && faces_.count(it->second) > 0) {
            fontKeyToIndex_[key] = it->second;
            return it->second;
        }
    }

    auto tryLoad = [&](const std::string& family) -> bool {
        const uint16_t idx = nextFontIndex_++;
        if (loadFontByName(family, weight, idx)) {
            fontKeyToIndex_[key] = idx;
            return true;
        }
        if (nextFontIndex_ > 0) {
            --nextFontIndex_;
        }
        return false;
    };

    if (cacheName != "default") {
        if (tryLoad(cacheName)) {
            return fontKeyToIndex_[key];
        }
        return std::nullopt;
    }

    static const char* fallbacks[] = {
        "Helvetica", "Arial", ".AppleSystemUIFont", "SF Pro Text",
        "DejaVu Sans", "Segoe UI", "Liberation Sans",
    };
    for (const char* fb : fallbacks) {
        if (tryLoad(std::string(fb))) {
            return fontKeyToIndex_[key];
        }
    }
    return std::nullopt;
}

std::string GlyphAtlas::faceFilePath(FT_Face face) const {
    if (face && face->stream && face->stream->pathname.pointer) {
        return std::string(static_cast<const char*>(face->stream->pathname.pointer));
    }
    return {};
}

std::optional<uint16_t> GlyphAtlas::loadFallbackForCodepoint(uint32_t codepoint,
                                                              uint16_t baseFontIndex) {
    if (codepointMisses_.count(codepoint)) {
        return std::nullopt;
    }

    // Already-loaded fallbacks may cover this codepoint -- check them first.
    for (auto& [path, idx] : fallbackPathToIndex_) {
        auto fit = faces_.find(idx);
        if (fit == faces_.end()) continue;
        if (FT_Get_Char_Index(fit->second, codepoint) != 0) {
            return idx;
        }
    }

    // Ask the OS for the best font covering this codepoint.
    std::string basePath;
    if (auto it = faces_.find(baseFontIndex); it != faces_.end()) {
        basePath = faceFilePath(it->second);
    }
    auto resolved = FontProvider::findFontPathForCodepoint(codepoint, basePath);
    if (!resolved.has_value()) {
        codepointMisses_.insert(codepoint);
        return std::nullopt;
    }

    // Reuse if we already loaded this exact font file.
    if (auto it = fallbackPathToIndex_.find(resolved.value()); it != fallbackPathToIndex_.end()) {
        // Double-check the font actually has the glyph (the OS can return the base font itself
        // when there is truly no coverage).
        auto fit = faces_.find(it->second);
        if (fit != faces_.end() && FT_Get_Char_Index(fit->second, codepoint) != 0) {
            return it->second;
        }
        codepointMisses_.insert(codepoint);
        return std::nullopt;
    }

    uint16_t idx = nextFontIndex_++;
    if (!loadFont(resolved.value(), idx)) {
        --nextFontIndex_;
        codepointMisses_.insert(codepoint);
        return std::nullopt;
    }
    fallbackPathToIndex_[resolved.value()] = idx;

    if (FT_Get_Char_Index(faces_[idx], codepoint) == 0) {
        codepointMisses_.insert(codepoint);
        return std::nullopt;
    }
    return idx;
}

bool GlyphAtlas::rasterizeGlyph(const GlyphKey& key, GlyphInfo& out) {
    auto tryRender = [&](uint16_t fi) -> bool {
        auto it = faces_.find(fi);
        if (it == faces_.end()) return false;

        FT_Face face = it->second;
        FT_Set_Pixel_Sizes(face, 0, key.fontSize);

        FT_UInt glyphIndex = FT_Get_Char_Index(face, key.codepoint);
        if (glyphIndex == 0) return false;

        if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER) != 0) return false;

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
        if (cursorY_ + gh + pad > atlasSize_) {
            return false;
        }

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
    };

    // 1. Try the requested font.
    if (tryRender(key.fontIndex)) return true;

    // 2. Ask the OS for a fallback and try it.
    auto fb = loadFallbackForCodepoint(key.codepoint, key.fontIndex);
    if (fb.has_value() && tryRender(fb.value())) return true;

    return false;
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

    utf8ForEach(text, [&](uint32_t cp) {
        auto* g = getGlyph(cp, fsz, fontIndex);
        if (g) {
            width += g->advance;
            maxH = std::max(maxH, g->height);
        } else if (const auto* sp = getGlyph(static_cast<uint32_t>(' '), fsz, fontIndex)) {
            width += sp->advance;
        } else {
            width += fontSize * 0.5f;
        }
    });
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
        utf8ForEach(w, [&](uint32_t cp) {
            auto* g = getGlyph(cp, fsz, fontIndex);
            if (g) {
                ww += g->advance;
            } else if (const auto* sp = getGlyph(static_cast<uint32_t>(' '), fsz, fontIndex)) {
                ww += sp->advance;
            } else {
                ww += fontSize * 0.5f;
            }
        });
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

    utf8ForEach(text, [&](uint32_t cp) {
        auto* g = getGlyph(cp, fsz, fontIndex);
        if (!g) {
            if (const auto* sp = getGlyph(static_cast<uint32_t>(' '), fsz, fontIndex)) {
                penX += sp->advance;
            } else {
                penX += fontSize * 0.5f;
            }
            return;
        }
        if (g->width == 0 && g->height == 0) {
            penX += g->advance;
            return;
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
    });

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
