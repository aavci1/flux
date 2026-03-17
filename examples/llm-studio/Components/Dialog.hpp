#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Views/VStack.hpp>
#include <Flux/Views/HStack.hpp>
#include <Flux/Views/Text.hpp>
#include <Flux/Views/Button.hpp>
#include <Flux/Views/Spacer.hpp>
#include "../Theme.hpp"
#include <string>
#include <vector>
#include <functional>

namespace llm_studio {

using namespace flux;

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

    void init() {
        focusable = true;

        onMouseDown = [](float, float, int) {};
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

    View body() const {
        if (!static_cast<bool>(isVisible)) {
            return View(VStack{.visible = false});
        }

        std::vector<View> btnViews;
        btnViews.push_back(View(Spacer{}));
        for (const auto& btn : buttons) {
            Color bg = Theme::SurfaceRaised;
            if (btn.isPrimary) bg = Theme::Accent;
            if (btn.isDestructive) bg = Theme::Destructive;

            btnViews.push_back(View(Button{
                .text = btn.label,
                .backgroundColor = bg,
                .padding = EdgeInsets(8, 16, 8, 16),
                .cornerRadius = Theme::RadiusSmall,
                .onClick = btn.onClick
            }));
        }

        std::string titleStr = title;
        std::string msgStr = message;

        std::vector<View> contentViews;
        if (!titleStr.empty()) {
            contentViews.push_back(View(Text{
                .value = titleStr,
                .fontSize = Theme::FontH1,
                .fontWeight = FontWeight::bold,
                .color = Theme::TextPrimary,
                .horizontalAlignment = HorizontalAlignment::leading
            }));
        }
        if (!msgStr.empty()) {
            contentViews.push_back(View(Text{
                .value = msgStr,
                .fontSize = Theme::FontBody,
                .color = Theme::TextMuted,
                .horizontalAlignment = HorizontalAlignment::leading,
                .padding = EdgeInsets(4, 0, 12, 0)
            }));
        }
        contentViews.push_back(View(HStack{
            .spacing = Theme::Space2,
            .justifyContent = JustifyContent::end,
            .children = std::move(btnViews)
        }));

        return View(VStack{
            .backgroundColor = Color(0.0f, 0.0f, 0.0f, 0.6f),
            .children = {
                View(Spacer{}),
                View(HStack{
                    .justifyContent = JustifyContent::center,
                    .children = {
                        View(Spacer{}),
                        View(VStack{
                            .spacing = Theme::Space2,
                            .backgroundColor = Theme::Surface,
                            .padding = Theme::Space6,
                            .cornerRadius = Theme::RadiusDialog,
                            .borderColor = Theme::Border,
                            .borderWidth = 1.0f,
                            .minWidth = static_cast<float>(dialogWidth),
                            .maxWidth = static_cast<float>(dialogWidth),
                            .children = std::move(contentViews)
                        }),
                        View(Spacer{})
                    }
                }),
                View(Spacer{})
            }
        });
    }
};

} // namespace llm_studio
