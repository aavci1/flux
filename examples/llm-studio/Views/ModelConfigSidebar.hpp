#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/Typography.hpp>
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
#include "../AppState.hpp"
#include <format>
#include <algorithm>

namespace llm_studio {

using namespace flux;

struct ModelConfigSidebar {
    FLUX_VIEW_PROPERTIES;

    AppState* state = nullptr;

    View body() const {
        if (!state) return VStack{};

        bool expanded = state->rightSidebarExpanded;
        float sidebarW = expanded ? 280.0f : 48.0f;

        Theme d = Theme::dark();

        if (!expanded) {
            return VStack{
                .backgroundColor = d.surface,
                .borderColor = d.border,
                .borderWidth = 1.0f,
                .minWidth = sidebarW,
                .maxWidth = sidebarW,
                .children = {
                    Button{
                        .text = std::string("\xE2\x89\xA1"),
                        .backgroundColor = Colors::transparent,
                        .textColor = d.foreground,
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

        Color statusColor = Colors::darkGray;
        std::string statusDot = "\xE2\x97\x8B";
        if (activeModel.has_value()) {
            switch (loadSt) {
                case ModelLoadState::READY:
                    statusColor = Colors::green;
                    statusDot = "\xE2\x97\x8F";
                    break;
                case ModelLoadState::LOADING:
                    statusColor = Theme::dark().accent;
                    statusDot = "\xE2\x97\x8F";
                    break;
                case ModelLoadState::ERROR:
                    statusColor = Colors::red;
                    statusDot = "\xE2\x97\x8F";
                    break;
                default: break;
            }
        }

        return VStack{
            .spacing = 0.0f,
            .backgroundColor = d.surface,
            .borderColor = d.border,
            .borderWidth = 1.0f,
            .minWidth = sidebarW,
            .maxWidth = sidebarW,
            .children = {
                HStack{
                    .spacing = 8.0f,
                    .justifyContent = JustifyContent::spaceBetween,
                    .alignItems = AlignItems::center,
                    .padding = EdgeInsets(12.0f),
                    .children = {
                        Text{
                            .value = std::string("Configuration"),
                            .fontSize = Typography::subheadline,
                            .fontWeight = FontWeight::semibold,
                            .color = d.foreground,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },
                        Button{
                            .text = std::string("\xC3\x97"),
                            .backgroundColor = Colors::transparent,
                            .textColor = d.foreground,
                            .padding = EdgeInsets(4),
                            .onClick = [this]() { state->rightSidebarExpanded = false; }
                        }
                    }
                },

                Divider{.borderColor = d.border},

                ScrollArea{
                    .expansionBias = 1.0f,
                    .children = {
                        VStack{
                            .spacing = 16.0f,
                            .padding = 12.0f,
                            .expansionBias = 1.0f,
                            .children = {
                                HStack{
                                    .spacing = 8.0f,
                                    .alignItems = AlignItems::center,
                                    .children = {
                                        Text{
                                            .value = statusDot,
                                            .fontSize = Typography::caption,
                                            .color = statusColor
                                        },
                                        Text{
                                            .value = modelName,
                                            .fontWeight = FontWeight::medium,
                                            .color = d.foreground,
                                            .horizontalAlignment = HorizontalAlignment::leading,
                                            .expansionBias = 1.0f
                                        }
                                    }
                                },

                                Divider{.borderColor = d.border},

                                makeParamSlider("Temperature", params.temperature, 0.0f, 2.0f, 0.1f,
                                    [this](float v) { state->updateActiveSession([v](ChatSession& s) { s.params.temperature = v; }); }),
                                makeParamSlider("Top P", params.topP, 0.0f, 1.0f, 0.05f,
                                    [this](float v) { state->updateActiveSession([v](ChatSession& s) { s.params.topP = v; }); }),
                                makeParamSlider("Max Tokens", static_cast<float>(params.maxTokens), 256.0f, 8192.0f, 256.0f,
                                    [this](float v) { state->updateActiveSession([v](ChatSession& s) { s.params.maxTokens = static_cast<int>(v); }); }),

                                Divider{.borderColor = d.border},

                                VStack{
                                    .spacing = 12.0f,
                                    .children = {
                                        Text{
                                            .value = std::string("System Prompt"),
                                            .fontSize = Typography::subheadline,
                                            .color = d.secondaryForeground,
                                            .horizontalAlignment = HorizontalAlignment::leading
                                        },
                                        TextArea{
                                            .value = params.systemPrompt,
                                            .placeholder = std::string("You are a helpful assistant..."),
                                            .areaMinHeight = 80.0f,
                                            .areaMaxHeight = 200.0f,
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
        bool integerFormat = maxV > 50;
        std::string valStr = integerFormat
            ? std::format("{:.0f}", value)
            : std::format("{:.2f}", value);

        std::string widestStr = integerFormat
            ? std::format("{:.0f}", maxV)
            : std::format("{:.2f}", maxV);
        float valueLabelWidth = std::max(28.0f, widestStr.size() * 8.0f);

        return VStack{
            .spacing = 12.0f,
            .children = {
                HStack{
                    .spacing = 8.0f,
                    .justifyContent = JustifyContent::spaceBetween,
                    .alignItems = AlignItems::center,
                    .children = {
                        Text{
                            .value = label,
                            .fontSize = Typography::subheadline,
                            .color = Colors::darkGray,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },
                        Text{
                            .value = valStr,
                            .fontSize = Typography::subheadline,
                            .fontWeight = FontWeight::medium,
                            .color = Colors::blue,
                            .horizontalAlignment = HorizontalAlignment::trailing,
                            .minWidth = valueLabelWidth
                        }
                    }
                },
                Slider{
                    .value = value,
                    .minValue = minV,
                    .maxValue = maxV,
                    .step = step,
                    .activeColor = Colors::blue,
                    .inactiveColor = Colors::gray,
                    .onValueChange = std::move(callback)
                }
            }
        };
    }
};

} // namespace llm_studio
