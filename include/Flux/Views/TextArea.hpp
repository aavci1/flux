#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <string>
#include <functional>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <sstream>

namespace flux {

struct TextArea {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;

    Property<std::string> value = std::string("");
    Property<std::string> placeholder = std::string("");
    Property<bool> readOnly = false;
    Property<float> fontSize = 14.0f;
    Property<float> areaMinHeight = 72.0f;
    Property<float> areaMaxHeight = 200.0f;
    Property<bool> autoExpand = true;
    Property<Color> textColor = Color(0.92f, 0.92f, 0.92f);
    Property<Color> bgColor = Color(0.12f, 0.12f, 0.12f);
    Property<Color> borderCol = Color(0.22f, 0.22f, 0.22f);
    Property<Color> focusBorderColor = Colors::blue;
    Property<Color> placeholderColor = Color(0.48f, 0.48f, 0.48f);
    Property<float> areaCornerRadius = 4.0f;
    Property<float> areaPadding = 10.0f;
    Property<float> areaWidth = 400.0f;

    std::function<void(const std::string&)> onValueChange;
    std::function<void()> onSubmit;

    mutable size_t caretPos = 0;
    mutable float scrollY = 0.0f;

    void init() {
        focusable = true;
        cursor = CursorType::Text;

        onScroll = [this](float, float, float, float dy) {
            scrollY = std::max(0.0f, scrollY + dy);
        };
    }

    bool handleTextInput(const TextInputEvent& event) const {
        if (static_cast<bool>(readOnly)) return false;
        std::string val = value;
        val.insert(caretPos, event.text);
        caretPos += event.text.size();
        const_cast<Property<std::string>&>(value) = val;
        if (onValueChange) onValueChange(val);
        return true;
    }

    bool handleKeyDown(const KeyEvent& event) const {
        std::string val = value;

        if (event.key == Key::Enter) {
            if (event.hasCtrl() || event.hasSuper()) {
                if (onSubmit) onSubmit();
                return true;
            }
            if (!static_cast<bool>(readOnly)) {
                val.insert(caretPos, "\n");
                caretPos++;
                const_cast<Property<std::string>&>(value) = val;
                if (onValueChange) onValueChange(val);
            }
            return true;
        }

        if (event.key == Key::Backspace && !static_cast<bool>(readOnly)) {
            if (caretPos > 0) {
                val.erase(caretPos - 1, 1);
                caretPos--;
                const_cast<Property<std::string>&>(value) = val;
                if (onValueChange) onValueChange(val);
            }
            return true;
        }

        if (event.key == Key::Delete && !static_cast<bool>(readOnly)) {
            if (caretPos < val.size()) {
                val.erase(caretPos, 1);
                const_cast<Property<std::string>&>(value) = val;
                if (onValueChange) onValueChange(val);
            }
            return true;
        }

        if (event.key == Key::Left) {
            if (caretPos > 0) caretPos--;
            return true;
        }
        if (event.key == Key::Right) {
            if (caretPos < val.size()) caretPos++;
            return true;
        }
        if (event.key == Key::Home) { caretPos = 0; return true; }
        if (event.key == Key::End) { caretPos = val.size(); return true; }

        if (event.key == Key::Up) {
            moveCaretVertically(val, -1);
            return true;
        }
        if (event.key == Key::Down) {
            moveCaretVertically(val, 1);
            return true;
        }

        if (event.hasCtrl() && event.key == Key::A) {
            caretPos = val.size();
            return true;
        }

        return false;
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        float pad = areaPadding;
        float rad = areaCornerRadius;
        bool isFocused = ctx.isCurrentViewFocused();
        bool isHovered = ctx.isCurrentViewHovered();

        ctx.setFillStyle(FillStyle::solid(bgColor));
        ctx.setStrokeStyle(StrokeStyle::none());
        ctx.drawRect(bounds, CornerRadius(rad));

        Color bc = isFocused ? static_cast<Color>(focusBorderColor)
                 : isHovered ? static_cast<Color>(borderCol).lighten(0.3f)
                 : static_cast<Color>(borderCol);
        float bw = isFocused ? 2.0f : 1.0f;
        Path border;
        border.rect(bounds, CornerRadius(rad));
        ctx.setFillStyle(FillStyle::none());
        ctx.setStrokeStyle(StrokeStyle::solid(bc, bw));
        ctx.drawPath(border);

        float fs = fontSize;
        std::string val = value;
        ctx.setTextStyle(TextStyle::regular("default", fs));

        float textX = bounds.x + pad;
        float textY = bounds.y + pad + fs;
        float lineHeight = fs * 1.4f;

        if (val.empty()) {
            std::string ph = placeholder;
            if (!ph.empty()) {
                ctx.setFillStyle(FillStyle::solid(placeholderColor));
                ctx.drawText(ph, {textX, textY},
                    HorizontalAlignment::leading, VerticalAlignment::bottom);
            }
            if (isFocused) {
                auto now = std::chrono::steady_clock::now();
                float secs = std::chrono::duration<float>(now.time_since_epoch()).count();
                if (std::fmod(secs, 1.0f) < 0.5f) {
                    ctx.setStrokeStyle(StrokeStyle::solid(static_cast<Color>(textColor), 1.5f));
                    ctx.drawLine({textX, bounds.y + pad}, {textX, bounds.y + pad + fs});
                }
            }
            return;
        }

        std::vector<std::string> lines;
        std::istringstream stream(val);
        std::string line;
        while (std::getline(stream, line)) lines.push_back(line);
        if (val.back() == '\n') lines.push_back("");

        ctx.setFillStyle(FillStyle::solid(textColor));
        for (size_t i = 0; i < lines.size(); i++) {
            float y = textY + i * lineHeight - scrollY;
            if (y < bounds.y - lineHeight || y > bounds.y + bounds.height + lineHeight) continue;
            ctx.drawText(lines[i], {textX, y},
                HorizontalAlignment::leading, VerticalAlignment::bottom);
        }

        if (isFocused) {
            auto now = std::chrono::steady_clock::now();
            float secs = std::chrono::duration<float>(now.time_since_epoch()).count();
            if (std::fmod(secs, 1.0f) < 0.5f) {
                size_t lineIdx = 0, col = 0;
                getLineCol(val, caretPos, lineIdx, col);
                std::string beforeCaret = (lineIdx < lines.size() && col <= lines[lineIdx].size())
                    ? lines[lineIdx].substr(0, col) : "";
                Size cs = ctx.measureText(beforeCaret, TextStyle::regular("default", fs));
                float cx = textX + cs.width;
                float cy = bounds.y + pad + lineIdx * lineHeight - scrollY;
                ctx.setStrokeStyle(StrokeStyle::solid(static_cast<Color>(textColor), 1.0f));
                ctx.drawLine({cx, cy}, {cx, cy + fs});
            }
        }
    }

    Size preferredSize(TextMeasurement& tm) const {
        float fs = fontSize;
        float pad = areaPadding;
        float w = areaWidth;
        float minH = areaMinHeight;
        float maxH = areaMaxHeight;

        std::string val = value;
        if (val.empty()) return {w, minH};

        int lineCount = 1;
        for (char c : val) if (c == '\n') lineCount++;

        float contentH = pad * 2 + lineCount * fs * 1.4f;
        float h = std::clamp(contentH, minH, maxH);
        return {w, h};
    }

private:
    void getLineCol(const std::string& text, size_t pos, size_t& line, size_t& col) const {
        line = 0; col = 0;
        for (size_t i = 0; i < pos && i < text.size(); i++) {
            if (text[i] == '\n') { line++; col = 0; }
            else col++;
        }
    }

    void moveCaretVertically(const std::string& text, int dir) const {
        size_t lineIdx = 0, col = 0;
        getLineCol(text, caretPos, lineIdx, col);
        int targetLine = static_cast<int>(lineIdx) + dir;
        if (targetLine < 0) { caretPos = 0; return; }

        std::vector<size_t> lineStarts = {0};
        for (size_t i = 0; i < text.size(); i++) {
            if (text[i] == '\n') lineStarts.push_back(i + 1);
        }

        if (static_cast<size_t>(targetLine) >= lineStarts.size()) {
            caretPos = text.size();
            return;
        }

        size_t ls = lineStarts[targetLine];
        size_t le = (static_cast<size_t>(targetLine + 1) < lineStarts.size())
            ? lineStarts[targetLine + 1] - 1 : text.size();
        size_t lineLen = le - ls;
        caretPos = ls + std::min(col, lineLen);
    }
};

} // namespace flux
