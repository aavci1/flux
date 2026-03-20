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
#include <Flux/Views/Toggle.hpp>
#include <Flux/Views/Slider.hpp>
#include <Flux/Views/Divider.hpp>
#include <Flux/Views/ScrollArea.hpp>
#include "../AppState.hpp"
#include <Flux/Views/TextInput.hpp>
#include <Flux/Views/SectionHeader.hpp>
#include <Flux/Views/SelectInput.hpp>
#include <Flux/Core/Theme.hpp>
#include <format>

namespace llm_studio {

using namespace flux;

struct SettingsRow {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> label = std::string("");
    Property<std::string> description = std::string("");

    Property<std::vector<View>> controls = {};

    View body() const {
        Theme d = Theme::dark();
        std::string lbl = label;
        std::string desc = description;
        std::vector<View> controlVec = controls;

        std::vector<View> labelChildren;
        labelChildren.push_back(Text{
            .value = lbl,
            .color = d.foreground,
            .horizontalAlignment = HorizontalAlignment::leading
        });
        if (!desc.empty()) {
            labelChildren.push_back(Text{
                .value = desc,
                .fontSize = Typography::subheadline,
                .color = d.secondaryForeground,
                .horizontalAlignment = HorizontalAlignment::leading
            });
        }

        return HStack{
            .spacing = 24.0f,
            .alignItems = AlignItems::center,
            .padding = EdgeInsets(12.0f, 8.0f, 12.0f, 8.0f),
            .children = {
                VStack{
                    .spacing = 6.0f,
                    .expansionBias = 1.0f,
                    .children = std::move(labelChildren)
                },
                HStack{
                    .spacing = 8.0f,
                    .alignItems = AlignItems::center,
                    .minWidth = 280.0f,
                    .maxWidth = 280.0f,
                    .justifyContent = JustifyContent::end,
                    .children = std::move(controlVec)
                }
            }
        };
    }
};

struct SettingsView {
    FLUX_VIEW_PROPERTIES;

    AppState* state = nullptr;

    View body() const {
        if (!state) return VStack{};

        AppSettings settings = state->settings;
        Theme d = Theme::dark();

        return VStack{
            .spacing = 0.0f,
            .expansionBias = 1.0f,
            .children = {
                HStack{
                    .spacing = 8.0f,
                    .justifyContent = JustifyContent::spaceBetween,
                    .alignItems = AlignItems::center,
                    .padding = EdgeInsets(20.0f, 28.0f, 16.0f, 28.0f),
                    .children = {
                        Text{
                            .value = std::string("Settings"),
                            .fontSize = 22.0f,
                            .fontWeight = FontWeight::bold,
                            .color = d.foreground,
                        },
                        HStack{
                            .spacing = 10.0f,
                            .children = {
                                Button{
                                    .text = std::string("Reset to Defaults"),
                                    .backgroundColor = d.surfaceElevated,
                                    .textColor = d.foreground,
                                    .padding = EdgeInsets(8, 16, 8, 16),
                                    .cornerRadius = 6.0f,
                                    .borderColor = d.borderStrong,
                                    .borderWidth = 1.0f,
                                    .onClick = [this]() { state->settings = AppSettings{}; }
                                },
                                Button{
                                    .text = std::string("Save"),
                                    .backgroundColor = d.accent,
                                    .textColor = d.onAccent,
                                    .padding = EdgeInsets(8, 20, 8, 20),
                                    .cornerRadius = 6.0f
                                }
                            }
                        }
                    }
                },
                Divider{.borderColor = d.border},
                ScrollArea{
                    .expansionBias = 1.0f,
                    .children = {
                        VStack{
                            .spacing = 16.0f,
                            .padding = EdgeInsets(8.0f, 28.0f, 32.0f, 28.0f),
                            .expansionBias = 1.0f,
                            .maxWidth = 720.0f,
                            .children = {
                                SectionHeader{
                                    .title = std::string("Inference"),
                                    .color = d.secondaryForeground,
                                    .dividerColor = d.border,
                                },

                                SettingsRow{
                                    .label = std::string("Backend path"),
                                    .description = std::string("Path to llama-server binary"),
                                    .controls = {
                                        TextInput{
                                            .value = settings.backendPath,
                                            .inputWidth = 280.0f,
                                            .onValueChange = [this](const std::string& val) {
                                                state->updateSettings([&](AppSettings& s) { s.backendPath = val; });
                                            }
                                        }
                                    }
                                },

                                SettingsRow{
                                    .label = std::string("Context length"),
                                    .controls = {
                                        TextInput{
                                            .value = std::to_string(settings.contextLength),
                                            .inputWidth = 280.0f,
                                            .onValueChange = [this](const std::string& val) {
                                                try { state->updateSettings([&](AppSettings& s) { s.contextLength = std::stoi(val); }); }
                                                catch (...) {}
                                            }
                                        }
                                    }
                                },

                                SettingsRow{
                                    .label = std::string("GPU layers"),
                                    .description = std::string("Layers to offload to GPU (0 = CPU only)"),
                                    .controls = {
                                        Slider{
                                            .value = static_cast<float>(settings.gpuLayers),
                                            .minValue = 0.0f,
                                            .maxValue = 100.0f,
                                            .step = 1.0f,
                                            .activeColor = d.accent,
                                            .inactiveColor = d.border,
                                            .maxWidth = 220.0f,
                                            .onValueChange = [this](float v) {
                                                state->updateSettings([v](AppSettings& s) { s.gpuLayers = static_cast<int>(v); });
                                            }
                                        },
                                        Text{
                                            .value = std::to_string(settings.gpuLayers),
                                            .fontWeight = FontWeight::medium,
                                            .color = d.accent,
                                            .minWidth = 30.0f
                                        }
                                    }
                                },

                                SettingsRow{
                                    .label = std::string("Threads"),
                                    .controls = {
                                        Slider{
                                            .value = static_cast<float>(settings.threads),
                                            .minValue = 1.0f,
                                            .maxValue = 32.0f,
                                            .step = 1.0f,
                                            .activeColor = d.accent,
                                            .inactiveColor = d.border,
                                            .maxWidth = 220.0f,
                                            .onValueChange = [this](float v) {
                                                state->updateSettings([v](AppSettings& s) { s.threads = static_cast<int>(v); });
                                            }
                                        },
                                        Text{
                                            .value = std::to_string(settings.threads),
                                            .fontWeight = FontWeight::medium,
                                            .color = d.accent,
                                            .minWidth = 30.0f
                                        }
                                    }
                                },

                                SectionHeader{
                                    .title = std::string("Interface"),
                                    .color = d.secondaryForeground,
                                    .dividerColor = d.border,
                                },

                                SettingsRow{
                                    .label = std::string("Theme"),
                                    .controls = {
                                        SelectInput{
                                            .options = std::vector<std::string>{"Dark", "Light", "System"},
                                            .selectWidth = 280.0f,
                                            .bgColor = d.inputBackground,
                                            .borderColor_ = d.borderStrong,
                                            .textColor_ = d.inputForeground,
                                            .mutedColor = d.secondaryForeground,
                                            .accentColor = d.accent,
                                            .onSelect = [this](int, const std::string& val) {
                                                state->updateSettings([&](AppSettings& s) { s.theme = val; });
                                            }
                                        }
                                    }
                                },

                                SettingsRow{
                                    .label = std::string("Font size"),
                                    .controls = {
                                        Slider{
                                            .value = settings.fontSize,
                                            .minValue = 10.0f,
                                            .maxValue = 22.0f,
                                            .step = 1.0f,
                                            .activeColor = d.accent,
                                            .inactiveColor = d.border,
                                            .maxWidth = 220.0f,
                                            .onValueChange = [this](float v) {
                                                state->updateSettings([v](AppSettings& s) { s.fontSize = v; });
                                            }
                                        },
                                        Text{
                                            .value = std::format("{:.0f}px", settings.fontSize),
                                            .color = d.accent,
                                            .minWidth = 40.0f
                                        }
                                    }
                                },

                                SectionHeader{
                                    .title = std::string("Storage"),
                                    .color = d.secondaryForeground,
                                    .dividerColor = d.border,
                                },

                                SettingsRow{
                                    .label = std::string("Model directory"),
                                    .controls = {
                                        TextInput{
                                            .value = settings.modelDirectory.string(),
                                            .inputWidth = 280.0f,
                                            .onValueChange = [this](const std::string& val) {
                                                state->updateSettings([&](AppSettings& s) { s.modelDirectory = val; });
                                            }
                                        }
                                    }
                                },

                                SectionHeader{
                                    .title = std::string("HuggingFace"),
                                    .color = d.secondaryForeground,
                                    .dividerColor = d.border,
                                },

                                SettingsRow{
                                    .label = std::string("API Token"),
                                    .description = std::string("Required for gated models"),
                                    .controls = {
                                        TextInput{
                                            .value = settings.hfToken.empty()
                                                ? std::string("")
                                                : std::string("hf_****"),
                                            .placeholder = std::string("hf_..."),
                                            .password = true,
                                            .inputWidth = 280.0f,
                                            .onValueChange = [this](const std::string& val) {
                                                state->updateSettings([&](AppSettings& s) { s.hfToken = val; });
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
};

} // namespace llm_studio
