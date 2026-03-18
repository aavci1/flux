#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <string>
#include <functional>
#include <chrono>
#include <algorithm>
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

    Property<float> fontSize = 14.0f;
    Property<Color> textColor = Color(0.92f, 0.92f, 0.92f);
    Property<Color> placeholderColor = Color(0.48f, 0.48f, 0.48f);
    Property<Color> bgColor = Color(0.12f, 0.12f, 0.12f);
    Property<Color> borderCol = Color(0.22f, 0.22f, 0.22f);
    Property<Color> focusBorderColor = Colors::blue;
    Property<Color> selectionColor = Colors::blue;
    Property<float> inputCornerRadius = 4.0f;
    Property<float> inputPadding = 8.0f;
    Property<float> inputWidth = 200.0f;
    Property<float> inputHeight = 36.0f;

    std::function<void(const std::string&)> onValueChange;
    std::function<void()> onReturn;

    mutable size_t caretPos = 0;
    mutable size_t selStart = 0;
    mutable size_t selEnd = 0;
    mutable bool focused = false;
    mutable float blinkTimer = 0.0f;
    mutable float scrollOffset = 0.0f;
    mutable std::shared_ptr<std::atomic<bool>> blinkRunning =
        std::make_shared<std::atomic<bool>>(false);

    void init() {
        focusable = true;
        cursor = CursorType::Text;

        onFocus = [this]() {
            focused = true;
            blinkTimer = 0.0f;
            if (!blinkRunning->exchange(true)) {
                std::thread([this]() {
                    while (*blinkRunning) {
                        auto now = std::chrono::steady_clock::now();
                        blinkTimer = std::chrono::duration<float>(
                            now.time_since_epoch()).count();
                        requestApplicationRedraw();
                        std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    }
                }).detach();
            }
        };

        onBlur = [this]() {
            focused = false;
            *blinkRunning = false;
            selStart = selEnd = caretPos;
        };

        onMouseDown = [this](float x, float, int button) {
            if (button == 0) {
                selStart = selEnd = caretPos;
            }
        };
    }

    bool handleTextInput(const TextInputEvent& event) const {
        if (static_cast<bool>(readOnly)) return false;
        std::string val = value;
        int maxLen = maxLength;

        deleteSelection(val);

        if (maxLen >= 0 && static_cast<int>(val.size() + event.text.size()) > maxLen)
            return true;

        val.insert(caretPos, event.text);
        caretPos += event.text.size();
        selStart = selEnd = caretPos;

        const_cast<Property<std::string>&>(value) = val;
        if (onValueChange) onValueChange(val);
        return true;
    }

    bool handleKeyDown(const KeyEvent& event) const {
        std::string val = value;

        if (event.key == Key::Backspace) {
            if (static_cast<bool>(readOnly)) return true;
            if (selStart != selEnd) {
                deleteSelection(val);
            } else if (caretPos > 0) {
                val.erase(caretPos - 1, 1);
                caretPos--;
            }
            selStart = selEnd = caretPos;
            const_cast<Property<std::string>&>(value) = val;
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
            const_cast<Property<std::string>&>(value) = val;
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

        ctx.setFillStyle(FillStyle::solid(bgColor));
        ctx.setStrokeStyle(StrokeStyle::none());
        ctx.drawRect(bounds, CornerRadius(rad));

        Color bc = isFocused ? static_cast<Color>(focusBorderColor) : static_cast<Color>(borderCol);
        float bw = isFocused ? 2.0f : 1.0f;
        Path border;
        border.rect(bounds, CornerRadius(rad));
        ctx.setFillStyle(FillStyle::none());
        ctx.setStrokeStyle(StrokeStyle::solid(bc, bw));
        ctx.drawPath(border);

        Rect textArea = {bounds.x + pad, bounds.y, bounds.width - pad * 2, bounds.height};
        float fs = fontSize;
        std::string val = value;
        std::string displayText;

        if (static_cast<bool>(password)) {
            displayText = std::string(val.size(), '\xE2');
        } else {
            displayText = val;
        }

        ctx.setTextStyle(TextStyle::regular("default", fs));

        if (val.empty() && !isFocused) {
            std::string ph = placeholder;
            if (!ph.empty()) {
                ctx.setFillStyle(FillStyle::solid(placeholderColor));
                ctx.drawText(ph, {textArea.x, bounds.center().y},
                    HorizontalAlignment::leading, VerticalAlignment::center);
            }
            return;
        }

        if (val.empty() && isFocused) {
            std::string ph = placeholder;
            if (!ph.empty()) {
                ctx.setFillStyle(FillStyle::solid(static_cast<Color>(placeholderColor).opacity(0.4f)));
                ctx.drawText(ph, {textArea.x, bounds.center().y},
                    HorizontalAlignment::leading, VerticalAlignment::center);
            }
        }

        if (selStart != selEnd && isFocused) {
            size_t sMin = std::min(selStart, selEnd);
            size_t sMax = std::max(selStart, selEnd);
            std::string beforeSel = displayText.substr(0, sMin);
            std::string selText = displayText.substr(sMin, sMax - sMin);
            Size beforeSize = ctx.measureText(beforeSel, TextStyle::regular("default", fs));
            Size selSize = ctx.measureText(selText, TextStyle::regular("default", fs));
            Rect selRect = {textArea.x + beforeSize.width, bounds.y + 4,
                           selSize.width, bounds.height - 8};
            ctx.setFillStyle(FillStyle::solid(static_cast<Color>(selectionColor).opacity(0.3f)));
            ctx.setStrokeStyle(StrokeStyle::none());
            ctx.drawRect(selRect, CornerRadius(2));
        }

        ctx.setFillStyle(FillStyle::solid(textColor));
        ctx.drawText(displayText, {textArea.x, bounds.center().y},
            HorizontalAlignment::leading, VerticalAlignment::center);

        if (isFocused) {
            float phase = std::fmod(blinkTimer * 2.0f, 2.0f);
            if (phase < 1.0f) {
                std::string beforeCaret = displayText.substr(0, caretPos);
                Size caretSize = ctx.measureText(beforeCaret, TextStyle::regular("default", fs));
                float cx = textArea.x + caretSize.width;
                ctx.setFillStyle(FillStyle::none());
                ctx.setStrokeStyle(StrokeStyle::solid(static_cast<Color>(textColor), 1.5f));
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
