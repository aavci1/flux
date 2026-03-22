#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Core/OverlayManager.hpp>
#include <Flux/Views/VStack.hpp>
#include <Flux/Views/HStack.hpp>
#include <Flux/Views/Text.hpp>
#include <Flux/Views/Button.hpp>
#include <Flux/Views/Spacer.hpp>
#include <Flux/Core/Typography.hpp>
#include <string>
#include <vector>
#include <functional>

namespace flux {

struct DialogButton {
    std::string label;
    bool isPrimary = false;
    bool isDestructive = false;
    std::function<void()> onClick;
};

struct Dialog {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;

    Property<bool> isVisible = false;
    Property<std::string> title = std::string("");
    Property<std::string> message = std::string("");
    Property<float> dialogWidth = 400.0f;
    std::vector<DialogButton> buttons;

    Property<Color> overlayColor = Color(0.0f, 0.0f, 0.0f, 0.6f);
    Property<Color> dialogBgColor = Color(0.12f, 0.12f, 0.12f);
    Property<Color> dialogBorderColor = Color(0.22f, 0.22f, 0.22f);
    Property<Color> titleColor = Color(0.92f, 0.92f, 0.92f);
    Property<Color> messageColor = Color(0.48f, 0.48f, 0.48f);
    Property<Color> primaryColor = Colors::blue;
    Property<Color> destructiveColor = Colors::red;
    Property<Color> defaultButtonColor = Color(0.16f, 0.16f, 0.16f);
    Property<float> titleFontSize = Typography::body;
    Property<float> messageFontSize = Typography::body;

    mutable std::string overlayId_;
    mutable bool overlayShown_ = false;

    void transferState(const Dialog& old) {
        overlayId_ = old.overlayId_;
        overlayShown_ = old.overlayShown_;
    }

    void init() {
        focusable = true;
        onMouseDown = [](float, float, int) {};

        static int nextId = 0;
        overlayId_ = "dialog-" + std::to_string(nextId++);
    }

    bool handleKeyDown(const KeyEvent& event) const {
        if (!static_cast<bool>(isVisible)) return false;

        if (event.key == Key::Escape) {
            for (auto& btn : buttons) {
                if (!btn.isPrimary && btn.onClick) { btn.onClick(); return true; }
            }
            return true;
        }
        if (event.key == Key::Enter) {
            for (auto& btn : buttons) {
                if (btn.isPrimary && btn.onClick) { btn.onClick(); return true; }
            }
            return true;
        }
        return true;
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        bool vis = static_cast<bool>(isVisible);
        if (vis && !overlayShown_) {
            showDialogOverlay();
            overlayShown_ = true;
        } else if (!vis && overlayShown_) {
            hideOverlay(overlayId_);
            overlayShown_ = false;
        }
    }

    View body() const {
        return VStack{.visible = static_cast<bool>(isVisible)};
    }

private:
    void showDialogOverlay() const {
        std::vector<View> btnViews;
        btnViews.push_back(Spacer{});
        for (const auto& btn : buttons) {
            Color bg = defaultButtonColor;
            if (btn.isPrimary) bg = primaryColor;
            if (btn.isDestructive) bg = destructiveColor;

            btnViews.push_back(Button{
                .text = btn.label,
                .backgroundColor = bg,
                .padding = EdgeInsets(8, 16, 8, 16),
                .cornerRadius = 4.0f,
                .onClick = btn.onClick
            });
        }

        std::string titleStr = title;
        std::string msgStr = message;

        std::vector<View> contentViews;
        if (!titleStr.empty()) {
            contentViews.push_back(Text{
                .value = titleStr,
                .fontSize = titleFontSize,
                .fontWeight = FontWeight::bold,
                .color = titleColor,
                .horizontalAlignment = HorizontalAlignment::leading
            });
        }
        if (!msgStr.empty()) {
            contentViews.push_back(Text{
                .value = msgStr,
                .fontSize = messageFontSize,
                .lineHeightMultiplier = Typography::lineHeightBody,
                .color = messageColor,
                .horizontalAlignment = HorizontalAlignment::leading,
                .padding = EdgeInsets(4, 0, 12, 0)
            });
        }
        contentViews.push_back(HStack{
            .spacing = 8.0f,
            .justifyContent = JustifyContent::end,
            .children = std::move(btnViews)
        });

        View dialogContent = VStack{
            .children = {
                Spacer{},
                HStack{
                    .justifyContent = JustifyContent::center,
                    .children = {
                        Spacer{},
                        VStack{
                            .spacing = 8.0f,
                            .backgroundColor = dialogBgColor,
                            .padding = 24.0f,
                            .cornerRadius = 10.0f,
                            .borderColor = dialogBorderColor,
                            .borderWidth = 1.0f,
                            .minWidth = static_cast<float>(dialogWidth),
                            .maxWidth = static_cast<float>(dialogWidth),
                            .children = std::move(contentViews)
                        },
                        Spacer{}
                    }
                },
                Spacer{}
            }
        };

        flux::showOverlay(overlayId_, std::move(dialogContent), {}, {
            .position = OverlayPosition::Center,
            .dismissOnClickOutside = false,
            .modal = true,
            .backdrop = overlayColor
        });
    }
};

} // namespace flux
