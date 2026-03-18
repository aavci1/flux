#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Views/VStack.hpp>
#include <Flux/Views/HStack.hpp>
#include <Flux/Views/Text.hpp>
#include <Flux/Views/Button.hpp>
#include <Flux/Views/Spacer.hpp>
#include <Flux/Views/Slider.hpp>
#include <Flux/Views/Divider.hpp>
#include <Flux/Views/ScrollArea.hpp>
#include <Flux/Views/TextArea.hpp>
#include <Flux/Views/TextInput.hpp>
#include "../Theme.hpp"
#include "../AppState.hpp"
#include <format>

namespace llm_studio {

using namespace flux;

struct ModelConfigSidebar {
    FLUX_VIEW_PROPERTIES;

    AppState* state = nullptr;

    View body() const {
        if (!state) return VStack{};

        bool expanded = state->rightSidebarExpanded;
        float sidebarW = expanded ? 280.0f : 48.0f;

        if (!expanded) {
            return VStack{
                .backgroundColor = Theme::Surface,
                .borderColor = Theme::Border,
                .borderWidth = 1.0f,
                .minWidth = sidebarW,
                .maxWidth = sidebarW,
                .children = {
                    Button{
                        .text = std::string("\xE2\x89\xA1"),
                        .backgroundColor = Colors::transparent,
                        .padding = EdgeInsets(12),
                        .onClick = [this]() { state->rightSidebarExpanded = true; }
                    }
                }
            };
        }

        auto session = state->getActiveSessionCopy();
        ChatParams params = session.has_value() ? session->params : ChatParams{};
        std::optional<ModelInfo> activeModel = state->activeModel;

        std::string modelName = activeModel.has_value() ? activeModel->name : "No model selected";
        ModelLoadState loadSt = state->loadState;

        Color statusColor = Theme::TextMuted;
        std::string statusDot = "\xE2\x97\x8B";
        if (activeModel.has_value()) {
            switch (loadSt) {
                case ModelLoadState::READY:
                    statusColor = Theme::Success;
                    statusDot = "\xE2\x97\x8F";
                    break;
                case ModelLoadState::LOADING:
                    statusColor = Theme::Accent;
                    statusDot = "\xE2\x97\x8F";
                    break;
                case ModelLoadState::ERROR:
                    statusColor = Theme::Destructive;
                    statusDot = "\xE2\x97\x8F";
                    break;
                default: break;
            }
        }

        return VStack{
            .spacing = 0.0f,
            .backgroundColor = Theme::Surface,
            .borderColor = Theme::Border,
            .borderWidth = 1.0f,
            .minWidth = sidebarW,
            .maxWidth = sidebarW,
            .children = {
                HStack{
                    .spacing = Theme::Space2,
                    .justifyContent = JustifyContent::spaceBetween,
                    .alignItems = AlignItems::center,
                    .padding = EdgeInsets(Theme::Space3),
                    .children = {
                        Text{
                            .value = std::string("Configuration"),
                            .fontSize = Theme::FontCaption,
                            .fontWeight = FontWeight::semibold,
                            .color = Theme::TextMuted,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },
                        Button{
                            .text = std::string("\xC3\x97"),
                            .backgroundColor = Colors::transparent,
                            .padding = EdgeInsets(4),
                            .onClick = [this]() { state->rightSidebarExpanded = false; }
                        }
                    }
                },

                Divider{.borderColor = Theme::Border},

                ScrollArea{
                    .expansionBias = 1.0f,
                    .children = {
                        VStack{
                            .spacing = Theme::Space4,
                            .padding = Theme::Space3,
                            .expansionBias = 1.0f,
                            .children = {
                                HStack{
                                    .spacing = Theme::Space2,
                                    .alignItems = AlignItems::center,
                                    .children = {
                                        Text{
                                            .value = statusDot,
                                            .fontSize = 10.0f,
                                            .color = statusColor
                                        },
                                        Text{
                                            .value = modelName,
                                            .fontSize = Theme::FontBody,
                                            .fontWeight = FontWeight::medium,
                                            .color = Theme::TextPrimary,
                                            .horizontalAlignment = HorizontalAlignment::leading,
                                            .expansionBias = 1.0f
                                        }
                                    }
                                },

                                Divider{.borderColor = Theme::Border},

                                makeParamSlider("Temperature", params.temperature, 0.0f, 2.0f, 0.1f,
                                    [this](float v) { state->updateActiveSession([v](ChatSession& s) { s.params.temperature = v; }); }),
                                makeParamSlider("Top P", params.topP, 0.0f, 1.0f, 0.05f,
                                    [this](float v) { state->updateActiveSession([v](ChatSession& s) { s.params.topP = v; }); }),
                                makeParamSlider("Max Tokens", static_cast<float>(params.maxTokens), 256.0f, 8192.0f, 256.0f,
                                    [this](float v) { state->updateActiveSession([v](ChatSession& s) { s.params.maxTokens = static_cast<int>(v); }); }),

                                Divider{.borderColor = Theme::Border},

                                VStack{
                                    .spacing = Theme::Space1,
                                    .children = {
                                        Text{
                                            .value = std::string("System Prompt"),
                                            .fontSize = Theme::FontCaption,
                                            .color = Theme::TextMuted,
                                            .horizontalAlignment = HorizontalAlignment::leading
                                        },
                                        TextArea{
                                            .value = params.systemPrompt,
                                            .placeholder = std::string("You are a helpful assistant..."),
                                            .fontSize = Theme::FontCaption,
                                            .areaMinHeight = 80.0f,
                                            .areaMaxHeight = 200.0f,
                                            .bgColor = Theme::Background,
                                            .areaWidth = 248.0f,
                                            .onValueChange = [this](const std::string& val) {
                                                state->updateActiveSession([&](ChatSession& s) {
                                                    s.params.systemPrompt = val;
                                                });
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        };
    }

private:
    View makeParamSlider(const std::string& label, float value, float minV, float maxV, float step,
                         std::function<void(float)> callback) const {
        std::string valStr = (maxV > 50)
            ? std::format("{:.0f}", value)
            : std::format("{:.2f}", value);

        return VStack{
            .spacing = Theme::Space1,
            .children = {
                HStack{
                    .spacing = Theme::Space2,
                    .justifyContent = JustifyContent::spaceBetween,
                    .alignItems = AlignItems::center,
                    .children = {
                        Text{
                            .value = label,
                            .fontSize = Theme::FontCaption,
                            .color = Theme::TextMuted,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },
                        Text{
                            .value = valStr,
                            .fontSize = Theme::FontCaption,
                            .fontWeight = FontWeight::medium,
                            .color = Theme::Accent
                        }
                    }
                },
                Slider{
                    .value = value,
                    .minValue = minV,
                    .maxValue = maxV,
                    .step = step,
                    .activeColor = Theme::Accent,
                    .inactiveColor = Theme::Border,
                    .onValueChange = std::move(callback)
                }
            }
        };
    }
};

} // namespace llm_studio
