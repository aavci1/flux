#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Views/VStack.hpp>
#include <Flux/Views/HStack.hpp>
#include <Flux/Views/Text.hpp>
#include <Flux/Views/Button.hpp>
#include <Flux/Views/Spacer.hpp>
#include <Flux/Views/Divider.hpp>
#include <Flux/Views/ScrollArea.hpp>
#include <Flux/Views/Badge.hpp>
#include "../Theme.hpp"
#include "../AppState.hpp"
#include <string>

namespace llm_studio {

using namespace flux;

struct ModelListItem {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;

    Property<ModelInfo> model;
    Property<bool> isActive = false;

    void init() {
        cursor = CursorType::Pointer;
    }

    View body() const {
        ModelInfo m = model;
        bool active = isActive;

        Color bg = active ? Theme::Accent.opacity(0.15f) : Colors::transparent;
        Color leftBorder = active ? Theme::Accent : Colors::transparent;

        std::string sizeStr = formatBytes(m.sizeBytes);

        return View(HStack{
            .spacing = 0.0f,
            .alignItems = AlignItems::stretch,
            .backgroundColor = bg,
            .padding = EdgeInsets(0),
            .children = {
                View(VStack{
                    .backgroundColor = leftBorder,
                    .minWidth = 3.0f,
                    .maxWidth = 3.0f
                }),
                View(VStack{
                    .spacing = 2.0f,
                    .padding = EdgeInsets(Theme::Space2, Theme::Space3, Theme::Space2, Theme::Space3),
                    .expansionBias = 1.0f,
                    .children = {
                        View(Text{
                            .value = m.name,
                            .fontSize = Theme::FontBody,
                            .fontWeight = FontWeight::medium,
                            .color = Theme::TextPrimary,
                            .horizontalAlignment = HorizontalAlignment::leading
                        }),
                        View(HStack{
                            .spacing = Theme::Space1,
                            .alignItems = AlignItems::center,
                            .children = {
                                View(Text{
                                    .value = m.quantization,
                                    .fontSize = Theme::FontCaption,
                                    .color = Theme::TextMuted,
                                    .horizontalAlignment = HorizontalAlignment::leading
                                }),
                                View(Text{
                                    .value = std::string(" \xC2\xB7 "),
                                    .fontSize = Theme::FontCaption,
                                    .color = Theme::TextMuted
                                }),
                                View(Text{
                                    .value = sizeStr,
                                    .fontSize = Theme::FontCaption,
                                    .color = Theme::TextMuted,
                                    .horizontalAlignment = HorizontalAlignment::leading
                                })
                            }
                        })
                    }
                })
            }
        });
    }
};

struct SidebarView {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;

    AppState* state = nullptr;

    View body() const {
        if (!state) return View(VStack{});

        std::vector<ModelInfo> models = state->installedModels;
        std::optional<ModelInfo> active = state->activeModel;
        bool expanded = state->sidebarExpanded;
        ModelLoadState loadSt = state->loadState;

        float sidebarW = expanded ? 240.0f : 48.0f;

        if (!expanded) {
            return View(VStack{
                .backgroundColor = Theme::Surface,
                .borderColor = Theme::Border,
                .borderWidth = 1.0f,
                .minWidth = sidebarW,
                .maxWidth = sidebarW,
                .expansionBias = 0.0f,
                .children = {
                    View(Button{
                        .text = std::string("\xE2\x89\xA1"),
                        .backgroundColor = Colors::transparent,
                        .padding = EdgeInsets(12),
                        .onClick = [this]() { state->sidebarExpanded = true; }
                    })
                }
            });
        }

        std::vector<View> modelViews;
        for (size_t i = 0; i < models.size(); i++) {
            bool isActive = active.has_value() && active->id == models[i].id;
            ModelInfo captured = models[i];
            modelViews.push_back(View(ModelListItem{
                .model = captured,
                .isActive = isActive,
                .onClick = [this, captured]() {
                    state->activeModel = captured;
                    state->loadState = ModelLoadState::READY;
                }
            }));
        }

        std::string statusText = "No model loaded";
        Color statusColor = Theme::TextMuted;
        std::string statusDot = "\xE2\x97\x8B";

        if (active.has_value()) {
            switch (loadSt) {
                case ModelLoadState::READY:
                    statusText = active->name;
                    statusColor = Theme::Success;
                    statusDot = "\xE2\x97\x8F";
                    break;
                case ModelLoadState::LOADING:
                    statusText = "Loading...";
                    statusColor = Theme::Accent;
                    statusDot = "\xE2\x97\x8F";
                    break;
                case ModelLoadState::ERROR:
                    statusText = "Error";
                    statusColor = Theme::Destructive;
                    statusDot = "\xE2\x97\x8F";
                    break;
                default: break;
            }
        }

        return View(VStack{
            .spacing = 0.0f,
            .backgroundColor = Theme::Surface,
            .borderColor = Theme::Border,
            .borderWidth = 1.0f,
            .minWidth = sidebarW,
            .maxWidth = sidebarW,
            .expansionBias = 0.0f,
            .children = {
                View(HStack{
                    .spacing = Theme::Space2,
                    .justifyContent = JustifyContent::spaceBetween,
                    .alignItems = AlignItems::center,
                    .padding = EdgeInsets(Theme::Space3),
                    .children = {
                        View(Text{
                            .value = std::string("Models"),
                            .fontSize = Theme::FontCaption,
                            .fontWeight = FontWeight::semibold,
                            .color = Theme::TextMuted,
                            .horizontalAlignment = HorizontalAlignment::leading
                        }),
                        View(Button{
                            .text = std::string("\xE2\x89\xA1"),
                            .backgroundColor = Colors::transparent,
                            .padding = EdgeInsets(4),
                            .onClick = [this]() { state->sidebarExpanded = false; }
                        })
                    }
                }),

                View(Divider{.borderColor = Theme::Border}),

                View(ScrollArea{
                    .expansionBias = 1.0f,
                    .children = modelViews.empty()
                        ? std::vector<View>{View(Text{
                            .value = std::string("No models installed"),
                            .fontSize = Theme::FontCaption,
                            .color = Theme::TextMuted,
                            .padding = Theme::Space4
                        })}
                        : std::move(modelViews)
                }),

                View(Button{
                    .text = std::string("+ Browse Hub"),
                    .backgroundColor = Colors::transparent,
                    .padding = EdgeInsets(Theme::Space3),
                    .cornerRadius = 0.0f,
                    .onClick = [this]() { state->currentView = AppView::HUB; }
                }),

                View(Divider{.borderColor = Theme::Border}),

                View(HStack{
                    .spacing = Theme::Space2,
                    .alignItems = AlignItems::center,
                    .padding = EdgeInsets(Theme::Space3),
                    .children = {
                        View(Text{
                            .value = statusDot,
                            .fontSize = 10.0f,
                            .color = statusColor
                        }),
                        View(VStack{
                            .spacing = 1.0f,
                            .expansionBias = 1.0f,
                            .children = {
                                View(Text{
                                    .value = std::string("Active"),
                                    .fontSize = Theme::FontCaption,
                                    .color = Theme::TextMuted,
                                    .horizontalAlignment = HorizontalAlignment::leading
                                }),
                                View(Text{
                                    .value = statusText,
                                    .fontSize = Theme::FontCaption,
                                    .fontWeight = FontWeight::medium,
                                    .color = Theme::TextPrimary,
                                    .horizontalAlignment = HorizontalAlignment::leading
                                })
                            }
                        })
                    }
                })
            }
        });
    }
};

} // namespace llm_studio
