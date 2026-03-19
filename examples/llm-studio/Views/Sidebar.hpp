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
#include <Flux/Views/Divider.hpp>
#include <Flux/Views/ScrollArea.hpp>
#include "../AppState.hpp"
#include <string>

namespace llm_studio {

using namespace flux;

struct ChatHistoryItem {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;

    Property<ChatSession> session;
    Property<bool> isActive = false;

    void init() {
        cursor = CursorType::Pointer;
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        bool isHovered = ctx.isCurrentViewHovered();
        bool isPressed = ctx.isCurrentViewPressed();
        bool active = isActive;

        if (!active) {
            if (isPressed) {
                ctx.setFillStyle(FillStyle::solid(Colors::lightGray.darken(0.05f)));
                ctx.drawRect(bounds, CornerRadius(0));
            } else if (isHovered) {
                ctx.setFillStyle(FillStyle::solid(Colors::lightGray));
                ctx.drawRect(bounds, CornerRadius(0));
            }
        }
    }

    View body() const {
        ChatSession s = session;
        bool active = isActive;

        Color bg = active ? Colors::blue.opacity(0.15f) : Colors::transparent;
        Color leftBorder = active ? Colors::blue : Colors::transparent;

        std::string title = s.title;
        if (title.empty()) title = "New Chat";

        std::string timeStr = formatTime(s.createdAt);
        std::string msgCount = std::format("{} message{}", s.messages.size(),
            s.messages.size() == 1 ? "" : "s");

        return HStack{
            .spacing = 0.0f,
            .alignItems = AlignItems::stretch,
            .backgroundColor = bg,
            .children = {
                VStack{
                    .backgroundColor = leftBorder,
                    .minWidth = 3.0f,
                    .maxWidth = 3.0f
                },
                VStack{
                    .spacing = 8.0f,
                    .padding = EdgeInsets(8.0f, 12.0f, 8.0f, 12.0f),
                    .expansionBias = 1.0f,
                    .children = {
                        Text{
                            .value = title,
                            .fontWeight = FontWeight::medium,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },
                        HStack{
                            .spacing = 4.0f,
                            .alignItems = AlignItems::center,
                            .children = {
                                Text{
                                    .value = timeStr,
                                    .fontSize = Typography::caption,
                                    .color = Colors::darkGray,
                                    .horizontalAlignment = HorizontalAlignment::leading
                                },
                                Text{
                                    .value = std::string(" \xC2\xB7 "),
                                    .fontSize = Typography::caption,
                                    .color = Colors::darkGray
                                },
                                Text{
                                    .value = msgCount,
                                    .fontSize = Typography::caption,
                                    .color = Colors::darkGray,
                                    .horizontalAlignment = HorizontalAlignment::leading
                                }
                            }
                        }
                    }
                }
            }
        };
    }
};

struct SidebarView {
    FLUX_VIEW_PROPERTIES;

    AppState* state = nullptr;

    View body() const {
        if (!state) return VStack{};

        bool expanded = state->leftSidebarExpanded;
        float sidebarW = expanded ? 260.0f : 48.0f;

        if (!expanded) {
            return VStack{
                .backgroundColor = Colors::lightGray,
                .borderColor = Colors::gray,
                .borderWidth = 1.0f,
                .minWidth = sidebarW,
                .maxWidth = sidebarW,
                .children = {
                    Button{
                        .text = std::string("\xE2\x89\xA1"),
                        .backgroundColor = Colors::transparent,
                        .textColor = Colors::black,
                        .padding = EdgeInsets(12),
                        .onClick = [this]() { state->leftSidebarExpanded = true; }
                    }
                }
            };
        }

        std::vector<ChatSession> sessions = state->chatSessions;
        std::string activeId = state->activeChatId;

        std::vector<View> sessionViews;
        for (const auto& session : sessions) {
            bool isActive = session.id == activeId;
            ChatSession captured = session;
            sessionViews.push_back(ChatHistoryItem{
                .session = captured,
                .isActive = isActive,
                .onClick = [this, captured]() {
                    state->activeChatId = captured.id;
                    state->currentPage = AppPage::CHAT;
                }
            });
        }

        View listContent = sessionViews.empty()
            ? View(VStack{
                .spacing = 16.0f,
                .padding = 24.0f,
                .expansionBias = 1.0f,
                .children = {
                    VStack{
                        .spacing = 24.0f,
                        .children = {
                            Text{
                                .value = std::string("No chat history"),
                                .color = Colors::darkGray.opacity(0.55f)
                            },
                            Text{
                                .value = std::string("Start a new chat to begin."),
                                .fontSize = Typography::subheadline,
                                .color = Colors::darkGray.opacity(0.45f)
                            }
                        }
                    }
                }
            })
            : View(ScrollArea{
                .expansionBias = 1.0f,
                .children = std::move(sessionViews)
            });

        return VStack{
            .spacing = 0.0f,
            .backgroundColor = Colors::lightGray,
            .borderColor = Colors::gray,
            .borderWidth = 1.0f,
            .minWidth = sidebarW,
            .maxWidth = sidebarW,
            .children = {
                HStack{
                    .spacing = 8.0f,
                    .justifyContent = JustifyContent::spaceBetween,
                    .alignItems = AlignItems::center,
                    .padding = EdgeInsets(12.0f),
                    .children = {
                        Text{
                            .value = std::string("Chats"),
                            .fontSize = Typography::subheadline,
                            .fontWeight = FontWeight::semibold,
                            .color = Colors::darkGray,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },
                        Button{
                            .text = std::string("\xC3\x97"),
                            .backgroundColor = Colors::transparent,
                            .textColor = Colors::black,
                            .padding = EdgeInsets(4),
                            .onClick = [this]() { state->leftSidebarExpanded = false; }
                        }
                    }
                },

                Button{
                    .text = std::string("+ New Chat"),
                    .backgroundColor = Colors::blue,
                    .padding = EdgeInsets(8, 12, 8, 12),
                    .cornerRadius = 4.0f,
                    .onClick = [this]() { state->createNewChat(); }
                },

                Divider{.borderColor = Colors::gray},

                listContent
            }
        };
    }
};

} // namespace llm_studio
