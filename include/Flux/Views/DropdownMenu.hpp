#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Views/VStack.hpp>
#include <Flux/Views/HStack.hpp>
#include <Flux/Views/Text.hpp>
#include <Flux/Views/Button.hpp>
#include <string>
#include <vector>
#include <functional>

namespace flux {

struct DropdownMenuItem {
    std::string label;
    std::string subtitle;
    bool enabled = true;
    std::function<void()> onClick;
};

struct DropdownMenu {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;

    Property<std::string> label = std::string("");
    Property<bool> open = false;
    Property<std::vector<DropdownMenuItem>> items;
    Property<float> menuWidth = 180.0f;

    Property<Color> bgColor = Color(0.16f, 0.16f, 0.16f);
    Property<Color> dropdownBgColor = Color(0.12f, 0.12f, 0.12f);
    Property<Color> borderColor_ = Color(0.22f, 0.22f, 0.22f);
    Property<Color> textColor_ = Color(0.92f, 0.92f, 0.92f);
    Property<Color> mutedColor = Color(0.48f, 0.48f, 0.48f);
    Property<float> itemFontSize = 14.0f;
    Property<float> subtitleFontSize = 12.0f;

    void init() {
        focusable = true;
        cursor = CursorType::Pointer;

        onClick = [this]() {
            open = !static_cast<bool>(open);
        };
    }

    bool handleKeyDown(const KeyEvent& event) const {
        if (event.key == Key::Escape && static_cast<bool>(open)) {
            const_cast<Property<bool>&>(open) = false;
            return true;
        }
        return false;
    }

    View body() const {
        std::string lbl = label;
        bool isOpen = open;

        std::vector<View> children;

        children.push_back(HStack{
            .spacing = 4.0f,
            .alignItems = AlignItems::center,
            .backgroundColor = bgColor,
            .padding = EdgeInsets(6, 12, 6, 12),
            .cornerRadius = 4.0f,
            .borderColor = borderColor_,
            .borderWidth = 1.0f,
            .children = {
                Text{
                    .value = lbl,
                    .fontSize = itemFontSize,
                    .color = textColor_,
                    .horizontalAlignment = HorizontalAlignment::leading
                },
                Text{
                    .value = std::string(isOpen ? "\xE2\x96\xB4" : "\xE2\x96\xBE"),
                    .fontSize = 10.0f,
                    .color = mutedColor
                }
            }
        });

        if (isOpen) {
            std::vector<DropdownMenuItem> menuItems = items;
            std::vector<View> itemViews;
            for (const auto& item : menuItems) {
                Color textCol = item.enabled ? static_cast<Color>(textColor_) : static_cast<Color>(mutedColor).opacity(0.4f);
                std::vector<View> itemContent;
                itemContent.push_back(Text{
                    .value = item.label,
                    .fontSize = itemFontSize,
                    .color = textCol,
                    .horizontalAlignment = HorizontalAlignment::leading
                });
                if (!item.subtitle.empty()) {
                    itemContent.push_back(Text{
                        .value = item.subtitle,
                        .fontSize = subtitleFontSize,
                        .color = mutedColor,
                        .horizontalAlignment = HorizontalAlignment::leading
                    });
                }
                auto cb = item.onClick;
                bool en = item.enabled;
                itemViews.push_back(VStack{
                    .backgroundColor = bgColor,
                    .padding = EdgeInsets(6, 10, 6, 10),
                    .cursor = en ? std::optional(CursorType::Pointer) : std::nullopt,
                    .onClick = (en && cb) ? [cb, this]() {
                        cb();
                        const_cast<Property<bool>&>(open) = false;
                    } : std::function<void()>(nullptr),
                    .children = std::move(itemContent)
                });
            }

            children.push_back(VStack{
                .spacing = 1.0f,
                .backgroundColor = dropdownBgColor,
                .cornerRadius = 8.0f,
                .borderColor = borderColor_,
                .borderWidth = 1.0f,
                .minWidth = static_cast<float>(menuWidth),
                .children = std::move(itemViews)
            });
        }

        return VStack{
            .spacing = 2.0f,
            .children = std::move(children)
        };
    }
};

} // namespace flux
