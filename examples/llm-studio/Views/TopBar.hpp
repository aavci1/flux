#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Views/HStack.hpp>
#include <Flux/Views/Text.hpp>
#include <Flux/Views/Button.hpp>
#include <Flux/Views/Spacer.hpp>
#include "../Theme.hpp"
#include "../AppState.hpp"
#include <string>

namespace llm_studio {

using namespace flux;

struct NavTab {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;

    Property<std::string> label = std::string("");
    Property<bool> active = false;

    void init() {
        cursor = CursorType::Pointer;
        focusable = true;
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        bool isActive = active;
        Color bg = isActive ? Theme::Accent.opacity(0.15f) : Colors::transparent;
        Color textCol = isActive ? Theme::Accent : Theme::TextMuted;
        float rad = Theme::RadiusSmall;

        ctx.setFillStyle(FillStyle::solid(bg));
        ctx.setStrokeStyle(StrokeStyle::none());
        ctx.drawRect(bounds, CornerRadius(rad));

        if (isActive) {
            Rect indicator = {bounds.x + 4, bounds.y + bounds.height - 2,
                             bounds.width - 8, 2};
            ctx.setFillStyle(FillStyle::solid(Theme::Accent));
            ctx.drawRect(indicator, CornerRadius(1));
        }

        std::string lbl = label;
        ctx.setTextStyle(TextStyle::regular("default", Theme::FontBody));
        ctx.setFillStyle(FillStyle::solid(textCol));
        ctx.drawText(lbl, bounds.center(),
            HorizontalAlignment::center, VerticalAlignment::center);
    }

    Size preferredSize(TextMeasurement& tm) const {
        std::string lbl = label;
        Size ts = tm.measureText(lbl, TextStyle::regular("default", Theme::FontBody));
        return {ts.width + 24.0f, 36.0f};
    }
};

struct TopBarView {
    FLUX_VIEW_PROPERTIES;

    AppState* state = nullptr;

    View body() const {
        if (!state) return View(HStack{});

        AppView current = state->currentView;

        return View(HStack{
            .spacing = 0.0f,
            .alignItems = AlignItems::center,
            .backgroundColor = Theme::Surface,
            .padding = EdgeInsets(Theme::Space1, Theme::Space4, Theme::Space1, Theme::Space4),
            .borderColor = Theme::Border,
            .borderWidth = 1.0f,
            .minHeight = 48.0f,
            .maxHeight = 48.0f,
            .children = {
                View(Text{
                    .value = std::string("LLM Studio"),
                    .fontSize = Theme::FontH1,
                    .fontWeight = FontWeight::bold,
                    .color = Theme::TextPrimary,
                    .padding = EdgeInsets(0, Theme::Space4, 0, 0)
                }),

                View(Spacer{}),

                View(HStack{
                    .spacing = Theme::Space1,
                    .alignItems = AlignItems::center,
                    .children = {
                        View(NavTab{
                            .label = std::string("Chat"),
                            .active = current == AppView::CHAT,
                            .onClick = [this]() { state->currentView = AppView::CHAT; }
                        }),
                        View(NavTab{
                            .label = std::string("Image"),
                            .active = current == AppView::IMAGE,
                            .onClick = [this]() { state->currentView = AppView::IMAGE; }
                        }),
                        View(NavTab{
                            .label = std::string("Hub"),
                            .active = current == AppView::HUB,
                            .onClick = [this]() { state->currentView = AppView::HUB; }
                        })
                    }
                }),

                View(Spacer{}),

                View(Button{
                    .text = std::string("\xE2\x9A\x99"),
                    .backgroundColor = current == AppView::SETTINGS
                        ? Theme::Accent.opacity(0.15f)
                        : Colors::transparent,
                    .padding = EdgeInsets(8, 12, 8, 12),
                    .cornerRadius = Theme::RadiusSmall,
                    .onClick = [this]() { state->currentView = AppView::SETTINGS; }
                })
            }
        });
    }
};

} // namespace llm_studio
