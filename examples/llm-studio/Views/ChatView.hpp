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
#include <Flux/Views/Divider.hpp>
#include "../Theme.hpp"
#include "../AppState.hpp"
#include <Flux/Views/TextArea.hpp>
#include "../Components/ChatBubble.hpp"
#include <chrono>

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
            .spacing = Theme::Space1,
            .backgroundColor = Theme::SurfaceRaised,
            .padding = EdgeInsets(Theme::Space3, Theme::Space4, Theme::Space3, Theme::Space4),
            .cornerRadius = Theme::RadiusCard,
            .borderColor = Theme::Border,
            .borderWidth = 1.0f,
            .minWidth = 200.0f,
            .children = {
                Text{
                    .value = m.name,
                    .fontSize = Theme::FontBody,
                    .fontWeight = FontWeight::semibold,
                    .color = Theme::TextPrimary,
                    .horizontalAlignment = HorizontalAlignment::leading
                },
                HStack{
                    .spacing = Theme::Space2,
                    .children = {
                        Text{
                            .value = m.quantization,
                            .fontSize = Theme::FontCaption,
                            .color = Theme::TextMuted
                        },
                        Text{
                            .value = std::string("\xC2\xB7"),
                            .fontSize = Theme::FontCaption,
                            .color = Theme::TextMuted
                        },
                        Text{
                            .value = sizeStr,
                            .fontSize = Theme::FontCaption,
                            .color = Theme::TextMuted
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
        std::string inputVal = state->chatInput;
        std::vector<ModelInfo> installedModels = state->installedModels;

        bool hasSession = session.has_value();
        bool hasModel = hasSession && session->model.has_value();
        std::vector<ChatMessage> msgs = hasSession ? session->messages : std::vector<ChatMessage>{};

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
                .spacing = Theme::Space2,
                .padding = EdgeInsets(Theme::Space12),
                .expansionBias = 1.0f,
                .children = {
                    Text{
                        .value = std::string("LLM Studio"),
                        .fontSize = Theme::FontLargeTitle,
                        .fontWeight = FontWeight::bold,
                        .color = Theme::TextMuted.opacity(0.3f)
                    },
                    Text{
                        .value = std::string("Model: ") + modelName + ". Start a conversation.",
                        .fontSize = Theme::FontBody,
                        .color = Theme::TextMuted
                    }
                }
            });
        }

        std::string sendLabel = generating ? "Stop" : "Send";
        Color sendColor = generating ? Theme::Destructive : Theme::Accent;

        return VStack{
            .spacing = 0.0f,
            .expansionBias = 1.0f,
            .children = {
                ScrollArea{
                    .padding = EdgeInsets(Theme::Space2),
                    .expansionBias = 1.0f,
                    .children = {
                        VStack{
                            .spacing = Theme::Space2,
                            .children = std::move(msgViews)
                        }
                    }
                },

                Divider{.borderColor = Theme::Border},

                VStack{
                    .spacing = Theme::Space2,
                    .backgroundColor = Theme::Surface,
                    .padding = EdgeInsets(Theme::Space3, Theme::Space4, Theme::Space3, Theme::Space4),
                    .children = {
                        HStack{
                            .spacing = Theme::Space2,
                            .alignItems = AlignItems::end,
                            .children = {
                                TextArea{
                                    .value = state->chatInput,
                                    .placeholder = std::string("Type a message..."),
                                    .fontSize = Theme::FontBody,
                                    .areaMinHeight = 40.0f,
                                    .areaMaxHeight = 150.0f,
                                    .bgColor = Theme::Background,
                                    .expansionBias = 1.0f,
                                    .onValueChange = [this](const std::string& val) {
                                        state->chatInput = val;
                                    },
                                    .onSubmit = [this]() { sendMessage(); }
                                },
                                Button{
                                    .text = sendLabel,
                                    .backgroundColor = sendColor,
                                    .padding = EdgeInsets(10, 20, 10, 20),
                                    .cornerRadius = Theme::RadiusCard,
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
        };
    }

private:
    View buildWelcomeView(const std::vector<ModelInfo>& models) const {
        return VStack{
            .spacing = Theme::Space4,
            .padding = EdgeInsets(Theme::Space12),
            .expansionBias = 1.0f,
            .children = {
                Spacer{},
                Text{
                    .value = std::string("LLM Studio"),
                    .fontSize = Theme::FontLargeTitle,
                    .fontWeight = FontWeight::bold,
                    .color = Theme::TextMuted.opacity(0.3f)
                },
                Text{
                    .value = std::string("Create a new chat from the sidebar to get started."),
                    .fontSize = Theme::FontBody,
                    .color = Theme::TextMuted
                },
                Spacer{}
            }
        };
    }

    View buildModelPickerView(const std::vector<ModelInfo>& models) const {
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
                .spacing = Theme::Space2,
                .children = {
                    Text{
                        .value = std::string("No models installed."),
                        .fontSize = Theme::FontBody,
                        .color = Theme::TextMuted
                    },
                    Button{
                        .text = std::string("Browse Models"),
                        .backgroundColor = Theme::Accent,
                        .padding = EdgeInsets(8, 16, 8, 16),
                        .cornerRadius = Theme::RadiusSmall,
                        .onClick = [this]() { state->currentPage = AppPage::MODELS; }
                    }
                }
            });
        }

        return VStack{
            .spacing = Theme::Space4,
            .padding = EdgeInsets(Theme::Space12),
            .expansionBias = 1.0f,
            .children = {
                Spacer{},
                Text{
                    .value = std::string("Select a Model"),
                    .fontSize = Theme::FontTitle1,
                    .fontWeight = FontWeight::bold,
                    .color = Theme::TextPrimary
                },
                Text{
                    .value = std::string("Choose a model to start chatting."),
                    .fontSize = Theme::FontBody,
                    .color = Theme::TextMuted,
                    .padding = EdgeInsets(0, 0, Theme::Space4, 0)
                },
                HStack{
                    .spacing = Theme::Space3,
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
