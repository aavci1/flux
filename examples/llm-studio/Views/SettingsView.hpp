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
#include "../Components/TextInput.hpp"
#include "../Components/SectionHeader.hpp"
#include "../Components/SelectInput.hpp"
#include <format>

namespace llm_studio {

using namespace flux;

struct SettingsRow {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> label = std::string("");
    Property<std::string> description = std::string("");

    Property<std::vector<View>> children = {};

    View body() const {
        std::string lbl = label;
        std::string desc = description;
        std::vector<View> controls = children;

        std::vector<View> labelChildren;
        labelChildren.push_back(View(Text{
            .value = lbl,
            .fontSize = Theme::FontBody,
            .color = Theme::TextPrimary,
            .horizontalAlignment = HorizontalAlignment::leading
        }));
        if (!desc.empty()) {
            labelChildren.push_back(View(Text{
                .value = desc,
                .fontSize = Theme::FontCaption,
                .color = Theme::TextMuted,
                .horizontalAlignment = HorizontalAlignment::leading
            }));
        }

        return View(HStack{
            .spacing = Theme::Space4,
            .alignItems = AlignItems::center,
            .padding = EdgeInsets(Theme::Space2, 0, Theme::Space2, 0),
            .children = {
                View(VStack{
                    .spacing = 2.0f,
                    .expansionBias = 1.0f,
                    .children = std::move(labelChildren)
                }),
                View(HStack{
                    .spacing = Theme::Space2,
                    .alignItems = AlignItems::center,
                    .children = std::move(controls)
                })
            }
        });
    }
};

struct SettingsView {
    FLUX_VIEW_PROPERTIES;

    AppState* state = nullptr;

    View body() const {
        if (!state) return View(VStack{});

        AppSettings settings = state->settings;

        return View(ScrollArea{
            .padding = EdgeInsets(0, Theme::Space6, 0, Theme::Space6),
            .expansionBias = 1.0f,
            .children = {
                View(VStack{
                    .spacing = Theme::Space2,
                    .padding = EdgeInsets(Theme::Space4, Theme::Space8,
                                          Theme::Space8, Theme::Space8),
                    .expansionBias = 1.0f,
                    .maxWidth = 700.0f,
                    .children = {
                        View(Text{
                            .value = std::string("Settings"),
                            .fontSize = 22.0f,
                            .fontWeight = FontWeight::bold,
                            .color = Theme::TextPrimary,
                            .horizontalAlignment = HorizontalAlignment::leading,
                            .padding = EdgeInsets(0, 0, Theme::Space4, 0)
                        }),

                        // Inference section
                        View(SectionHeader{.title = std::string("Inference")}),

                        View(SettingsRow{
                            .label = std::string("Backend path"),
                            .description = std::string("Path to llama-server binary"),
                            .children = {
                                View(TextInput{
                                    .value = settings.backendPath,
                                    .inputWidth = 250.0f
                                })
                            }
                        }),

                        View(SettingsRow{
                            .label = std::string("Context length"),
                            .children = {
                                View(TextInput{
                                    .value = std::to_string(settings.contextLength),
                                    .inputWidth = 100.0f
                                })
                            }
                        }),

                        View(SettingsRow{
                            .label = std::string("GPU layers"),
                            .description = std::string("Number of layers to offload to GPU (0 = CPU only)"),
                            .children = {
                                View(Slider{
                                    .value = static_cast<float>(settings.gpuLayers),
                                    .minValue = 0.0f,
                                    .maxValue = 100.0f,
                                    .step = 1.0f,
                                    .activeColor = Theme::Accent,
                                    .inactiveColor = Theme::Border,
                                    .maxWidth = 150.0f
                                }),
                                View(Text{
                                    .value = std::to_string(settings.gpuLayers),
                                    .fontSize = Theme::FontBody,
                                    .fontWeight = FontWeight::medium,
                                    .color = Theme::Accent,
                                    .minWidth = 30.0f
                                })
                            }
                        }),

                        View(SettingsRow{
                            .label = std::string("Threads"),
                            .children = {
                                View(Slider{
                                    .value = static_cast<float>(settings.threads),
                                    .minValue = 1.0f,
                                    .maxValue = 32.0f,
                                    .step = 1.0f,
                                    .activeColor = Theme::Accent,
                                    .inactiveColor = Theme::Border,
                                    .maxWidth = 150.0f
                                }),
                                View(Text{
                                    .value = std::to_string(settings.threads),
                                    .fontSize = Theme::FontBody,
                                    .fontWeight = FontWeight::medium,
                                    .color = Theme::Accent,
                                    .minWidth = 30.0f
                                })
                            }
                        }),

                        // Interface section
                        View(SectionHeader{.title = std::string("Interface")}),

                        View(SettingsRow{
                            .label = std::string("Theme"),
                            .children = {
                                View(SelectInput{
                                    .options = std::vector<std::string>{"Dark", "Light", "System"},
                                    .selectWidth = 140.0f
                                })
                            }
                        }),

                        View(SettingsRow{
                            .label = std::string("Font size"),
                            .children = {
                                View(Slider{
                                    .value = settings.fontSize,
                                    .minValue = 10.0f,
                                    .maxValue = 22.0f,
                                    .step = 1.0f,
                                    .activeColor = Theme::Accent,
                                    .inactiveColor = Theme::Border,
                                    .maxWidth = 120.0f
                                }),
                                View(Text{
                                    .value = std::format("{:.0f}px", settings.fontSize),
                                    .fontSize = Theme::FontBody,
                                    .color = Theme::Accent,
                                    .minWidth = 40.0f
                                })
                            }
                        }),

                        View(SettingsRow{
                            .label = std::string("Sidebar visible on start"),
                            .children = {
                                View(Toggle{
                                    .isOn = settings.sidebarDefault,
                                    .onColor = Theme::Accent,
                                    .offColor = Theme::Border
                                })
                            }
                        }),

                        // Storage section
                        View(SectionHeader{.title = std::string("Storage")}),

                        View(SettingsRow{
                            .label = std::string("Model directory"),
                            .children = {
                                View(TextInput{
                                    .value = settings.modelDirectory.string(),
                                    .inputWidth = 280.0f
                                })
                            }
                        }),

                        View(SettingsRow{
                            .label = std::string("Image output directory"),
                            .children = {
                                View(TextInput{
                                    .value = settings.imageDirectory.string(),
                                    .inputWidth = 280.0f
                                })
                            }
                        }),

                        // HuggingFace section
                        View(SectionHeader{.title = std::string("HuggingFace")}),

                        View(SettingsRow{
                            .label = std::string("API Token"),
                            .description = std::string("Required for gated models"),
                            .children = {
                                View(TextInput{
                                    .value = settings.hfToken.empty()
                                        ? std::string("")
                                        : std::string("hf_****"),
                                    .placeholder = std::string("hf_..."),
                                    .password = true,
                                    .inputWidth = 250.0f
                                })
                            }
                        }),

                        View(Spacer{}),

                        View(HStack{
                            .spacing = Theme::Space2,
                            .padding = EdgeInsets(Theme::Space4, 0, 0, 0),
                            .children = {
                                View(Button{
                                    .text = std::string("Save Settings"),
                                    .backgroundColor = Theme::Accent,
                                    .padding = EdgeInsets(10, 20, 10, 20),
                                    .cornerRadius = Theme::RadiusCard
                                }),
                                View(Button{
                                    .text = std::string("Reset to Defaults"),
                                    .backgroundColor = Theme::SurfaceRaised,
                                    .padding = EdgeInsets(10, 16, 10, 16),
                                    .cornerRadius = Theme::RadiusCard,
                                    .borderColor = Theme::Border,
                                    .borderWidth = 1.0f
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
