#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Views/VStack.hpp>
#include <Flux/Views/HStack.hpp>
#include <Flux/Views/Text.hpp>
#include <Flux/Views/Button.hpp>
#include "../Theme.hpp"
#include <string>
#include <vector>
#include <functional>

namespace llm_studio {

using namespace flux;

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

        children.push_back(View(HStack{
            .spacing = 4.0f,
            .alignItems = AlignItems::center,
            .backgroundColor = Theme::SurfaceRaised,
            .padding = EdgeInsets(6, 12, 6, 12),
            .cornerRadius = Theme::RadiusSmall,
            .borderColor = Theme::Border,
            .borderWidth = 1.0f,
            .children = {
                View(Text{
                    .value = lbl,
                    .fontSize = Theme::FontBody,
                    .color = Theme::TextPrimary,
                    .horizontalAlignment = HorizontalAlignment::leading
                }),
                View(Text{
                    .value = std::string(isOpen ? "\xE2\x96\xB4" : "\xE2\x96\xBE"),
                    .fontSize = 10.0f,
                    .color = Theme::TextMuted
                })
            }
        }));

        if (isOpen) {
            std::vector<DropdownMenuItem> menuItems = items;
            std::vector<View> itemViews;
            for (const auto& item : menuItems) {
                Color textCol = item.enabled ? Theme::TextPrimary : static_cast<Color>(Theme::TextMuted).opacity(0.4f);
                std::vector<View> itemContent;
                itemContent.push_back(View(Text{
                    .value = item.label,
                    .fontSize = Theme::FontBody,
                    .color = textCol,
                    .horizontalAlignment = HorizontalAlignment::leading
                }));
                if (!item.subtitle.empty()) {
                    itemContent.push_back(View(Text{
                        .value = item.subtitle,
                        .fontSize = Theme::FontCaption,
                        .color = Theme::TextMuted,
                        .horizontalAlignment = HorizontalAlignment::leading
                    }));
                }
                auto cb = item.onClick;
                bool en = item.enabled;
                itemViews.push_back(View(VStack{
                    .backgroundColor = Theme::SurfaceRaised,
                    .padding = EdgeInsets(6, 10, 6, 10),
                    .cursor = en ? std::optional(CursorType::Pointer) : std::nullopt,
                    .onClick = (en && cb) ? [cb, this]() {
                        cb();
                        const_cast<Property<bool>&>(open) = false;
                    } : std::function<void()>(nullptr),
                    .children = std::move(itemContent)
                }));
            }

            children.push_back(View(VStack{
                .spacing = 1.0f,
                .backgroundColor = Theme::Surface,
                .cornerRadius = Theme::RadiusCard,
                .borderColor = Theme::Border,
                .borderWidth = 1.0f,
                .minWidth = static_cast<float>(menuWidth),
                .children = std::move(itemViews)
            }));
        }

        return View(VStack{
            .spacing = 2.0f,
            .children = std::move(children)
        });
    }
};

} // namespace llm_studio
