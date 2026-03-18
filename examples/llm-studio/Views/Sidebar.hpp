#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Views/VStack.hpp>
#include <Flux/Views/HStack.hpp>
#include <Flux/Views/Text.hpp>
#include <Flux/Views/Button.hpp>
#include <Flux/Views/Spacer.hpp>
#include <Flux/Views/Divider.hpp>
#include <Flux/Views/ScrollArea.hpp>
#include "../Theme.hpp"
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
                ctx.setFillStyle(FillStyle::solid(Theme::SurfaceRaised.darken(0.05f)));
                ctx.drawRect(bounds, CornerRadius(0));
            } else if (isHovered) {
                ctx.setFillStyle(FillStyle::solid(Theme::SurfaceRaised));
                ctx.drawRect(bounds, CornerRadius(0));
            }
        }
    }

    View body() const {
        ChatSession s = session;
        bool active = isActive;

        Color bg = active ? Theme::Accent.opacity(0.15f) : Colors::transparent;
        Color leftBorder = active ? Theme::Accent : Colors::transparent;

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
                    .spacing = 2.0f,
                    .padding = EdgeInsets(Theme::Space2, Theme::Space3, Theme::Space2, Theme::Space3),
                    .expansionBias = 1.0f,
                    .children = {
                        Text{
                            .value = title,
                            .fontSize = Theme::FontBody,
                            .fontWeight = FontWeight::medium,
                            .color = Theme::TextPrimary,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },
                        HStack{
                            .spacing = Theme::Space1,
                            .alignItems = AlignItems::center,
                            .children = {
                                Text{
                                    .value = timeStr,
                                    .fontSize = Theme::FontCaption,
                                    .color = Theme::TextMuted,
                                    .horizontalAlignment = HorizontalAlignment::leading
                                },
                                Text{
                                    .value = std::string(" \xC2\xB7 "),
                                    .fontSize = Theme::FontCaption,
                                    .color = Theme::TextMuted
                                },
                                Text{
                                    .value = msgCount,
                                    .fontSize = Theme::FontCaption,
                                    .color = Theme::TextMuted,
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
                .backgroundColor = Theme::Surface,
                .borderColor = Theme::Border,
                .borderWidth = 1.0f,
                .minWidth = sidebarW,
                .maxWidth = sidebarW,
                .children = {
                    Button{
                        .text = std::string("\xE2\x89\xA1"),
                        .backgroundColor = Colors::transparent,
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
                .spacing = Theme::Space2,
                .padding = Theme::Space6,
                .expansionBias = 1.0f,
                .children = {
                    Text{
                        .value = std::string("No chat history"),
                        .fontSize = Theme::FontBody,
                        .color = Theme::TextMuted.opacity(0.5f)
                    },
                    Text{
                        .value = std::string("Start a new chat to begin."),
                        .fontSize = Theme::FontCaption,
                        .color = Theme::TextMuted.opacity(0.3f)
                    }
                }
            })
            : View(ScrollArea{
                .expansionBias = 1.0f,
                .children = std::move(sessionViews)
            });

        return VStack{
            .spacing = 0.0f,
            .backgroundColor = Theme::Surface,
            .borderColor = Theme::Border,
            .borderWidth = 1.0f,
            .minWidth = sidebarW,
            .maxWidth = sidebarW,
            .children = {
                HStack{
                    .spacing = Theme::Space2,
                    .justifyContent = JustifyContent::spaceBetween,
                    .alignItems = AlignItems::center,
                    .padding = EdgeInsets(Theme::Space3),
                    .children = {
                        Text{
                            .value = std::string("Chats"),
                            .fontSize = Theme::FontCaption,
                            .fontWeight = FontWeight::semibold,
                            .color = Theme::TextMuted,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },
                        Button{
                            .text = std::string("\xC3\x97"),
                            .backgroundColor = Colors::transparent,
                            .padding = EdgeInsets(4),
                            .onClick = [this]() { state->leftSidebarExpanded = false; }
                        }
                    }
                },

                Button{
                    .text = std::string("+ New Chat"),
                    .backgroundColor = Theme::Accent,
                    .padding = EdgeInsets(8, Theme::Space3, 8, Theme::Space3),
                    .cornerRadius = Theme::RadiusSmall,
                    .onClick = [this]() { state->createNewChat(); }
                },

                Divider{.borderColor = Theme::Border},

                listContent
            }
        };
    }
};

} // namespace llm_studio
