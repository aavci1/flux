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

        std::string sendLabel = generating ? "Stop" : "Send";
        Color sendColor = generating ? Colors::red : d.accent;

        return VStack{
            .spacing = 0.0f,
            .expansionBias = 1.0f,
            .children = {
                ScrollArea{
                    .padding = EdgeInsets(8.0f),
                    .expansionBias = 1.0f,
                    .children = {
                        VStack{
                            .spacing = 8.0f,
                            .children = std::move(msgViews)
                        }
                    }
                },

                Divider{.borderColor = d.border},

                VStack{
                    .spacing = 8.0f,
                    .backgroundColor = d.surface,
                    .padding = EdgeInsets(12.0f, 16.0f, 12.0f, 16.0f),
                    .children = {
                        HStack{
                            .spacing = 8.0f,
                            .alignItems = AlignItems::end,
                            .children = {
                                TextArea{
                                    .value = state->chatInput,
                                    .placeholder = std::string("Type a message..."),
                                    .areaMinHeight = 40.0f,
                                    .areaMaxHeight = 150.0f,
                                    .expansionBias = 1.0f,
                                    .onValueChange = [this](const std::string& val) {
                                        state->chatInput = val;
                                    },
                                    .onSubmit = [this]() { sendMessage(); }
                                },
                                Button{
                                    .text = sendLabel,
                                    .backgroundColor = sendColor,
                                    .textColor = generating ? Colors::white : d.onAccent,
                                    .padding = EdgeInsets(10, 20, 10, 20),
                                    .cornerRadius = 8.0f,
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
    View buildWelcomeView(const std::vector<ModelInfo>& /*models*/) const {
        Theme d = Theme::dark();
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
                            .value = std::string("LLM Studio"),
                            .fontWeight = FontWeight::bold,
                            .color = d.foreground
                        },
                        Text{
                            .value = std::string("Create a new chat from the sidebar to get started."),
                            .color = d.secondaryForeground
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
