#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Views/VStack.hpp>
#include <Flux/Views/HStack.hpp>
#include <Flux/Views/Text.hpp>
#include <Flux/Views/Button.hpp>
#include <Flux/Views/Spacer.hpp>
#include <Flux/Views/ScrollArea.hpp>
#include <Flux/Views/ProgressBar.hpp>
#include <Flux/Views/Divider.hpp>
#include <Flux/Views/Badge.hpp>
#include "../Theme.hpp"
#include "../AppState.hpp"
#include <Flux/Views/TextInput.hpp>
#include <Flux/Views/DropdownMenu.hpp>
#include <format>

namespace llm_studio {

using namespace flux;

struct ModelCardView {
    FLUX_VIEW_PROPERTIES;

    Property<ModelCard> card;
    AppState* state = nullptr;

    View body() const {
        ModelCard c = card;

        std::string sizeStr = formatBytes(c.sizeBytes);
        std::string likesStr = std::format("\xE2\x98\x85 {}k", c.likes / 1000);

        return VStack{
            .spacing = Theme::Space2,
            .backgroundColor = Theme::SurfaceRaised,
            .padding = Theme::Space4,
            .cornerRadius = Theme::RadiusCard,
            .borderColor = Theme::Border,
            .borderWidth = 1.0f,
            .children = {
                Text{
                    .value = c.repoId,
                    .fontSize = Theme::FontBody,
                    .fontWeight = FontWeight::semibold,
                    .color = Theme::TextPrimary,
                    .horizontalAlignment = HorizontalAlignment::leading
                },
                HStack{
                    .spacing = Theme::Space3,
                    .alignItems = AlignItems::center,
                    .children = {
                        Badge{
                            .text = likesStr,
                            .badgeColor = Color(0.3f, 0.3f, 0.1f),
                            .textColor = Color(1.0f, 0.85f, 0.0f),
                            .fontSize = Theme::FontBadge
                        },
                        Badge{
                            .text = c.format,
                            .badgeColor = Theme::Accent.opacity(0.2f),
                            .textColor = Theme::Accent,
                            .fontSize = Theme::FontBadge
                        },
                        Badge{
                            .text = c.parameterSize,
                            .badgeColor = Theme::Surface,
                            .textColor = Theme::TextMuted,
                            .fontSize = Theme::FontBadge
                        },
                        Text{
                            .value = sizeStr,
                            .fontSize = Theme::FontCaption,
                            .color = Theme::TextMuted
                        }
                    }
                },
                HStack{
                    .spacing = Theme::Space2,
                    .children = {
                        DropdownMenu{
                            .label = std::string("Download"),
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
                            .padding = EdgeInsets(6, 12, 6, 12),
                            .cornerRadius = Theme::RadiusSmall,
                            .borderColor = Theme::Border,
                            .borderWidth = 1.0f
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

        std::vector<ModelCard> results = state->searchResults;
        std::optional<DownloadJob> dl = state->activeDownload;
        std::string query = state->hubSearchQuery;

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
                .fontSize = Theme::FontBody,
                .color = Theme::TextMuted,
                .padding = Theme::Space8
            });
        }

        std::vector<View> mainChildren;

        mainChildren.push_back(HStack{
            .spacing = Theme::Space2,
            .alignItems = AlignItems::center,
            .padding = EdgeInsets(Theme::Space4),
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
                    .backgroundColor = Theme::Accent,
                    .padding = EdgeInsets(8, 16, 8, 16),
                    .cornerRadius = Theme::RadiusSmall,
                    .onClick = [this]() { populateSearchResults(); }
                }
            }
        });

        mainChildren.push_back(HStack{
            .spacing = Theme::Space2,
            .padding = EdgeInsets(0, Theme::Space4, Theme::Space2, Theme::Space4),
            .children = {
                Badge{
                    .text = std::string("Text Gen"),
                    .badgeColor = Theme::Accent.opacity(0.2f),
                    .textColor = Theme::Accent,
                    .fontSize = Theme::FontBadge
                },
                Badge{
                    .text = std::string("GGUF"),
                    .badgeColor = Theme::SurfaceRaised,
                    .textColor = Theme::TextMuted,
                    .fontSize = Theme::FontBadge
                },
                Badge{
                    .text = std::string("\xE2\x89\xA4 7B"),
                    .badgeColor = Theme::SurfaceRaised,
                    .textColor = Theme::TextMuted,
                    .fontSize = Theme::FontBadge
                }
            }
        });

        mainChildren.push_back(Divider{.borderColor = Theme::Border});

        mainChildren.push_back(ScrollArea{
            .expansionBias = 1.0f,
            .children = {
                VStack{
                    .spacing = Theme::Space3,
                    .padding = Theme::Space4,
                    .expansionBias = 1.0f,
                    .children = std::move(resultViews)
                }
            }
        });

        if (dl.has_value()) {
            mainChildren.push_back(Divider{.borderColor = Theme::Border});
            mainChildren.push_back(VStack{
                .spacing = Theme::Space2,
                .backgroundColor = Theme::Surface,
                .padding = EdgeInsets(Theme::Space3, Theme::Space4, Theme::Space3, Theme::Space4),
                .children = {
                    HStack{
                        .spacing = Theme::Space2,
                        .alignItems = AlignItems::center,
                        .children = {
                            Text{
                                .value = std::string("Downloading: ") + dl->modelId + " " + dl->variant,
                                .fontSize = Theme::FontCaption,
                                .fontWeight = FontWeight::medium,
                                .color = Theme::TextPrimary,
                                .horizontalAlignment = HorizontalAlignment::leading,
                                .expansionBias = 1.0f
                            },
                            Text{
                                .value = std::format("{:.0f}%", dl->progress * 100),
                                .fontSize = Theme::FontCaption,
                                .fontWeight = FontWeight::bold,
                                .color = Theme::Accent
                            },
                            Button{
                                .text = std::string("Cancel"),
                                .backgroundColor = Theme::Destructive.opacity(0.2f),
                                .padding = EdgeInsets(4, 8, 4, 8),
                                .cornerRadius = Theme::RadiusSmall,
                                .onClick = [this]() {
                                    state->activeDownload = std::optional<DownloadJob>(std::nullopt);
                                }
                            }
                        }
                    },
                    ProgressBar{
                        .value = dl->progress,
                        .height = 6.0f,
                        .fillColor = Theme::Accent,
                        .trackColor = Theme::Border
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
