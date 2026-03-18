#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Views/VStack.hpp>
#include <Flux/Views/Text.hpp>
#include <Flux/Views/Spacer.hpp>
#include "../Theme.hpp"
#include "../AppState.hpp"

namespace llm_studio {

using namespace flux;

struct IconRailButton {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;

    Property<std::string> icon = std::string("");
    Property<bool> active = false;
    Property<std::string> tooltip = std::string("");

    void init() {
        cursor = CursorType::Pointer;
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        bool isActive = active;

        if (isActive) {
            Rect indicator = {bounds.x, bounds.y + 6, 3, bounds.height - 12};
            ctx.setFillStyle(FillStyle::solid(Theme::Accent));
            ctx.drawRect(indicator, CornerRadius(1.5f));
        }

        Color iconColor = isActive ? Theme::Accent : Theme::TextMuted;
        ctx.setTextStyle(TextStyle::regular("default", 20.0f));
        ctx.setFillStyle(FillStyle::solid(iconColor));
        ctx.drawText(static_cast<std::string>(icon), bounds.center(),
            HorizontalAlignment::center, VerticalAlignment::center);
    }

    Size preferredSize(TextMeasurement&) const {
        return {48.0f, 44.0f};
    }
};

struct IconRailView {
    FLUX_VIEW_PROPERTIES;

    AppState* state = nullptr;

    View body() const {
        if (!state) return VStack{};

        AppPage page = state->currentPage;

        return VStack{
            .spacing = 4.0f,
            .backgroundColor = Color(0.06f, 0.06f, 0.06f),
            .padding = EdgeInsets(8.0f, 0.0f, 8.0f, 0.0f),
            .borderColor = Theme::Border,
            .borderWidth = 1.0f,
            .minWidth = 48.0f,
            .maxWidth = 48.0f,
            .children = {
                IconRailButton{
                    .icon = std::string("\xF0\x9F\x92\xAC"),
                    .active = page == AppPage::CHAT,
                    .tooltip = std::string("Chat"),
                    .onClick = [this]() { state->currentPage = AppPage::CHAT; }
                },
                IconRailButton{
                    .icon = std::string("\xE2\x96\xA6"),
                    .active = page == AppPage::MODELS,
                    .tooltip = std::string("Models"),
                    .onClick = [this]() { state->currentPage = AppPage::MODELS; }
                },
                Spacer{},
                IconRailButton{
                    .icon = std::string("\xE2\x9A\x99"),
                    .active = false,
                    .tooltip = std::string("Settings"),
                    .onClick = [this]() { state->showSettingsDialog = true; }
                }
            }
        };
    }
};

} // namespace llm_studio
