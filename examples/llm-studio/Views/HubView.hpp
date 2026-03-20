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
#include <Flux/Views/ScrollArea.hpp>
#include <Flux/Views/ProgressBar.hpp>
#include <Flux/Views/Divider.hpp>
#include <Flux/Views/Badge.hpp>
#include "../AppState.hpp"
#include <Flux/Views/TextInput.hpp>
#include <Flux/Views/DropdownMenu.hpp>
#include <format>
#include <thread>
#include <chrono>

namespace llm_studio {

using namespace flux;

struct ModelCardView {
    FLUX_VIEW_PROPERTIES;

    Property<ModelCard> card;
    AppState* state = nullptr;

    View body() const {
        ModelCard c = card;
        Theme d = Theme::dark();

        std::string sizeStr = formatBytes(c.sizeBytes);
        std::string likesStr = std::format("\xE2\x98\x85 {}k", c.likes / 1000);

        return HStack{
            .spacing = 16.0f,
            .alignItems = AlignItems::start,
            .backgroundColor = d.surfaceElevated,
            .padding = EdgeInsets(16.0f),
            .cornerRadius = 8.0f,
            .borderColor = d.borderStrong,
            .borderWidth = 1.0f,
            .children = {
                VStack{
                    .spacing = 8.0f,
                    .expansionBias = 1.0f,
                    .children = {
                        Text{
                            .value = c.repoId,
                            .fontWeight = FontWeight::semibold,
                            .color = d.foreground,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },
                        HStack{
                            .spacing = 8.0f,
                            .alignItems = AlignItems::center,
                            .children = {
                                Badge{
                                    .text = likesStr,
                                    .badgeColor = Colors::yellow,
                                    .textColor = Colors::black,
                                },
                                Badge{
                                    .text = c.format,
                                    .badgeColor = d.accent.opacity(0.22f),
                                    .textColor = d.foreground,
                                },
                                Badge{
                                    .text = c.parameterSize,
                                    .badgeColor = d.badgeBackground.opacity(0.55f),
                                    .textColor = d.badgeForeground,
                                },
                            }
                        },
                        HStack{
                            .spacing = 8.0f,
                            .children = {
                                DropdownMenu{
                                    .label = std::string("Download"),
                                    .bgColor = d.inputBackground,
                                    .dropdownBgColor = d.surface,
                                    .borderColor_ = d.borderStrong,
                                    .textColor_ = d.inputForeground,
                                    .mutedColor = d.secondaryForeground,
                                    .items = std::vector<DropdownMenuItem>{
                                        {.label = "Q4_K_M", .subtitle = "~4.6 GB", .onClick = [this, c]() {
                                            startDownload(c.repoId, "Q4_K_M", c.sizeBytes);
                                        }},
                                        {.label = "Q5_K_M", .subtitle = "~5.3 GB", .onClick = [this, c]() {
                                            startDownload(c.repoId, "Q5_K_M", c.sizeBytes);
                                        }},
                                        {.label = "Q8_0", .subtitle = "~7.7 GB", .onClick = [this, c]() {
                                            startDownload(c.repoId, "Q8_0", c.sizeBytes);
                                        }}
                                    }
                                },
                                Button{
                                    .text = std::string("View on HF"),
                                    .backgroundColor = Colors::transparent,
                                    .textColor = d.foreground,
                                    .padding = EdgeInsets(6, 12, 6, 12),
                                    .cornerRadius = 4.0f,
                                    .borderColor = d.borderStrong,
                                    .borderWidth = 1.0f
                                }
                            }
                        }
                    }
                },
                VStack{
                    .spacing = 4.0f,
                    .alignItems = AlignItems::end,
                    .children = {
                        Text{
                            .value = sizeStr,
                            .fontSize = Typography::subheadline,
                            .fontWeight = FontWeight::semibold,
                            .color = d.secondaryForeground,
                            .horizontalAlignment = HorizontalAlignment::trailing
                        }
                    }
                }
            }
        };
    }

private:
    void startDownload(const std::string& repoId, const std::string& quant, uint64_t size) const {
        if (!state) return;
        state->activeDownload = DownloadJob{
            .modelId = repoId,
            .variant = quant,
            .progress = 0.0f,
            .totalBytes = size
        };

        std::thread([this, repoId, quant, size]() {
            for (int i = 0; i <= 100; i++) {
                auto dl = state->activeDownload.get();
                if (!dl.has_value()) break;
                state->activeDownload = DownloadJob{
                    .modelId = repoId,
                    .variant = quant,
                    .progress = i / 100.0f,
                    .speedBytesPerSec = 2.1e9f,
                    .totalBytes = size,
                    .downloadedBytes = static_cast<uint64_t>(size * i / 100.0)
                };
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }

            auto models = static_cast<std::vector<ModelInfo>>(state->installedModels);
            std::string name = repoId;
            auto slash = name.find('/');
            if (slash != std::string::npos) name = name.substr(slash + 1);
            models.push_back(ModelInfo{
                .id = repoId,
                .name = name,
                .quantization = quant,
                .type = "text",
                .sizeBytes = size,
                .isLoaded = false
            });
            state->installedModels = std::move(models);
            state->activeDownload = std::optional<DownloadJob>(std::nullopt);
        }).detach();
    }
};

struct HubView {
    FLUX_VIEW_PROPERTIES;

    AppState* state = nullptr;

    View body() const {
        if (!state) return VStack{};

        Theme d = Theme::dark();
        std::vector<ModelCard> results = state->searchResults;
        std::optional<DownloadJob> dl = state->activeDownload;

        std::vector<View> resultViews;
        for (const auto& card : results) {
            resultViews.push_back(ModelCardView{
                .card = card,
                .state = state
            });
        }

        if (resultViews.empty()) {
            resultViews.push_back(Text{
                .value = std::string("Search for models on HuggingFace above."),
                .color = d.secondaryForeground,
                .padding = 32.0f
            });
        }

        std::vector<View> mainChildren;

        mainChildren.push_back(HStack{
            .spacing = 8.0f,
            .alignItems = AlignItems::center,
            .padding = EdgeInsets(16.0f),
            .children = {
                TextInput{
                    .value = state->hubSearchQuery,
                    .placeholder = std::string("Search HuggingFace models..."),
                    .inputWidth = 400.0f,
                    .expansionBias = 1.0f,
                    .onValueChange = [this](const std::string& val) {
                        state->hubSearchQuery = val;
                    },
                    .onReturn = [this]() {
                        populateSearchResults();
                    }
                },
                Button{
                    .text = std::string("Search"),
                    .backgroundColor = d.accent,
                    .textColor = d.onAccent,
                    .padding = EdgeInsets(8, 16, 8, 16),
                    .cornerRadius = 4.0f,
                    .onClick = [this]() { populateSearchResults(); }
                }
            }
        });

        mainChildren.push_back(HStack{
            .spacing = 8.0f,
            .alignItems = AlignItems::center,
            .padding = EdgeInsets(4.0f, 16.0f, 8.0f, 16.0f),
            .children = {
                Badge{
                    .text = std::string("Text Gen"),
                    .badgeColor = d.accent.opacity(0.22f),
                    .textColor = d.foreground,
                },
                Badge{
                    .text = std::string("GGUF"),
                    .badgeColor = d.surfaceElevated,
                    .textColor = d.foreground,
                },
                Badge{
                    .text = std::string("\xE2\x89\xA4 7B"),
                    .badgeColor = d.surfaceElevated,
                    .textColor = d.foreground,
                }
            }
        });

        mainChildren.push_back(Divider{.borderColor = d.border});

        mainChildren.push_back(ScrollArea{
            .expansionBias = 1.0f,
            .children = {
                VStack{
                    .spacing = 12.0f,
                    .padding = 16.0f,
                    .expansionBias = 1.0f,
                    .children = std::move(resultViews)
                }
            }
        });

        if (dl.has_value()) {
            mainChildren.push_back(Divider{.borderColor = d.border});
            mainChildren.push_back(VStack{
                .spacing = 8.0f,
                .backgroundColor = d.surface,
                .padding = EdgeInsets(12.0f, 16.0f, 12.0f, 16.0f),
                .children = {
                    HStack{
                        .spacing = 8.0f,
                        .alignItems = AlignItems::center,
                        .children = {
                            Text{
                                .value = std::string("Downloading: ") + dl->modelId + " " + dl->variant,
                                .fontSize = Typography::subheadline,
                                .fontWeight = FontWeight::medium,
                                .horizontalAlignment = HorizontalAlignment::leading,
                                .expansionBias = 1.0f
                            },
                            Text{
                                .value = std::format("{:.0f}%", dl->progress * 100),
                                .fontSize = Typography::subheadline,
                                .fontWeight = FontWeight::bold,
                                .color = d.accent
                            },
                            Button{
                                .text = std::string("Cancel"),
                                .backgroundColor = Colors::red.opacity(0.2f),
                                .padding = EdgeInsets(4, 8, 4, 8),
                                .cornerRadius = 4.0f,
                                .onClick = [this]() {
                                    state->activeDownload = std::optional<DownloadJob>(std::nullopt);
                                }
                            }
                        }
                    },
                    ProgressBar{
                        .value = dl->progress,
                        .height = 6.0f,
                        .fillColor = d.accent,
                        .trackColor = d.border
                    }
                }
            });
        }

        return VStack{
            .spacing = 0.0f,
            .expansionBias = 1.0f,
            .children = std::move(mainChildren)
        };
    }

private:
    void populateSearchResults() const {
        if (!state) return;
        state->searchResults = std::vector<ModelCard>{
            ModelCard{
                .repoId = "meta-llama/Llama-3.1-8B-Instruct",
                .name = "Llama 3.1 8B Instruct",
                .likes = 24000,
                .format = "GGUF",
                .parameterSize = "8B",
                .sizeBytes = 4600000000ULL
            },
            ModelCard{
                .repoId = "mistralai/Mistral-7B-Instruct-v0.3",
                .name = "Mistral 7B Instruct v0.3",
                .likes = 18000,
                .format = "GGUF",
                .parameterSize = "7B",
                .sizeBytes = 4100000000ULL
            },
            ModelCard{
                .repoId = "Qwen/Qwen2.5-7B-Instruct",
                .name = "Qwen 2.5 7B Instruct",
                .likes = 12000,
                .format = "GGUF",
                .parameterSize = "7B",
                .sizeBytes = 4300000000ULL
            },
            ModelCard{
                .repoId = "google/gemma-2-9b-it",
                .name = "Gemma 2 9B Instruct",
                .likes = 9500,
                .format = "GGUF",
                .parameterSize = "9B",
                .sizeBytes = 5200000000ULL
            },
            ModelCard{
                .repoId = "microsoft/Phi-3.5-mini-instruct",
                .name = "Phi 3.5 Mini Instruct",
                .likes = 7800,
                .format = "GGUF",
                .parameterSize = "3.8B",
                .sizeBytes = 2200000000ULL
            }
        };
    }
};

} // namespace llm_studio
