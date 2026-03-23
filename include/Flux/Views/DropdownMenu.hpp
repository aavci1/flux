#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/OverlayManager.hpp>
#include <Flux/Views/VStack.hpp>
#include <Flux/Views/HStack.hpp>
#include <Flux/Views/Text.hpp>
#include <string>
#include <vector>
#include <functional>
#include <Flux/Core/Typography.hpp>
#include <Flux/Core/ControlMetrics.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <Flux/Views/MenuView.hpp>
#include <Flux/Views/MenuRow.hpp>
#include <algorithm>

namespace flux {

struct DropdownMenuItem {
    /** UTF-8 text or emoji shown in a fixed left column; empty still reserves column width. */
    std::string icon;
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
    /** Row with this index shows a checkmark on the right; -1 hides all checkmarks. */
    mutable Property<int> selectedIndex = -1;

    Property<Color> bgColor = Color(0.16f, 0.16f, 0.16f);
    Property<Color> dropdownBgColor = Color(0.12f, 0.12f, 0.12f);
    Property<Color> borderColor_ = Color(0.22f, 0.22f, 0.22f);
    Property<Color> textColor_ = Color(0.92f, 0.92f, 0.92f);
    Property<Color> mutedColor = Color(0.48f, 0.48f, 0.48f);
    Property<Color> accentColor = Colors::blue;
    Property<float> itemFontSize = ControlMetrics::kStandardFontSize;
    Property<float> subtitleFontSize = ControlMetrics::kStandardFontSize;

    mutable Rect lastBounds_{};
    mutable std::string overlayId_;

    void transferState(const DropdownMenu& old) {
        open = static_cast<bool>(old.open);
        selectedIndex = static_cast<int>(old.selectedIndex);
        lastBounds_ = old.lastBounds_;
        overlayId_ = old.overlayId_;
    }

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

    Size preferredSize(TextMeasurement& tm) const {
        float w = minContentWidth(tm);
        View b = body();
        float h = b.isValid() ? b.preferredSize(tm).height : 0.0f;
        return {w, h};
    }

    View body() const {
        std::string lbl = label;
        bool isOpen = open;
        std::vector<DropdownMenuItem> menuItems = items;
        int sel = static_cast<int>(selectedIndex);
        std::string iconStr;
        if (sel >= 0 && sel < static_cast<int>(menuItems.size())) {
            iconStr = menuItems[static_cast<size_t>(sel)].icon;
        }

        Color textCol = textColor_;
        Color iconCol = textColor_;

        return HStack{
            .spacing = kRowHSpacing,
            .alignItems = AlignItems::center,
            .backgroundColor = bgColor,
            .padding = ControlMetrics::kStandardPadding,
            .cornerRadius = 4.0f,
            .borderColor = borderColor_,
            .borderWidth = 1.0f,
            .minHeight = ControlMetrics::kStandardMinHeight,
            .children = {
                HStack{
                    .spacing = 0.0f,
                    .minWidth = kItemIconSlotWidth,
                    .maxWidth = kItemIconSlotWidth,
                    .justifyContent = JustifyContent::center,
                    .alignItems = AlignItems::center,
                    .children = {
                        Text{
                            .value = iconStr,
                            .fontSize = itemFontSize,
                            .color = iconCol,
                            .horizontalAlignment = HorizontalAlignment::center
                        }
                    }
                },
                Text{
                    .value = lbl,
                    .fontSize = itemFontSize,
                    .color = textCol,
                    .horizontalAlignment = HorizontalAlignment::leading,
                    .expansionBias = 1.0f,
                    .truncateTail = true
                },
                HStack{
                    .spacing = 0.0f,
                    .minWidth = kItemTickColumnWidth,
                    .maxWidth = kItemTickColumnWidth,
                    .justifyContent = JustifyContent::center,
                    .alignItems = AlignItems::center,
                    .children = {
                        Text{
                            .value = std::string(""),
                            .fontSize = itemFontSize,
                            .color = textColor_,
                            .horizontalAlignment = HorizontalAlignment::center
                        }
                    }
                },
                Text{
                    .value = std::string(isOpen ? "\xE2\x96\xB4" : "\xE2\x96\xBE"),
                    .fontSize = itemFontSize,
                    .color = mutedColor
                }
            }
        };
    }

private:
    static constexpr float kItemIconSlotWidth = 20.0f;
    static constexpr float kItemTickColumnWidth = 18.0f;
    static constexpr float kRowHSpacing = 8.0f;
    static constexpr float kCaretAfterTickGap = 8.0f;

    float minContentWidth(TextMeasurement& tm) const {
        TextStyle itemSt = makeTextStyle("default", FontWeight::regular, static_cast<float>(itemFontSize),
            Typography::lineHeightTight,
            Typography::trackingFor(static_cast<float>(itemFontSize), FontWeight::regular));
        TextStyle subSt = makeTextStyle("default", FontWeight::regular, static_cast<float>(subtitleFontSize),
            Typography::lineHeightTight,
            Typography::trackingFor(static_cast<float>(subtitleFontSize), FontWeight::regular));

        float maxCol = tm.measureText(static_cast<std::string>(label), itemSt).width;
        std::vector<DropdownMenuItem> menuItems = items;
        for (const auto& item : menuItems) {
            float lw = tm.measureText(item.label, itemSt).width;
            float sw = item.subtitle.empty() ? 0.0f : tm.measureText(item.subtitle, subSt).width;
            maxCol = std::max(maxCol, std::max(lw, sw));
        }

        float arrowW = std::max(
            tm.measureText(std::string("\xE2\x96\xBE"), itemSt).width,
            tm.measureText(std::string("\xE2\x96\xB4"), itemSt).width);

        const float rowHPad = ControlMetrics::kStandardPadding.horizontal();
        return rowHPad + kItemIconSlotWidth + kRowHSpacing + maxCol + kRowHSpacing + kItemTickColumnWidth
            + kCaretAfterTickGap + arrowW;
    }

    void showDropdownOverlay() const {
        std::vector<DropdownMenuItem> menuItems = items;
        int selIdx = static_cast<int>(selectedIndex);
        float anchorW = lastBounds_.width;
        float overlayW = (anchorW > 0.5f) ? anchorW : 180.0f;

        std::vector<View> itemViews;
        for (int i = 0; i < static_cast<int>(menuItems.size()); ++i) {
            const auto& item = menuItems[static_cast<size_t>(i)];
            Color textCol = item.enabled ? static_cast<Color>(textColor_) : static_cast<Color>(mutedColor).opacity(0.4f);
            Color iconCol = item.enabled ? static_cast<Color>(textColor_) : static_cast<Color>(mutedColor).opacity(0.4f);
            bool rowSelected = (selIdx >= 0 && i == selIdx);

            std::vector<View> labelChildren;
            labelChildren.push_back(Text{
                .value = item.label,
                .fontSize = itemFontSize,
                .color = textCol,
                .horizontalAlignment = HorizontalAlignment::leading,
                .expansionBias = 1.0f,
                .truncateTail = true
            });
            if (!item.subtitle.empty()) {
                labelChildren.push_back(Text{
                    .value = item.subtitle,
                    .fontSize = subtitleFontSize,
                    .color = mutedColor,
                    .horizontalAlignment = HorizontalAlignment::leading,
                    .expansionBias = 1.0f,
                    .truncateTail = true
                });
            }

            View labelColumn = VStack{
                .spacing = 4.0f,
                .alignItems = AlignItems::stretch,
                .expansionBias = 1.0f,
                .children = std::move(labelChildren)
            };

            View iconSlot = HStack{
                .spacing = 0.0f,
                .minWidth = kItemIconSlotWidth,
                .maxWidth = kItemIconSlotWidth,
                .justifyContent = JustifyContent::center,
                .alignItems = AlignItems::center,
                .children = {
                    Text{
                        .value = item.icon,
                        .fontSize = itemFontSize,
                        .color = iconCol,
                        .horizontalAlignment = HorizontalAlignment::center
                    }
                }
            };

            bool showTick = (selIdx >= 0 && i == selIdx);
            View tickColumn = HStack{
                .spacing = 0.0f,
                .minWidth = kItemTickColumnWidth,
                .maxWidth = kItemTickColumnWidth,
                .justifyContent = JustifyContent::center,
                .alignItems = AlignItems::center,
                .children = {
                    Text{
                        .value = showTick ? std::string("\xE2\x9C\x93") : std::string(""),
                        .fontSize = itemFontSize,
                        .color = accentColor
                    }
                }
            };

            auto cb = item.onClick;
            bool en = item.enabled;
            int capturedIdx = i;
            itemViews.push_back(MenuRow{
                .spacing = kRowHSpacing,
                .alignItems = AlignItems::center,
                .selected = rowSelected,
                .enabled = en,
                .rowBackground = bgColor,
                .selectionAccent = accentColor,
                .padding = ControlMetrics::kMenuRowPadding,
                .cursor = en ? std::optional(CursorType::Pointer) : std::nullopt,
                .onClick = en ? [this, capturedIdx, cb]() {
                    selectedIndex = capturedIdx;
                    if (cb) cb();
                    open = false;
                    hideOverlay(overlayId_);
                } : std::function<void()>(nullptr),
                .children = {std::move(iconSlot), std::move(labelColumn), std::move(tickColumn)}
            });
        }

        MenuView panel{
            .spacing = 4.0f,
            .backgroundColor = bgColor,
            .cornerRadius = 8.0f,
            .borderColor = borderColor_,
            .borderWidth = 2.0f,
            .minWidth = overlayW,
            .maxWidth = overlayW,
            .children = std::move(itemViews)
        };

        flux::showMenuOverlay(overlayId_, View(std::move(panel)), lastBounds_, OverlayPosition::Below, [this]() {
            open = false;
            requestApplicationRedraw();
        });
    }
};

} // namespace flux
