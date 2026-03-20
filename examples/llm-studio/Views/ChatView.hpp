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
#include <Flux/Views/Divider.hpp>
#include <Flux/Views/DropdownMenu.hpp>
#include "../AppState.hpp"
#include <Flux/Views/TextArea.hpp>
#include "../Components/ChatBubble.hpp"
#include <chrono>
#include <thread>

namespace llm_studio {

using namespace flux;

struct ModelPickerCard {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;

    Property<ModelInfo> model;

    void init() {
        cursor = CursorType::Pointer;
    }

    View body() const {
        ModelInfo m = model;
        std::string sizeStr = formatBytes(m.sizeBytes);

        return VStack{
            .spacing = 4.0f,
            .backgroundColor = Theme::dark().surfaceElevated,
            .padding = EdgeInsets(12.0f, 16.0f, 12.0f, 16.0f),
            .cornerRadius = 8.0f,
            .borderColor = Theme::dark().borderStrong,
            .borderWidth = 1.0f,
            .minWidth = 200.0f,
            .children = {
                Text{
                    .value = m.name,
                    .fontWeight = FontWeight::semibold,
                    .color = Theme::dark().foreground,
                    .horizontalAlignment = HorizontalAlignment::leading
                },
                HStack{
                    .spacing = 8.0f,
                    .children = {
                        Text{
                            .value = m.quantization,
                            .fontSize = Typography::caption,
                            .color = Theme::dark().secondaryForeground
                        },
                        Text{
                            .value = std::string("\xC2\xB7"),
                            .fontSize = Typography::caption,
                            .color = Theme::dark().secondaryForeground
                        },
                        Text{
                            .value = sizeStr,
                            .fontSize = Typography::caption,
                            .color = Theme::dark().secondaryForeground
                        }
                    }
                }
            }
        };
    }
};

struct ChatView {
    FLUX_VIEW_PROPERTIES;

    AppState* state = nullptr;

    View body() const {
        if (!state) return VStack{};

        auto session = state->getActiveSessionCopy();
        bool generating = state->isGenerating;
        std::vector<ModelInfo> installedModels = state->installedModels;

        bool hasSession = session.has_value();
        bool hasModel = hasSession && session->model.has_value();
        std::vector<ChatMessage> msgs = hasSession ? session->messages : std::vector<ChatMessage>{};

        Theme d = Theme::dark();

        if (!hasSession) {
            return buildWelcomeView(installedModels);
        }

        if (!hasModel && msgs.empty()) {
            return buildModelPickerView(installedModels);
        }

        std::vector<View> msgViews;
        for (const auto& msg : msgs) {
            msgViews.push_back(ChatBubble{
                .message = msg,
                .isStreaming = false
            });
        }

        if (generating) {
            ChatMessage streamMsg;
            streamMsg.role = ChatMessage::Role::Assistant;
            streamMsg.content = state->streamingToken;
            streamMsg.timestamp = std::chrono::system_clock::now();
            msgViews.push_back(ChatBubble{
                .message = streamMsg,
                .isStreaming = true
            });
        }

        if (msgViews.empty()) {
            std::string modelName = hasModel ? session->model->name : "";
            msgViews.push_back(VStack{
                .spacing = 16.0f,
                .padding = EdgeInsets(48.0f),
                .expansionBias = 1.0f,
                .children = {
                    VStack{
                        .spacing = 24.0f,
                        .children = {
                            Text{
                                .value = std::string("LLM Studio"),
                                .fontWeight = FontWeight::bold,
                                .color = d.secondaryForeground
                            },
                            Text{
                                .value = std::string("Model: ") + modelName + ". Start a conversation.",
                                .color = d.foreground
                            }
                        }
                    }
                }
            });
        }

        Color composerBg = d.codeBackground;
        Color composerBorder = d.borderStrong;
        Color sendCircleBg = Color::hex(0x4A4A4A);
        std::string sendGlyph = generating ? std::string("\xE2\x96\xA0") : std::string("\xE2\x86\x91");
        Color sendGlyphColor = generating ? Colors::white : Color::hex(0x1A1A1A);

        std::vector<DropdownMenuItem> agentItems = {
            {.label = "Agent", .subtitle = "Full workspace context", .onClick = []() {}},
            {.label = "Ask", .subtitle = "Quick answers", .onClick = []() {}},
        };

        std::vector<DropdownMenuItem> modelItems = {
            {.label = "Auto", .subtitle = "Pick best for the prompt", .onClick = []() {}},
        };
        if (hasModel && session->model.has_value()) {
            std::string mn = session->model->name;
            modelItems.push_back({.label = mn, .subtitle = "Active session model", .onClick = []() {}});
        }

        return VStack{
            .spacing = 0.0f,
            .expansionBias = 1.0f,
            .children = {
                ScrollArea{
                    .padding = EdgeInsets(8.0f),
                    .expansionBias = 1.0f,
                    .children = {
                        VStack{
                            .spacing = 14.0f,
                            .children = std::move(msgViews)
                        }
                    }
                },

                Divider{.borderColor = d.border},

                VStack{
                    .spacing = 0.0f,
                    .backgroundColor = d.surface,
                    .padding = EdgeInsets(12.0f, 16.0f, 16.0f, 16.0f),
                    .children = {
                        VStack{
                            .spacing = 0.0f,
                            .backgroundColor = composerBg,
                            .borderColor = composerBorder,
                            .borderWidth = 1.0f,
                            .cornerRadius = 12.0f,
                            .padding = EdgeInsets(14.0f, 14.0f, 10.0f, 14.0f),
                            .children = {
                                TextArea{
                                    .value = state->chatInput,
                                    .placeholder = std::string("Ask anything..."),
                                    .fontSize = Typography::callout,
                                    .areaMinHeight = 44.0f,
                                    .areaMaxHeight = 160.0f,
                                    .expansionBias = 1.0f,
                                    .bgColor = composerBg,
                                    .borderCol = composerBg,
                                    .focusBorderColor = composerBorder,
                                    .placeholderColor = d.placeholder,
                                    .textColor = d.foreground,
                                    .areaCornerRadius = 0.0f,
                                    .areaPadding = 0.0f,
                                    .onValueChange = [this](const std::string& val) {
                                        state->chatInput = val;
                                    },
                                    .onSubmit = [this]() { sendMessage(); }
                                },
                                HStack{
                                    .spacing = 0.0f,
                                    .justifyContent = JustifyContent::spaceBetween,
                                    .alignItems = AlignItems::center,
                                    .padding = EdgeInsets(10.0f, 0.0f, 0.0f, 0.0f),
                                    .children = {
                                        HStack{
                                            .spacing = 10.0f,
                                            .alignItems = AlignItems::center,
                                            .children = {
                                                DropdownMenu{
                                                    .label = std::string("\xE2\x88\x9E  Agent"),
                                                    .menuWidth = 200.0f,
                                                    .bgColor = d.inputBackground,
                                                    .dropdownBgColor = d.surfaceElevated,
                                                    .borderColor_ = d.borderStrong,
                                                    .textColor_ = d.foreground,
                                                    .mutedColor = d.secondaryForeground,
                                                    .items = std::move(agentItems)
                                                },
                                                DropdownMenu{
                                                    .label = std::string("Auto"),
                                                    .menuWidth = 220.0f,
                                                    .bgColor = composerBg,
                                                    .dropdownBgColor = d.surfaceElevated,
                                                    .borderColor_ = composerBg,
                                                    .textColor_ = d.secondaryForeground,
                                                    .mutedColor = d.tertiaryForeground,
                                                    .items = std::move(modelItems)
                                                }
                                            }
                                        },
                                        HStack{
                                            .spacing = 10.0f,
                                            .alignItems = AlignItems::center,
                                            .children = {
                                                Button{
                                                    .text = std::string("\xF0\x9F\x96\xBC"),
                                                    .backgroundColor = Colors::transparent,
                                                    .textColor = d.secondaryForeground,
                                                    .padding = EdgeInsets(8.0f, 10.0f, 8.0f, 10.0f),
                                                    .cornerRadius = 8.0f,
                                                    .onClick = []() {}
                                                },
                                                Button{
                                                    .text = sendGlyph,
                                                    .backgroundColor = generating ? d.error : sendCircleBg,
                                                    .textColor = sendGlyphColor,
                                                    .padding = EdgeInsets(0.0f, 0.0f, 0.0f, 0.0f),
                                                    .minWidth = 36.0f,
                                                    .minHeight = 36.0f,
                                                    .cornerRadius = CornerRadius(18.0f),
                                                    .onClick = [this, generating]() {
                                                        if (generating) {
                                                            state->isGenerating = false;
                                                        } else {
                                                            sendMessage();
                                                        }
                                                    }
                                                }
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
    View buildWelcomeView(const std::vector<ModelInfo>& /*models*/) const {
        Theme d = Theme::dark();
        return VStack{
            .spacing = 24.0f,
            .padding = EdgeInsets(48.0f),
            .expansionBias = 1.0f,
            .children = {
                Spacer{},
                VStack{
                    .spacing = 12.0f,
                    .children = {
                        Text{
                            .value = std::string("LLM Studio"),
                            .fontSize = 26.0f,
                            .fontWeight = FontWeight::bold,
                            .color = d.foreground
                        },
                        Text{
                            .value = std::string("Open the chat list and choose New Chat to begin."),
                            .fontSize = Typography::callout,
                            .color = d.secondaryForeground,
                            .lineHeightMultiplier = Typography::lineHeightBody
                        }
                    }
                },
                Spacer{}
            }
        };
    }

    View buildModelPickerView(const std::vector<ModelInfo>& models) const {
        Theme d = Theme::dark();
        std::vector<View> modelCards;
        for (const auto& m : models) {
            ModelInfo captured = m;
            modelCards.push_back(ModelPickerCard{
                .model = captured,
                .onClick = [this, captured]() {
                    state->activeModel = captured;
                    state->loadState = ModelLoadState::READY;
                    state->updateActiveSession([&](ChatSession& s) {
                        s.model = captured;
                    });
                }
            });
        }

        if (modelCards.empty()) {
            modelCards.push_back(VStack{
                .spacing = 8.0f,
                .children = {
                    Text{
                        .value = std::string("No models installed."),
                        .color = d.secondaryForeground
                    },
                    Button{
                        .text = std::string("Browse Models"),
                        .backgroundColor = d.accent,
                        .textColor = d.onAccent,
                        .padding = EdgeInsets(8, 16, 8, 16),
                        .cornerRadius = 4.0f,
                        .onClick = [this]() { state->currentPage = AppPage::MODELS; }
                    }
                }
            });
        }

        return VStack{
            .spacing = 24.0f,
            .padding = EdgeInsets(48.0f),
            .expansionBias = 1.0f,
            .children = {
                Spacer{},
                VStack{
                    .spacing = 24.0f,
                    .children = {
                        Text{
                            .value = std::string("Select a Model"),
                            .fontWeight = FontWeight::bold,
                            .color = d.foreground,
                        },
                        Text{
                            .value = std::string("Choose a model to start chatting."),
                            .color = d.secondaryForeground
                        }
                    },
                    .padding = EdgeInsets(0, 0, 16.0f, 0)
                },
                HStack{
                    .spacing = 12.0f,
                    .justifyContent = JustifyContent::center,
                    .children = std::move(modelCards)
                },
                Spacer{}
            }
        };
    }

    void sendMessage() const {
        if (!state) return;
        std::string input = state->chatInput;
        if (input.empty()) return;

        auto session = state->getActiveSessionCopy();
        if (!session.has_value()) return;

        if (session->title == "New Chat" && session->messages.empty()) {
            std::string title = input.substr(0, 40);
            if (input.size() > 40) title += "...";
            state->updateActiveSession([&](ChatSession& s) {
                s.title = title;
            });
        }

        state->updateActiveSession([&](ChatSession& s) {
            s.messages.push_back(ChatMessage{
                .role = ChatMessage::Role::User,
                .content = input,
                .timestamp = std::chrono::system_clock::now()
            });
        });
        state->chatInput = std::string("");

        simulateResponse();
    }

    void simulateResponse() const {
        if (!state) return;
        AppState* s = state;
        s->isGenerating = true;
        s->streamingToken = std::string("");

        std::thread([s]() {
            std::string response = "This is a simulated response from the model. "
                "In production, this would stream tokens from llama.cpp via the inference server.\n\n"
                "Here's an example code block:\n\n"
                "```python\ndef hello():\n    print(\"Hello, World!\")\n```\n\n"
                "The LLM Studio UI is working correctly!";

            std::string accumulated;
            for (size_t i = 0; i < response.size(); i++) {
                accumulated += response[i];
                s->streamingToken = accumulated;
                std::this_thread::sleep_for(std::chrono::milliseconds(15));
            }

            s->updateActiveSession([&](ChatSession& session) {
                session.messages.push_back(ChatMessage{
                    .role = ChatMessage::Role::Assistant,
                    .content = accumulated,
                    .timestamp = std::chrono::system_clock::now()
                });
            });
            s->isGenerating = false;
            s->streamingToken = std::string("");
        }).detach();
    }
};

} // namespace llm_studio
