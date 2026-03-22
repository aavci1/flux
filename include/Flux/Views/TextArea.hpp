#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Core/Typography.hpp>
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
    Property<float> fontSize = Typography::body;
    Property<float> areaMinHeight = 72.0f;
    Property<float> areaMaxHeight = 200.0f;
    Property<bool> autoExpand = true;
    Property<Color> textColor = Colors::inherit;
    Property<Color> bgColor = Colors::inherit;
    Property<Color> borderCol = Colors::inherit;
    Property<Color> focusBorderColor = Colors::inherit;
    Property<Color> placeholderColor = Colors::inherit;
    Property<float> areaCornerRadius = 4.0f;
    Property<float> areaPadding = 10.0f;
    Property<float> areaWidth = 400.0f;

    std::function<void(const std::string&)> onValueChange;
    std::function<void()> onSubmit;

    mutable size_t caretPos = std::string::npos;
    mutable float scrollY = 0.0f;

    void transferState(const TextArea& old) {
        caretPos = old.caretPos;
        scrollY = old.scrollY;
    }

    void init() {
        focusable = true;
        cursor = CursorType::Text;

        onScroll = [this](float, float, float, float dy) {
            scrollY = std::max(0.0f, scrollY + dy);
        };
    }

    bool handleTextInput(const TextInputEvent& event) {
        if (static_cast<bool>(readOnly)) return false;
        std::string val = value;
        if (caretPos > val.size()) caretPos = val.size();
        val.insert(caretPos, event.text);
        caretPos += event.text.size();
        value = val;
        if (onValueChange) onValueChange(val);
        return true;
    }

    bool handleKeyDown(const KeyEvent& event) {
        std::string val = value;
        if (caretPos > val.size()) caretPos = val.size();

        if (event.key == Key::Enter) {
            if (event.hasCtrl() || event.hasSuper()) {
                if (onSubmit) onSubmit();
                return true;
            }
            if (!static_cast<bool>(readOnly)) {
                val.insert(caretPos, "\n");
                caretPos++;
                value = val;
                if (onValueChange) onValueChange(val);
            }
            return true;
        }

        if (event.key == Key::Backspace && !static_cast<bool>(readOnly)) {
            if (caretPos > 0) {
                val.erase(caretPos - 1, 1);
                caretPos--;
                value = val;
                if (onValueChange) onValueChange(val);
            }
            return true;
        }

        if (event.key == Key::Delete && !static_cast<bool>(readOnly)) {
            if (caretPos < val.size()) {
                val.erase(caretPos, 1);
                value = val;
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
        const auto& th = ctx.theme();

        Color bg = resolveColor(bgColor, th.inputBackground);
        Color border = resolveColor(borderCol, th.borderStrong);
        Color focus = resolveColor(focusBorderColor, th.focusRing);
        Color text = resolveColor(textColor, th.inputForeground);
        Color ph = resolveColor(placeholderColor, th.placeholder);

        ViewHelpers::drawInputFieldChrome(ctx, bounds, bg, border, focus, rad);

        float fs = fontSize;
        std::string val = value;
        if (caretPos > val.size()) caretPos = val.size();
        const TextStyle textStyle = makeTextStyle("default", FontWeight::regular, fs,
            Typography::lineHeightBody, Typography::trackingFor(fs, FontWeight::regular));
        ctx.setTextStyle(textStyle);

        float textX = bounds.x + pad;
        float textY = bounds.y + pad + fs;
        float lineHeight = fs * Typography::lineHeightBody;

        if (val.empty()) {
            std::string phText = placeholder;
            if (!phText.empty()) {
                ctx.setFillStyle(FillStyle::solid(ph));
                ctx.drawText(phText, {textX, textY},
                    HorizontalAlignment::leading, VerticalAlignment::bottom);
            }
            if (isFocused) {
                auto now = std::chrono::steady_clock::now();
                float secs = std::chrono::duration<float>(now.time_since_epoch()).count();
                if (std::fmod(secs, 1.0f) < 0.5f) {
                    ctx.setStrokeStyle(StrokeStyle::solid(text, 1.5f));
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

        ctx.setFillStyle(FillStyle::solid(text));
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
                Size cs = ctx.measureText(beforeCaret, textStyle);
                float cx = textX + cs.width;
                float cy = bounds.y + pad + lineIdx * lineHeight - scrollY;
                ctx.setStrokeStyle(StrokeStyle::solid(text, 1.0f));
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

        float contentH = pad * 2 + lineCount * fs * Typography::lineHeightBody;
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
