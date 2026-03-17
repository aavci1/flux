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
#include <Flux/Views/ProgressBar.hpp>
#include <Flux/Views/ScrollArea.hpp>
#include "../Theme.hpp"
#include "../AppState.hpp"
#include "../Components/TextArea.hpp"
#include "../Components/TextInput.hpp"
#include <format>

namespace llm_studio {

using namespace flux;

struct ImageView {
    FLUX_VIEW_PROPERTIES;

    AppState* state = nullptr;

    View body() const {
        if (!state) return View(VStack{});

        ImageGenParams params = state->imageParams;
        bool generating = state->isGeneratingImage;
        float progress = state->imageGenProgress;
        std::vector<GeneratedImage> history = state->imageHistory;

        return View(HStack{
            .spacing = 0.0f,
            .alignItems = AlignItems::stretch,
            .expansionBias = 1.0f,
            .children = {
                // Left panel - Controls
                View(ScrollArea{
                    .minWidth = 300.0f,
                    .maxWidth = 300.0f,
                    .children = {
                        View(VStack{
                            .spacing = Theme::Space4,
                            .backgroundColor = Theme::Surface,
                            .padding = Theme::Space4,
                            .borderColor = Theme::Border,
                            .borderWidth = 1.0f,
                            .children = {
                                View(Text{
                                    .value = std::string("Image Generation"),
                                    .fontSize = Theme::FontH2,
                                    .fontWeight = FontWeight::semibold,
                                    .color = Theme::TextPrimary,
                                    .horizontalAlignment = HorizontalAlignment::leading
                                }),

                                View(VStack{
                                    .spacing = Theme::Space1,
                                    .children = {
                                        View(Text{
                                            .value = std::string("Prompt"),
                                            .fontSize = Theme::FontCaption,
                                            .color = Theme::TextMuted,
                                            .horizontalAlignment = HorizontalAlignment::leading
                                        }),
                                        View(TextArea{
                                            .value = [this]() -> std::string {
                                                return static_cast<ImageGenParams>(state->imageParams).prompt;
                                            },
                                            .placeholder = std::string("A cat wearing a top hat..."),
                                            .areaMinHeight = 60.0f,
                                            .areaMaxHeight = 100.0f,
                                            .bgColor = Theme::Background,
                                            .areaWidth = 268.0f
                                        })
                                    }
                                }),

                                View(VStack{
                                    .spacing = Theme::Space1,
                                    .children = {
                                        View(Text{
                                            .value = std::string("Negative Prompt"),
                                            .fontSize = Theme::FontCaption,
                                            .color = Theme::TextMuted,
                                            .horizontalAlignment = HorizontalAlignment::leading
                                        }),
                                        View(TextArea{
                                            .value = [this]() -> std::string {
                                                return static_cast<ImageGenParams>(state->imageParams).negativePrompt;
                                            },
                                            .placeholder = std::string("blurry, low quality"),
                                            .areaMinHeight = 40.0f,
                                            .areaMaxHeight = 60.0f,
                                            .bgColor = Theme::Background,
                                            .areaWidth = 268.0f
                                        })
                                    }
                                }),

                                View(Divider{.borderColor = Theme::Border}),

                                makeSliderRow("Steps", params.steps, 1, 100),
                                makeSliderRow("CFG Scale", params.cfgScale, 1.0f, 20.0f),
                                makeSliderRow("Width", static_cast<float>(params.width), 256, 1024),
                                makeSliderRow("Height", static_cast<float>(params.height), 256, 1024),

                                View(VStack{
                                    .spacing = Theme::Space1,
                                    .children = {
                                        View(Text{
                                            .value = std::string("Seed (-1 = random)"),
                                            .fontSize = Theme::FontCaption,
                                            .color = Theme::TextMuted,
                                            .horizontalAlignment = HorizontalAlignment::leading
                                        }),
                                        View(TextInput{
                                            .value = [this]() -> std::string {
                                                return std::to_string(
                                                    static_cast<ImageGenParams>(state->imageParams).seed);
                                            },
                                            .placeholder = std::string("-1"),
                                            .inputWidth = 268.0f
                                        })
                                    }
                                }),

                                View(Divider{.borderColor = Theme::Border}),

                                View(HStack{
                                    .spacing = Theme::Space2,
                                    .children = {
                                        View(Button{
                                            .text = generating
                                                ? std::string("Cancel")
                                                : std::string("Generate"),
                                            .backgroundColor = generating
                                                ? Theme::Destructive : Theme::Accent,
                                            .padding = EdgeInsets(10, 20, 10, 20),
                                            .cornerRadius = Theme::RadiusPill,
                                            .onClick = [this, generating]() {
                                                if (generating) {
                                                    state->isGeneratingImage = false;
                                                } else {
                                                    simulateImageGen();
                                                }
                                            }
                                        }),
                                        View(Button{
                                            .text = std::string("Save"),
                                            .backgroundColor = Theme::SurfaceRaised,
                                            .padding = EdgeInsets(10, 16, 10, 16),
                                            .cornerRadius = Theme::RadiusPill
                                        })
                                    }
                                })
                            }
                        })
                    }
                }),

                // Right panel - Canvas + history
                View(VStack{
                    .spacing = 0.0f,
                    .expansionBias = 1.0f,
                    .children = {
                        View(VStack{
                            .spacing = Theme::Space2,
                            .backgroundColor = Theme::Background,
                            .padding = Theme::Space8,
                            .expansionBias = 1.0f,
                            .children = {
                                generating
                                    ? View(VStack{
                                        .spacing = Theme::Space4,
                                        .children = {
                                            View(Text{
                                                .value = std::string("Generating..."),
                                                .fontSize = Theme::FontH2,
                                                .fontWeight = FontWeight::semibold,
                                                .color = Theme::TextPrimary
                                            }),
                                            View(ProgressBar{
                                                .value = progress,
                                                .height = 6.0f,
                                                .fillColor = Theme::Accent,
                                                .trackColor = Theme::Border,
                                                .showLabel = true
                                            }),
                                            View(Text{
                                                .value = std::format("{:.0f}%", progress * 100),
                                                .fontSize = Theme::FontCaption,
                                                .color = Theme::TextMuted
                                            })
                                        }
                                    })
                                    : View(VStack{
                                        .spacing = Theme::Space2,
                                        .children = {
                                            View(VStack{
                                                .backgroundColor = Theme::SurfaceRaised,
                                                .padding = Theme::Space12,
                                                .cornerRadius = Theme::RadiusCard,
                                                .borderColor = Theme::Border,
                                                .borderWidth = 1.0f,
                                                .children = {
                                                    View(Text{
                                                        .value = history.empty()
                                                            ? std::string("Generated images will appear here")
                                                            : std::string("Latest: ") + history.back().prompt,
                                                        .fontSize = Theme::FontBody,
                                                        .color = Theme::TextMuted
                                                    })
                                                }
                                            })
                                        }
                                    })
                            }
                        }),

                        View(Divider{.borderColor = Theme::Border}),

                        View(HStack{
                            .spacing = Theme::Space2,
                            .backgroundColor = Theme::Surface,
                            .padding = EdgeInsets(Theme::Space2, Theme::Space4,
                                                  Theme::Space2, Theme::Space4),
                            .minHeight = 80.0f,
                            .maxHeight = 80.0f,
                            .children = history.empty()
                                ? std::vector<View>{View(Text{
                                    .value = std::string("History will appear here"),
                                    .fontSize = Theme::FontCaption,
                                    .color = Theme::TextMuted,
                                    .padding = Theme::Space4
                                })}
                                : makeHistoryThumbs(history)
                        })
                    }
                })
            }
        });
    }

private:
    static View makeSliderRow(const std::string& label, float val, float minV, float maxV) {
        return View(HStack{
            .spacing = Theme::Space2,
            .alignItems = AlignItems::center,
            .children = {
                View(Text{
                    .value = label,
                    .fontSize = Theme::FontCaption,
                    .color = Theme::TextMuted,
                    .horizontalAlignment = HorizontalAlignment::leading,
                    .minWidth = 70.0f
                }),
                View(Slider{
                    .value = val,
                    .minValue = minV,
                    .maxValue = maxV,
                    .step = (maxV - minV) > 50 ? 1.0f : 0.1f,
                    .activeColor = Theme::Accent,
                    .inactiveColor = Theme::Border,
                    .expansionBias = 1.0f
                }),
                View(Text{
                    .value = (maxV > 50)
                        ? std::format("{:.0f}", val)
                        : std::format("{:.1f}", val),
                    .fontSize = Theme::FontCaption,
                    .fontWeight = FontWeight::medium,
                    .color = Theme::Accent,
                    .minWidth = 36.0f
                })
            }
        });
    }

    static std::vector<View> makeHistoryThumbs(const std::vector<GeneratedImage>& history) {
        std::vector<View> thumbs;
        int count = std::min(static_cast<int>(history.size()), 8);
        for (int i = static_cast<int>(history.size()) - count; i < static_cast<int>(history.size()); i++) {
            thumbs.push_back(View(VStack{
                .backgroundColor = Theme::SurfaceRaised,
                .cornerRadius = Theme::RadiusSmall,
                .borderColor = Theme::Border,
                .borderWidth = 1.0f,
                .minWidth = 64.0f,
                .minHeight = 64.0f,
                .maxWidth = 64.0f,
                .maxHeight = 64.0f,
                .children = {
                    View(Text{
                        .value = std::format("#{}", i + 1),
                        .fontSize = 10.0f,
                        .color = Theme::TextMuted
                    })
                }
            }));
        }
        return thumbs;
    }

    void simulateImageGen() const {
        if (!state) return;
        state->isGeneratingImage = true;
        state->imageGenProgress = 0.0f;

        std::thread([this]() {
            for (int i = 0; i <= 100; i++) {
                if (!static_cast<bool>(state->isGeneratingImage)) break;
                state->imageGenProgress = i / 100.0f;
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
            if (static_cast<bool>(state->isGeneratingImage)) {
                auto hist = static_cast<std::vector<GeneratedImage>>(state->imageHistory);
                hist.push_back(GeneratedImage{
                    .path = "/tmp/generated.png",
                    .prompt = static_cast<ImageGenParams>(state->imageParams).prompt,
                    .timestamp = std::chrono::system_clock::now()
                });
                state->imageHistory = std::move(hist);
            }
            state->isGeneratingImage = false;
        }).detach();
    }
};

} // namespace llm_studio
