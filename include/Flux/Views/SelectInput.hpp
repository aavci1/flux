#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Views/VStack.hpp>
#include <Flux/Views/HStack.hpp>
#include <Flux/Views/Text.hpp>
#include <string>
#include <vector>
#include <functional>

namespace flux {

struct SelectInput {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;

    Property<int> selectedIndex = 0;
    Property<std::vector<std::string>> options;
    Property<float> selectWidth = 200.0f;
    std::function<void(int, const std::string&)> onSelect;

    Property<Color> bgColor = Color(0.16f, 0.16f, 0.16f);
    Property<Color> dropdownBgColor = Color(0.12f, 0.12f, 0.12f);
    Property<Color> borderColor_ = Color(0.22f, 0.22f, 0.22f);
    Property<Color> textColor_ = Color(0.92f, 0.92f, 0.92f);
    Property<Color> mutedColor = Color(0.48f, 0.48f, 0.48f);
    Property<Color> accentColor = Colors::blue;
    Property<float> itemFontSize = 13.0f;

    mutable bool isOpen = false;

    void init() {
        focusable = true;
        cursor = CursorType::Pointer;

        onClick = [this]() {
            isOpen = !isOpen;
            requestApplicationRedraw();
        };
    }

    bool handleKeyDown(const KeyEvent& event) const {
        if (event.key == Key::Escape && isOpen) {
            isOpen = false;
            requestApplicationRedraw();
            return true;
        }

        std::vector<std::string> opts = options;
        int idx = selectedIndex;

        if (event.key == Key::Up && idx > 0) {
            idx--;
            const_cast<Property<int>&>(selectedIndex) = idx;
            if (onSelect) onSelect(idx, opts[idx]);
            return true;
        }
        if (event.key == Key::Down && idx < static_cast<int>(opts.size()) - 1) {
            idx++;
            const_cast<Property<int>&>(selectedIndex) = idx;
            if (onSelect) onSelect(idx, opts[idx]);
            return true;
        }

        return false;
    }

    View body() const {
        std::vector<std::string> opts = options;
        int idx = selectedIndex;
        float w = selectWidth;

        std::string currentLabel = (idx >= 0 && idx < static_cast<int>(opts.size()))
            ? opts[idx] : "";

        std::vector<View> children;

        children.push_back(HStack{
            .spacing = 4.0f,
            .alignItems = AlignItems::center,
            .backgroundColor = bgColor,
            .padding = EdgeInsets(8, 12, 8, 12),
            .cornerRadius = 4.0f,
            .borderColor = borderColor_,
            .borderWidth = 1.0f,
            .minWidth = w,
            .children = {
                Text{
                    .value = currentLabel,
                    .fontSize = itemFontSize,
                    .color = textColor_,
                    .horizontalAlignment = HorizontalAlignment::leading,
                    .expansionBias = 1.0f
                },
                Text{
                    .value = std::string(isOpen ? "\xE2\x96\xB4" : "\xE2\x96\xBE"),
                    .fontSize = 10.0f,
                    .color = mutedColor
                }
            }
        });

        if (isOpen) {
            std::vector<View> optViews;
            for (int i = 0; i < static_cast<int>(opts.size()); i++) {
                bool selected = (i == idx);
                Color bg = selected ? static_cast<Color>(accentColor).opacity(0.15f) : static_cast<Color>(bgColor);
                int capturedIdx = i;
                std::string capturedLabel = opts[i];

                std::vector<View> itemContent;
                if (selected) {
                    itemContent.push_back(Text{
                        .value = std::string("\xE2\x9C\x93"),
                        .fontSize = 12.0f,
                        .color = accentColor
                    });
                }
                itemContent.push_back(Text{
                    .value = capturedLabel,
                    .fontSize = itemFontSize,
                    .color = textColor_,
                    .horizontalAlignment = HorizontalAlignment::leading
                });

                optViews.push_back(HStack{
                    .spacing = 8.0f,
                    .alignItems = AlignItems::center,
                    .backgroundColor = bg,
                    .padding = EdgeInsets(6, 10, 6, 10),
                    .cursor = CursorType::Pointer,
                    .onClick = [this, capturedIdx, capturedLabel]() {
                        const_cast<Property<int>&>(selectedIndex) = capturedIdx;
                        isOpen = false;
                        if (onSelect) onSelect(capturedIdx, capturedLabel);
                    },
                    .children = std::move(itemContent)
                });
            }

            children.push_back(VStack{
                .spacing = 1.0f,
                .backgroundColor = dropdownBgColor,
                .cornerRadius = 4.0f,
                .borderColor = borderColor_,
                .borderWidth = 1.0f,
                .minWidth = w,
                .children = std::move(optViews)
            });
        }

        return VStack{
            .spacing = 2.0f,
            .children = std::move(children)
        };
    }
};

} // namespace flux
