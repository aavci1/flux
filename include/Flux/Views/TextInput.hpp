#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Core/Typography.hpp>
#include <string>
#include <functional>
#include <algorithm>
#include <chrono>
#include <cmath>

namespace flux {

struct TextInput {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;

    Property<std::string> value = std::string("");
    Property<std::string> placeholder = std::string("");
    Property<bool> password = false;
    Property<bool> readOnly = false;
    Property<int> maxLength = -1;

    Property<float> fontSize = Typography::body;
    Property<Color> textColor = Colors::inherit;
    Property<Color> placeholderColor = Colors::inherit;
    Property<Color> bgColor = Colors::inherit;
    Property<Color> borderCol = Colors::inherit;
    Property<Color> focusBorderColor = Colors::inherit;
    Property<Color> selectionColor = Colors::inherit;
    Property<float> inputCornerRadius = 4.0f;
    Property<float> inputPadding = 8.0f;
    Property<float> inputWidth = 200.0f;
    Property<float> inputHeight = 36.0f;

    std::function<void(const std::string&)> onValueChange;
    std::function<void()> onReturn;

    mutable size_t caretPos = std::string::npos;
    mutable size_t selStart = std::string::npos;
    mutable size_t selEnd = std::string::npos;
    mutable float scrollOffset = 0.0f;

    void init() {
        focusable = true;
        cursor = CursorType::Text;

        onMouseDown = [this](float, float, int button) {
            if (button == 0) {
                selStart = selEnd = caretPos;
            }
        };
    }

    bool handleTextInput(const TextInputEvent& event) {
        if (static_cast<bool>(readOnly)) return false;
        std::string val = value;
        int maxLen = maxLength;

        if (caretPos > val.size()) caretPos = val.size();
        if (selStart > val.size()) selStart = val.size();
        if (selEnd > val.size()) selEnd = val.size();

        deleteSelection(val);

        if (maxLen >= 0 && static_cast<int>(val.size() + event.text.size()) > maxLen)
            return true;

        val.insert(caretPos, event.text);
        caretPos += event.text.size();
        selStart = selEnd = caretPos;

        value = val;
        if (onValueChange) onValueChange(val);
        return true;
    }

    bool handleKeyDown(const KeyEvent& event) {
        std::string val = value;

        if (caretPos > val.size()) caretPos = val.size();
        if (selStart > val.size()) selStart = val.size();
        if (selEnd > val.size()) selEnd = val.size();

        if (event.key == Key::Backspace) {
            if (static_cast<bool>(readOnly)) return true;
            if (selStart != selEnd) {
                deleteSelection(val);
            } else if (caretPos > 0) {
                val.erase(caretPos - 1, 1);
                caretPos--;
            }
            selStart = selEnd = caretPos;
            value = val;
            if (onValueChange) onValueChange(val);
            return true;
        }

        if (event.key == Key::Delete) {
            if (static_cast<bool>(readOnly)) return true;
            if (selStart != selEnd) {
                deleteSelection(val);
            } else if (caretPos < val.size()) {
                val.erase(caretPos, 1);
            }
            selStart = selEnd = caretPos;
            value = val;
            if (onValueChange) onValueChange(val);
            return true;
        }

        if (event.key == Key::Left) {
            if (caretPos > 0) caretPos--;
            if (!event.hasShift()) selStart = selEnd = caretPos;
            else selEnd = caretPos;
            return true;
        }

        if (event.key == Key::Right) {
            if (caretPos < val.size()) caretPos++;
            if (!event.hasShift()) selStart = selEnd = caretPos;
            else selEnd = caretPos;
            return true;
        }

        if (event.key == Key::Home) {
            caretPos = 0;
            if (!event.hasShift()) selStart = selEnd = 0;
            else selEnd = 0;
            return true;
        }

        if (event.key == Key::End) {
            caretPos = val.size();
            if (!event.hasShift()) selStart = selEnd = caretPos;
            else selEnd = caretPos;
            return true;
        }

        if (event.key == Key::Enter) {
            if (onReturn) onReturn();
            return true;
        }

        if (event.hasCtrl() && event.key == Key::A) {
            selStart = 0;
            selEnd = caretPos = val.size();
            return true;
        }

        return false;
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        float pad = inputPadding;
        float rad = inputCornerRadius;
        bool isFocused = ctx.isCurrentViewFocused();
        const auto& th = ctx.theme();

        Color bg = resolveColor(bgColor, th.inputBackground);
        Color border = resolveColor(borderCol, th.borderStrong);
        Color focus = resolveColor(focusBorderColor, th.focusRing);
        Color text = resolveColor(textColor, th.inputForeground);
        Color ph = resolveColor(placeholderColor, th.placeholder);
        Color sel = resolveColor(selectionColor, th.selection);

        ViewHelpers::drawInputFieldChrome(ctx, bounds, bg, border, focus, rad);

        Rect textArea = {bounds.x + pad, bounds.y, bounds.width - pad * 2, bounds.height};
        float fs = fontSize;
        std::string val = value;
        std::string displayText;

        // Clamp positions in case value was externally changed
        if (caretPos > val.size()) caretPos = val.size();
        if (selStart > val.size()) selStart = val.size();
        if (selEnd > val.size()) selEnd = val.size();

        if (static_cast<bool>(password)) {
            displayText = std::string(val.size(), '\xE2');
        } else {
            displayText = val;
        }

        const TextStyle textStyle = makeTextStyle("default", FontWeight::regular, fs,
            Typography::lineHeightTight, Typography::trackingFor(fs, FontWeight::regular));
        ctx.setTextStyle(textStyle);

        if (val.empty() && !isFocused) {
            std::string phText = placeholder;
            if (!phText.empty()) {
                ctx.setFillStyle(FillStyle::solid(ph));
                ctx.drawText(phText, {textArea.x, bounds.center().y},
                    HorizontalAlignment::leading, VerticalAlignment::center);
            }
            return;
        }

        if (val.empty() && isFocused) {
            std::string phText = placeholder;
            if (!phText.empty()) {
                ctx.setFillStyle(FillStyle::solid(ph.opacity(0.4f)));
                ctx.drawText(phText, {textArea.x, bounds.center().y},
                    HorizontalAlignment::leading, VerticalAlignment::center);
            }
        }

        if (selStart != selEnd && isFocused) {
            size_t sMin = std::min(selStart, selEnd);
            size_t sMax = std::max(selStart, selEnd);
            std::string beforeSel = displayText.substr(0, sMin);
            std::string selText = displayText.substr(sMin, sMax - sMin);
            Size beforeSize = ctx.measureText(beforeSel, textStyle);
            Size selSize = ctx.measureText(selText, textStyle);
            Rect selRect = {textArea.x + beforeSize.width, bounds.y + 4,
                           selSize.width, bounds.height - 8};
            ctx.setFillStyle(FillStyle::solid(sel));
            ctx.setStrokeStyle(StrokeStyle::none());
            ctx.drawRect(selRect, CornerRadius(2));
        }

        ctx.setFillStyle(FillStyle::solid(text));
        ctx.drawText(displayText, {textArea.x, bounds.center().y},
            HorizontalAlignment::leading, VerticalAlignment::center);

        if (isFocused) {
            auto now = std::chrono::steady_clock::now();
            float secs = std::chrono::duration<float>(now.time_since_epoch()).count();
            bool caretVisible = std::fmod(secs, 1.0f) < 0.5f;
            if (caretVisible) {
                std::string beforeCaret = displayText.substr(0, caretPos);
                Size caretSize = ctx.measureText(beforeCaret, textStyle);
                float cx = textArea.x + caretSize.width;
                ctx.setFillStyle(FillStyle::none());
                ctx.setStrokeStyle(StrokeStyle::solid(text, 1.5f));
                ctx.drawLine({cx, bounds.y + 6}, {cx, bounds.y + bounds.height - 6});
            }
        }
    }

    Size preferredSize(TextMeasurement&) const {
        return {static_cast<float>(inputWidth), static_cast<float>(inputHeight)};
    }

private:
    void deleteSelection(std::string& val) const {
        if (selStart == selEnd) return;
        size_t sMin = std::min(selStart, selEnd);
        size_t sMax = std::max(selStart, selEnd);
        val.erase(sMin, sMax - sMin);
        caretPos = sMin;
        selStart = selEnd = caretPos;
    }
};

} // namespace flux
