#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Views/VStack.hpp>
#include <Flux/Views/HStack.hpp>
#include <Flux/Views/Text.hpp>
#include <Flux/Views/Button.hpp>
#include <Flux/Views/Spacer.hpp>
#include <Flux/Views/Toggle.hpp>
#include <Flux/Views/Slider.hpp>
#include <Flux/Views/Divider.hpp>
#include <Flux/Views/ScrollArea.hpp>
#include "../Theme.hpp"
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
            .fontSize = Theme::FontBody,
            .color = Theme::TextPrimary,
            .horizontalAlignment = HorizontalAlignment::leading
        });
        if (!desc.empty()) {
            labelChildren.push_back(Text{
                .value = desc,
                .fontSize = Theme::FontCaption,
                .color = Theme::TextMuted,
                .horizontalAlignment = HorizontalAlignment::leading
            });
        }

        return HStack{
            .spacing = Theme::Space4,
            .alignItems = AlignItems::center,
            .padding = EdgeInsets(Theme::Space2, 0, Theme::Space2, 0),
            .children = {
                VStack{
                    .spacing = 2.0f,
                    .expansionBias = 1.0f,
                    .children = std::move(labelChildren)
                },
                HStack{
                    .spacing = Theme::Space2,
                    .alignItems = AlignItems::center,
                    .children = std::move(controlVec)
                }
            }
        };
    }
};

struct SettingsDialog {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;

    AppState* state = nullptr;

    void init() {
        onMouseDown = [](float, float, int) {};
    }

    View body() const {
        if (!state) return VStack{.visible = false};

        bool visible = state->showSettingsDialog;
        if (!visible) return VStack{.visible = false};

        AppSettings settings = state->settings;

        View settingsContent = ScrollArea{
            .expansionBias = 1.0f,
            .children = {
                VStack{
                    .spacing = Theme::Space2,
                    .padding = EdgeInsets(0, Theme::Space4, Theme::Space4, Theme::Space4),
                    .expansionBias = 1.0f,
                    .children = {
                        SectionHeader{.title = std::string("Inference")},

                        SettingsRow{
                            .label = std::string("Backend path"),
                            .description = std::string("Path to llama-server binary"),
                            .controls = {
                                TextInput{
                                    .value = settings.backendPath,
                                    .inputWidth = 220.0f
                                }
                            }
                        },

                        SettingsRow{
                            .label = std::string("Context length"),
                            .controls = {
                                TextInput{
                                    .value = std::to_string(settings.contextLength),
                                    .inputWidth = 100.0f
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
                                    .activeColor = Theme::Accent,
                                    .inactiveColor = Theme::Border,
                                    .maxWidth = 120.0f
                                },
                                Text{
                                    .value = std::to_string(settings.gpuLayers),
                                    .fontSize = Theme::FontBody,
                                    .fontWeight = FontWeight::medium,
                                    .color = Theme::Accent,
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
                                    .activeColor = Theme::Accent,
                                    .inactiveColor = Theme::Border,
                                    .maxWidth = 120.0f
                                },
                                Text{
                                    .value = std::to_string(settings.threads),
                                    .fontSize = Theme::FontBody,
                                    .fontWeight = FontWeight::medium,
                                    .color = Theme::Accent,
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
                                    .selectWidth = 120.0f
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
                                    .activeColor = Theme::Accent,
                                    .inactiveColor = Theme::Border,
                                    .maxWidth = 100.0f
                                },
                                Text{
                                    .value = std::format("{:.0f}px", settings.fontSize),
                                    .fontSize = Theme::FontBody,
                                    .color = Theme::Accent,
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
                                    .inputWidth = 240.0f
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
                                    .inputWidth = 220.0f
                                }
                            }
                        }
                    }
                }
            }
        };

        return VStack{
            .backgroundColor = Color(0.0f, 0.0f, 0.0f, 0.5f),
            .expansionBias = 1.0f,
            .children = {
                Spacer{},
                HStack{
                    .justifyContent = JustifyContent::center,
                    .minHeight = 500.0f,
                    .children = {
                        Spacer{},
                        VStack{
                            .spacing = 0.0f,
                            .backgroundColor = Theme::Surface,
                            .cornerRadius = Theme::RadiusDialog,
                            .borderColor = Theme::Border,
                            .borderWidth = 1.0f,
                            .minWidth = 600.0f,
                            .maxWidth = 600.0f,
                            .minHeight = 500.0f,
                            .maxHeight = 500.0f,
                            .children = {
                                HStack{
                                    .spacing = Theme::Space2,
                                    .justifyContent = JustifyContent::spaceBetween,
                                    .alignItems = AlignItems::center,
                                    .padding = EdgeInsets(Theme::Space4),
                                    .children = {
                                        Text{
                                            .value = std::string("Settings"),
                                            .fontSize = Theme::FontH1,
                                            .fontWeight = FontWeight::bold,
                                            .color = Theme::TextPrimary
                                        },
                                        Button{
                                            .text = std::string("\xC3\x97"),
                                            .backgroundColor = Colors::transparent,
                                            .padding = EdgeInsets(4, 8, 4, 8),
                                            .cornerRadius = Theme::RadiusSmall,
                                            .onClick = [this]() {
                                                state->showSettingsDialog = false;
                                            }
                                        }
                                    }
                                },
                                Divider{.borderColor = Theme::Border},
                                settingsContent,
                                Divider{.borderColor = Theme::Border},
                                HStack{
                                    .spacing = Theme::Space2,
                                    .justifyContent = JustifyContent::end,
                                    .padding = EdgeInsets(Theme::Space3, Theme::Space4, Theme::Space3, Theme::Space4),
                                    .children = {
                                        Button{
                                            .text = std::string("Reset to Defaults"),
                                            .backgroundColor = Theme::SurfaceRaised,
                                            .padding = EdgeInsets(8, 16, 8, 16),
                                            .cornerRadius = Theme::RadiusSmall,
                                            .borderColor = Theme::Border,
                                            .borderWidth = 1.0f
                                        },
                                        Button{
                                            .text = std::string("Save"),
                                            .backgroundColor = Theme::Accent,
                                            .padding = EdgeInsets(8, 20, 8, 20),
                                            .cornerRadius = Theme::RadiusSmall,
                                            .onClick = [this]() {
                                                state->showSettingsDialog = false;
                                            }
                                        }
                                    }
                                }
                            }
                        },
                        Spacer{}
                    }
                },
                Spacer{}
            }
        };
    }
};

} // namespace llm_studio
