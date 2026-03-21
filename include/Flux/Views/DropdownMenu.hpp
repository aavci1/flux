#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/OverlayManager.hpp>
#include <Flux/Views/VStack.hpp>
#include <Flux/Views/HStack.hpp>
#include <Flux/Views/Text.hpp>
#include <Flux/Views/Button.hpp>
#include <string>
#include <vector>
#include <functional>
#include <Flux/Core/Typography.hpp>

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
    mutable Property<bool> open = false;
    Property<std::vector<DropdownMenuItem>> items;
    Property<float> menuWidth = 180.0f;

    Property<Color> bgColor = Color(0.16f, 0.16f, 0.16f);
    Property<Color> dropdownBgColor = Color(0.12f, 0.12f, 0.12f);
    Property<Color> borderColor_ = Color(0.22f, 0.22f, 0.22f);
    Property<Color> textColor_ = Color(0.92f, 0.92f, 0.92f);
    Property<Color> mutedColor = Color(0.48f, 0.48f, 0.48f);
    Property<float> itemFontSize = Typography::callout;
    Property<float> subtitleFontSize = Typography::subheadline;

    mutable Rect lastBounds_{};
    mutable std::string overlayId_;

    void init() {
        focusable = true;
        cursor = CursorType::Pointer;

        static int nextId = 0;
        overlayId_ = "dropdown-menu-" + std::to_string(nextId++);

        onClick = [this]() {
            bool isOpen = !static_cast<bool>(open);
            open = isOpen;
            if (isOpen) {
                showDropdownOverlay();
            } else {
                hideOverlay(overlayId_);
            }
        };
    }

    bool handleKeyDown(const KeyEvent& event) {
        if (event.key == Key::Escape && static_cast<bool>(open)) {
            open = false;
            hideOverlay(overlayId_);
            return true;
        }
        return false;
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        lastBounds_ = ctx.getCurrentViewGlobalBounds();
    }

    View body() const {
        std::string lbl = label;
        bool isOpen = open;

        return HStack{
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
                    .fontSize = Typography::caption,
                    .color = mutedColor
                }
            }
        };
    }

private:
    void showDropdownOverlay() const {
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
                    open = false;
                    hideOverlay(overlayId_);
                } : std::function<void()>(nullptr),
                .children = std::move(itemContent)
            });
        }

        View dropdown = VStack{
            .spacing = 1.0f,
            .backgroundColor = dropdownBgColor,
            .cornerRadius = 8.0f,
            .borderColor = borderColor_,
            .borderWidth = 1.0f,
            .minWidth = static_cast<float>(menuWidth),
            .children = std::move(itemViews)
        };

        flux::showOverlay(overlayId_, std::move(dropdown), lastBounds_, {
            .position = OverlayPosition::Below,
            .dismissOnClickOutside = true,
            .onDismiss = [this]() {
                open = false;
                requestApplicationRedraw();
            }
        });
    }
};

} // namespace flux
