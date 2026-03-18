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
#include <Flux/Views/Slider.hpp>
#include <Flux/Views/Divider.hpp>
#include "../Theme.hpp"
#include "../AppState.hpp"
#include "../Components/TextArea.hpp"
#include "../Components/ChatBubble.hpp"
#include <chrono>

namespace llm_studio {

using namespace flux;

struct ChatView {
    FLUX_VIEW_PROPERTIES;

    AppState* state = nullptr;

    View body() const {
        if (!state) return View(VStack{});

        std::vector<ChatMessage> msgs = state->messages;
        bool generating = state->isGenerating;
        bool showClear = state->showClearDialog;
        std::string inputVal = state->chatInput;
        std::optional<ModelInfo> active = state->activeModel;

        std::vector<View> msgViews;
        for (const auto& msg : msgs) {
            msgViews.push_back(View(ChatBubble{
                .message = msg,
                .isStreaming = false
            }));
        }

        if (generating) {
            ChatMessage streamMsg;
            streamMsg.role = ChatMessage::Role::Assistant;
            streamMsg.content = state->streamingToken;
            streamMsg.timestamp = std::chrono::system_clock::now();
            msgViews.push_back(View(ChatBubble{
                .message = streamMsg,
                .isStreaming = true
            }));
        }

        if (msgViews.empty()) {
            msgViews.push_back(View(VStack{
                .spacing = Theme::Space2,
                .padding = EdgeInsets(Theme::Space12),
                .expansionBias = 1.0f,
                .children = {
                    View(Text{
                        .value = std::string("LLM Studio"),
                        .fontSize = 28.0f,
                        .fontWeight = FontWeight::bold,
                        .color = Theme::TextMuted.opacity(0.3f)
                    }),
                    View(Text{
                        .value = active.has_value()
                            ? std::string("Model loaded. Start a conversation.")
                            : std::string("Select a model from the sidebar to begin."),
                        .fontSize = Theme::FontBody,
                        .color = Theme::TextMuted
                    })
                }
            }));
        }

        std::string sendLabel = generating ? "Stop" : "Send";
        Color sendColor = generating ? Theme::Destructive : Theme::Accent;

        AppSettings settings = state->settings;

        View chatContent = View(VStack{
            .spacing = 0.0f,
            .expansionBias = 1.0f,
            .children = {
                View(ScrollArea{
                    .padding = EdgeInsets(Theme::Space2),
                    .expansionBias = 1.0f,
                    .children = {
                        View(VStack{
                            .spacing = Theme::Space2,
                            .expansionBias = 1.0f,
                            .children = std::move(msgViews)
                        })
                    }
                }),

                View(Divider{.borderColor = Theme::Border}),

                View(VStack{
                    .spacing = Theme::Space2,
                    .backgroundColor = Theme::Surface,
                    .padding = EdgeInsets(Theme::Space3, Theme::Space4, Theme::Space3, Theme::Space4),
                    .children = {
                        View(HStack{
                            .spacing = Theme::Space2,
                            .alignItems = AlignItems::end,
                            .children = {
                                View(TextArea{
                                    .value = state->chatInput,
                                    .placeholder = std::string("Type a message..."),
                                    .fontSize = Theme::FontBody,
                                    .areaMinHeight = 40.0f,
                                    .areaMaxHeight = 150.0f,
                                    .bgColor = Theme::Background,
                                    .areaWidth = 600.0f,
                                    .expansionBias = 1.0f,
                                    .onValueChange = [this](const std::string& val) {
                                        state->chatInput = val;
                                    },
                                    .onSubmit = [this]() { sendMessage(); }
                                }),
                                View(Button{
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
                                })
                            }
                        }),
                        View(HStack{
                            .spacing = Theme::Space4,
                            .alignItems = AlignItems::center,
                            .children = {
                                View(Button{
                                    .text = std::string("Clear"),
                                    .backgroundColor = Colors::transparent,
                                    .padding = EdgeInsets(4, 8, 4, 8),
                                    .cornerRadius = Theme::RadiusSmall,
                                    .onClick = [this]() { state->showClearDialog = true; }
                                }),
                                View(Spacer{}),
                                View(Text{
                                    .value = std::string("Temp:"),
                                    .fontSize = Theme::FontCaption,
                                    .color = Theme::TextMuted
                                }),
                                View(Slider{
                                    .value = [this]() -> float {
                                        return static_cast<AppSettings>(state->settings).temperature;
                                    },
                                    .minValue = 0.0f,
                                    .maxValue = 2.0f,
                                    .step = 0.1f,
                                    .activeColor = Theme::Accent,
                                    .inactiveColor = Theme::Border,
                                    .maxWidth = 120.0f,
                                    .onChange = [this]() {
                                    }
                                }),
                                View(Text{
                                    .value = [this]() -> std::string {
                                        return std::format("{:.1f}",
                                            static_cast<AppSettings>(state->settings).temperature);
                                    },
                                    .fontSize = Theme::FontCaption,
                                    .fontWeight = FontWeight::medium,
                                    .color = Theme::Accent
                                })
                            }
                        })
                    }
                })
            }
        });

        if (!showClear) return chatContent;

        return View(VStack{
            .spacing = 0.0f,
            .expansionBias = 1.0f,
            .children = {
                View(VStack{
                    .backgroundColor = Color(0.0f, 0.0f, 0.0f, 0.5f),
                    .expansionBias = 1.0f,
                    .children = {
                        View(Spacer{}),
                        View(HStack{
                            .justifyContent = JustifyContent::center,
                            .children = {
                                View(Spacer{}),
                                View(VStack{
                                    .spacing = Theme::Space2,
                                    .backgroundColor = Theme::Surface,
                                    .padding = Theme::Space6,
                                    .cornerRadius = Theme::RadiusDialog,
                                    .borderColor = Theme::Border,
                                    .borderWidth = 1.0f,
                                    .minWidth = 400.0f,
                                    .maxWidth = 400.0f,
                                    .children = {
                                        View(Text{
                                            .value = std::string("Clear Conversation"),
                                            .fontSize = Theme::FontH1,
                                            .fontWeight = FontWeight::bold,
                                            .color = Theme::TextPrimary,
                                            .horizontalAlignment = HorizontalAlignment::leading
                                        }),
                                        View(Text{
                                            .value = std::string("Are you sure you want to clear all messages?"),
                                            .fontSize = Theme::FontBody,
                                            .color = Theme::TextMuted,
                                            .horizontalAlignment = HorizontalAlignment::leading,
                                            .padding = EdgeInsets(4, 0, 12, 0)
                                        }),
                                        View(HStack{
                                            .spacing = Theme::Space2,
                                            .justifyContent = JustifyContent::end,
                                            .children = {
                                                View(Spacer{}),
                                                View(Button{
                                                    .text = std::string("Cancel"),
                                                    .backgroundColor = Theme::SurfaceRaised,
                                                    .padding = EdgeInsets(8, 16, 8, 16),
                                                    .cornerRadius = Theme::RadiusSmall,
                                                    .onClick = [this]() { state->showClearDialog = false; }
                                                }),
                                                View(Button{
                                                    .text = std::string("Clear"),
                                                    .backgroundColor = Theme::Destructive,
                                                    .padding = EdgeInsets(8, 16, 8, 16),
                                                    .cornerRadius = Theme::RadiusSmall,
                                                    .onClick = [this]() {
                                                        state->messages = std::vector<ChatMessage>{};
                                                        state->showClearDialog = false;
                                                    }
                                                })
                                            }
                                        })
                                    }
                                }),
                                View(Spacer{})
                            }
                        }),
                        View(Spacer{})
                    }
                })
            }
        });
    }

private:
    void sendMessage() const {
        if (!state) return;
        std::string input = state->chatInput;
        if (input.empty()) return;

        auto msgs = static_cast<std::vector<ChatMessage>>(state->messages);
        msgs.push_back(ChatMessage{
            .role = ChatMessage::Role::User,
            .content = input,
            .timestamp = std::chrono::system_clock::now()
        });
        state->messages = std::move(msgs);
        state->chatInput = std::string("");

        simulateResponse();
    }

    void simulateResponse() const {
        if (!state) return;
        state->isGenerating = true;
        state->streamingToken = std::string("");

        std::thread([this]() {
            std::string response = "This is a simulated response from the model. "
                "In production, this would stream tokens from llama.cpp via the inference server.\n\n"
                "Here's an example code block:\n\n"
                "```python\ndef hello():\n    print(\"Hello, World!\")\n```\n\n"
                "The LLM Studio UI is working correctly!";

            std::string accumulated;
            for (size_t i = 0; i < response.size(); i++) {
                accumulated += response[i];
                state->streamingToken = accumulated;
                std::this_thread::sleep_for(std::chrono::milliseconds(15));
            }

            auto msgs = static_cast<std::vector<ChatMessage>>(state->messages);
            msgs.push_back(ChatMessage{
                .role = ChatMessage::Role::Assistant,
                .content = accumulated,
                .timestamp = std::chrono::system_clock::now()
            });
            state->messages = std::move(msgs);
            state->isGenerating = false;
            state->streamingToken = std::string("");
        }).detach();
    }
};

} // namespace llm_studio
