#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Core/OverlayManager.hpp>
#include <Flux/Core/ControlMetrics.hpp>
#include <Flux/Views/HStack.hpp>
#include <Flux/Views/Text.hpp>
#include <Flux/Views/MenuView.hpp>
#include <Flux/Views/MenuRow.hpp>
#include <string>
#include <vector>
#include <functional>
#include <Flux/Core/Typography.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <algorithm>

namespace flux {

struct SelectInput {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;

    mutable Property<int> selectedIndex = 0;
    Property<std::vector<std::string>> options;
    /** Parallel to `options`; UTF-8 icon/emoji per row. Empty entries reserve the icon column for alignment. */
    Property<std::vector<std::string>> optionIcons;
    Property<float> selectWidth = 200.0f;
    std::function<void(int, const std::string&)> onSelect;

    Property<Color> bgColor = Color(0.16f, 0.16f, 0.16f);
    Property<Color> dropdownBgColor = Color(0.12f, 0.12f, 0.12f);
    Property<Color> borderColor_ = Color(0.22f, 0.22f, 0.22f);
    Property<Color> textColor_ = Color(0.92f, 0.92f, 0.92f);
    Property<Color> mutedColor = Color(0.48f, 0.48f, 0.48f);
    Property<Color> accentColor = Colors::blue;
    Property<float> itemFontSize = ControlMetrics::kStandardFontSize;

    mutable bool isOpen = false;
    mutable Rect lastBounds_{};
    mutable std::string overlayId_;

    void transferState(const SelectInput& old) {
        isOpen = old.isOpen;
        lastBounds_ = old.lastBounds_;
        overlayId_ = old.overlayId_;
    }

    void init() {
        focusable = true;
        cursor = CursorType::Pointer;

        static int nextId = 0;
        overlayId_ = "select-input-" + std::to_string(nextId++);

        onClick = [this]() {
            isOpen = !isOpen;
            if (isOpen) {
                showSelectOverlay();
            } else {
                hideOverlay(overlayId_);
            }
            requestApplicationRedraw();
        };
    }

    bool handleKeyDown(const KeyEvent& event) {
        if (event.key == Key::Escape && isOpen) {
            isOpen = false;
            hideOverlay(overlayId_);
            requestApplicationRedraw();
            return true;
        }

        std::vector<std::string> opts = options;
        int idx = selectedIndex;

        if (event.key == Key::Up && idx > 0) {
            idx--;
            selectedIndex = idx;
            if (onSelect) onSelect(idx, opts[idx]);
            return true;
        }
        if (event.key == Key::Down && idx < static_cast<int>(opts.size()) - 1) {
            idx++;
            selectedIndex = idx;
            if (onSelect) onSelect(idx, opts[idx]);
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
        std::vector<std::string> opts = options;
        std::vector<std::string> icons = optionIcons;
        int idx = selectedIndex;

        std::string currentLabel = (idx >= 0 && idx < static_cast<int>(opts.size()))
            ? opts[idx] : "";

        std::string iconStr;
        if (idx >= 0 && idx < static_cast<int>(icons.size())) {
            iconStr = icons[static_cast<size_t>(idx)];
        }

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
                    .minWidth = kOptionIconSlotWidth,
                    .maxWidth = kOptionIconSlotWidth,
                    .justifyContent = JustifyContent::center,
                    .alignItems = AlignItems::center,
                    .children = {
                        Text{
                            .value = iconStr,
                            .fontSize = itemFontSize,
                            .color = textColor_,
                            .horizontalAlignment = HorizontalAlignment::center
                        }
                    }
                },
                Text{
                    .value = currentLabel,
                    .fontSize = itemFontSize,
                    .color = textColor_,
                    .horizontalAlignment = HorizontalAlignment::leading,
                    .expansionBias = 1.0f,
                    .truncateTail = true
                },
                HStack{
                    .spacing = 0.0f,
                    .minWidth = kOptionTickColumnWidth,
                    .maxWidth = kOptionTickColumnWidth,
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
    static constexpr float kOptionIconSlotWidth = 20.0f;
    static constexpr float kOptionTickColumnWidth = 18.0f;
    static constexpr float kRowHSpacing = 8.0f;
    static constexpr float kCaretAfterTickGap = 8.0f;

    float minContentWidth(TextMeasurement& tm) const {
        TextStyle itemSt = makeTextStyle("default", FontWeight::regular, static_cast<float>(itemFontSize),
            Typography::lineHeightTight,
            Typography::trackingFor(static_cast<float>(itemFontSize), FontWeight::regular));

        float maxCol = 0.0f;
        std::vector<std::string> opts = options;
        for (const auto& opt : opts) {
            maxCol = std::max(maxCol, tm.measureText(opt, itemSt).width);
        }

        float arrowW = std::max(
            tm.measureText(std::string("\xE2\x96\xBE"), itemSt).width,
            tm.measureText(std::string("\xE2\x96\xB4"), itemSt).width);

        const float rowHPad = ControlMetrics::kStandardPadding.horizontal();
        float w = rowHPad + kOptionIconSlotWidth + kRowHSpacing + maxCol + kRowHSpacing + kOptionTickColumnWidth
            + kCaretAfterTickGap + arrowW;
        float floorW = static_cast<float>(selectWidth);
        if (floorW > 0.5f) {
            w = std::max(w, floorW);
        }
        return w;
    }

    void showSelectOverlay() const {
        std::vector<std::string> opts = options;
        std::vector<std::string> icons = optionIcons;
        int idx = selectedIndex;
        float anchorW = lastBounds_.width;
        float fallbackW = static_cast<float>(selectWidth);
        float overlayW = (anchorW > 0.5f) ? anchorW : fallbackW;

        std::vector<View> optViews;
        for (int i = 0; i < static_cast<int>(opts.size()); i++) {
            bool rowSelected = (i == idx);
            int capturedIdx = i;
            std::string capturedLabel = opts[i];
            std::string iconStr;
            if (i < static_cast<int>(icons.size())) {
                iconStr = icons[static_cast<size_t>(i)];
            }

            View iconSlot = HStack{
                .spacing = 0.0f,
                .minWidth = kOptionIconSlotWidth,
                .maxWidth = kOptionIconSlotWidth,
                .justifyContent = JustifyContent::center,
                .alignItems = AlignItems::center,
                .children = {
                    Text{
                        .value = iconStr,
                        .fontSize = itemFontSize,
                        .color = textColor_,
                        .horizontalAlignment = HorizontalAlignment::center
                    }
                }
            };

            View labelText = Text{
                .value = capturedLabel,
                .fontSize = itemFontSize,
                .color = textColor_,
                .horizontalAlignment = HorizontalAlignment::leading,
                .expansionBias = 1.0f,
                .truncateTail = true
            };

            View tickColumn = HStack{
                .spacing = 0.0f,
                .minWidth = kOptionTickColumnWidth,
                .maxWidth = kOptionTickColumnWidth,
                .justifyContent = JustifyContent::center,
                .alignItems = AlignItems::center,
                .children = {
                    Text{
                        .value = rowSelected ? std::string("\xE2\x9C\x93") : std::string(""),
                        .fontSize = itemFontSize,
                        .color = accentColor
                    }
                }
            };

            optViews.push_back(MenuRow{
                .spacing = kRowHSpacing,
                .alignItems = AlignItems::center,
                .selected = rowSelected,
                .rowBackground = dropdownBgColor,
                .selectionAccent = accentColor,
                .padding = ControlMetrics::kMenuRowPadding,
                .cursor = CursorType::Pointer,
                .onClick = [this, capturedIdx, capturedLabel]() {
                    selectedIndex = capturedIdx;
                    isOpen = false;
                    hideOverlay(overlayId_);
                    if (onSelect) onSelect(capturedIdx, capturedLabel);
                },
                .children = {std::move(iconSlot), std::move(labelText), std::move(tickColumn)}
            });
        }

        MenuView panel{
            .spacing = 1.0f,
            .backgroundColor = dropdownBgColor,
            .cornerRadius = 4.0f,
            .borderColor = borderColor_,
            .borderWidth = 1.0f,
            .minWidth = overlayW,
            .maxWidth = overlayW,
            .children = std::move(optViews)
        };

        flux::showMenuOverlay(overlayId_, View(std::move(panel)), lastBounds_, OverlayPosition::Below, [this]() {
            isOpen = false;
            requestApplicationRedraw();
        });
    }
};

} // namespace flux
