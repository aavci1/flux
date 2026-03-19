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
#include <format>

namespace llm_studio {

using namespace flux;

struct SettingsRow {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> label = std::string("");
    Property<std::string> description = std::string("");

    Property<std::vector<View>> controls = {};

    View body() const {
        std::string lbl = label;
        std::string desc = description;
        std::vector<View> controlVec = controls;

        std::vector<View> labelChildren;
        labelChildren.push_back(Text{
            .value = lbl,
            .horizontalAlignment = HorizontalAlignment::leading
        });
        if (!desc.empty()) {
            labelChildren.push_back(Text{
                .value = desc,
                .fontSize = Typography::subheadline,
                .color = Colors::darkGray,
                .horizontalAlignment = HorizontalAlignment::leading
            });
        }

        return HStack{
            .spacing = 24.0f,
            .alignItems = AlignItems::center,
            .padding = EdgeInsets(12.0f, 0, 12.0f, 0),
            .children = {
                VStack{
                    .spacing = 8.0f,
                    .expansionBias = 1.0f,
                    .children = std::move(labelChildren)
                },
                HStack{
                    .spacing = 8.0f,
                    .alignItems = AlignItems::center,
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

        return VStack{
            .spacing = 0.0f,
            .expansionBias = 1.0f,
            .children = {
                HStack{
                    .spacing = 8.0f,
                    .justifyContent = JustifyContent::spaceBetween,
                    .alignItems = AlignItems::center,
                    .padding = EdgeInsets(24.0f),
                    .children = {
                        Text{
                            .value = std::string("Settings"),
                            .fontWeight = FontWeight::bold,
                        },
                        HStack{
                            .spacing = 8.0f,
                            .children = {
                                Button{
                                    .text = std::string("Reset to Defaults"),
                                    .backgroundColor = Colors::white,
                                    .textColor = Colors::black,
                                    .padding = EdgeInsets(8, 16, 8, 16),
                                    .cornerRadius = 4.0f,
                                    .borderColor = Colors::gray,
                                    .borderWidth = 1.0f,
                                    .onClick = [this]() { state->settings = AppSettings{}; }
                                },
                                Button{
                                    .text = std::string("Save"),
                                    .backgroundColor = Colors::blue,
                                    .padding = EdgeInsets(8, 20, 8, 20),
                                    .cornerRadius = 4.0f
                                }
                            }
                        }
                    }
                },
                Divider{.borderColor = Colors::gray},
                ScrollArea{
                    .expansionBias = 1.0f,
                    .children = {
                        VStack{
                            .spacing = 16.0f,
                            .padding = EdgeInsets(16.0f),
                            .expansionBias = 1.0f,
                            .maxWidth = 700.0f,
                            .children = {
                                SectionHeader{.title = std::string("Inference")},

                                SettingsRow{
                                    .label = std::string("Backend path"),
                                    .description = std::string("Path to llama-server binary"),
                                    .controls = {
                                        TextInput{
                                            .value = settings.backendPath,
                                            .inputWidth = 220.0f,
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
                                            .inputWidth = 100.0f,
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
                                            .activeColor = Colors::blue,
                                            .inactiveColor = Colors::gray,
                                            .maxWidth = 120.0f,
                                            .onValueChange = [this](float v) {
                                                state->updateSettings([v](AppSettings& s) { s.gpuLayers = static_cast<int>(v); });
                                            }
                                        },
                                        Text{
                                            .value = std::to_string(settings.gpuLayers),
                                            .fontWeight = FontWeight::medium,
                                            .color = Colors::blue,
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
                                            .activeColor = Colors::blue,
                                            .inactiveColor = Colors::gray,
                                            .maxWidth = 120.0f,
                                            .onValueChange = [this](float v) {
                                                state->updateSettings([v](AppSettings& s) { s.threads = static_cast<int>(v); });
                                            }
                                        },
                                        Text{
                                            .value = std::to_string(settings.threads),
                                            .fontWeight = FontWeight::medium,
                                            .color = Colors::blue,
                                            .minWidth = 30.0f
                                        }
                                    }
                                },

                                SectionHeader{.title = std::string("Interface")},

                                SettingsRow{
                                    .label = std::string("Theme"),
                                    .controls = {
                                        SelectInput{
                                            .options = std::vector<std::string>{"Dark", "Light", "System"},
                                            .selectWidth = 120.0f,
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
                                            .activeColor = Colors::blue,
                                            .inactiveColor = Colors::gray,
                                            .maxWidth = 100.0f,
                                            .onValueChange = [this](float v) {
                                                state->updateSettings([v](AppSettings& s) { s.fontSize = v; });
                                            }
                                        },
                                        Text{
                                            .value = std::format("{:.0f}px", settings.fontSize),
                                            .color = Colors::blue,
                                            .minWidth = 40.0f
                                        }
                                    }
                                },

                                SectionHeader{.title = std::string("Storage")},

                                SettingsRow{
                                    .label = std::string("Model directory"),
                                    .controls = {
                                        TextInput{
                                            .value = settings.modelDirectory.string(),
                                            .inputWidth = 240.0f,
                                            .onValueChange = [this](const std::string& val) {
                                                state->updateSettings([&](AppSettings& s) { s.modelDirectory = val; });
                                            }
                                        }
                                    }
                                },

                                SectionHeader{.title = std::string("HuggingFace")},

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
                                            .inputWidth = 220.0f,
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
