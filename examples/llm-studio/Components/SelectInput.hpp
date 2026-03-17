#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Views/VStack.hpp>
#include <Flux/Views/HStack.hpp>
#include <Flux/Views/Text.hpp>
#include "../Theme.hpp"
#include <string>
#include <vector>
#include <functional>

namespace llm_studio {

using namespace flux;

struct SelectInput {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;

    Property<int> selectedIndex = 0;
    Property<std::vector<std::string>> options;
    Property<float> selectWidth = 200.0f;
    std::function<void(int, const std::string&)> onSelect;

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

        children.push_back(View(HStack{
            .spacing = 4.0f,
            .alignItems = AlignItems::center,
            .backgroundColor = Theme::SurfaceRaised,
            .padding = EdgeInsets(8, 12, 8, 12),
            .cornerRadius = Theme::RadiusSmall,
            .borderColor = Theme::Border,
            .borderWidth = 1.0f,
            .minWidth = w,
            .children = {
                View(Text{
                    .value = currentLabel,
                    .fontSize = Theme::FontBody,
                    .color = Theme::TextPrimary,
                    .horizontalAlignment = HorizontalAlignment::leading,
                    .expansionBias = 1.0f
                }),
                View(Text{
                    .value = std::string(isOpen ? "\xE2\x96\xB4" : "\xE2\x96\xBE"),
                    .fontSize = 10.0f,
                    .color = Theme::TextMuted
                })
            }
        }));

        if (isOpen) {
            std::vector<View> optViews;
            for (int i = 0; i < static_cast<int>(opts.size()); i++) {
                bool selected = (i == idx);
                Color bg = selected ? Theme::Accent.opacity(0.15f) : Theme::SurfaceRaised;
                int capturedIdx = i;
                std::string capturedLabel = opts[i];

                std::vector<View> itemContent;
                if (selected) {
                    itemContent.push_back(View(Text{
                        .value = std::string("\xE2\x9C\x93"),
                        .fontSize = Theme::FontCaption,
                        .color = Theme::Accent
                    }));
                }
                itemContent.push_back(View(Text{
                    .value = capturedLabel,
                    .fontSize = Theme::FontBody,
                    .color = Theme::TextPrimary,
                    .horizontalAlignment = HorizontalAlignment::leading
                }));

                optViews.push_back(View(HStack{
                    .spacing = Theme::Space2,
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
                }));
            }

            children.push_back(View(VStack{
                .spacing = 1.0f,
                .backgroundColor = Theme::Surface,
                .cornerRadius = Theme::RadiusSmall,
                .borderColor = Theme::Border,
                .borderWidth = 1.0f,
                .minWidth = w,
                .children = std::move(optViews)
            }));
        }

        return View(VStack{
            .spacing = 2.0f,
            .children = std::move(children)
        });
    }
};

} // namespace llm_studio
