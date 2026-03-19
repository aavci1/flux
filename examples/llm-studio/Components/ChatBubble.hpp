#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/Typography.hpp>
#include <Flux/Views/VStack.hpp>
#include <Flux/Views/HStack.hpp>
#include <Flux/Views/Text.hpp>
#include <Flux/Views/Spacer.hpp>
#include "../AppState.hpp"
#include <Flux/Views/TypingIndicator.hpp>
#include <Flux/Views/CodeBlock.hpp>
#include <string>
#include <vector>
#include <regex>

namespace llm_studio {

using namespace flux;

struct ChatBubble {
    FLUX_VIEW_PROPERTIES;

    Property<ChatMessage> message;
    Property<bool> isStreaming = false;

    View body() const {
        ChatMessage msg = message;
        bool streaming = isStreaming;
        bool isUser = msg.role == ChatMessage::Role::User;

        std::string roleLabel = isUser ? "you" : "assistant";
        std::string timeStr = formatTime(msg.timestamp);

        Color bubbleBg = isUser ? Colors::blue.opacity(0.12f) : Colors::white;

        std::vector<View> contentViews;

        contentViews.push_back(HStack{
            .spacing = 8.0f,
            .justifyContent = JustifyContent::spaceBetween,
            .children = {
                Text{
                    .value = roleLabel,
                    .fontSize = Typography::subheadline,
                    .fontWeight = FontWeight::semibold,
                    .color = isUser ? Colors::blue : Colors::darkGray,
                    .horizontalAlignment = HorizontalAlignment::leading
                },
                Text{
                    .value = timeStr,
                    .fontSize = Typography::caption,
                    .color = Colors::darkGray,
                    .horizontalAlignment = HorizontalAlignment::trailing
                }
            }
        });

        if (streaming && msg.content.empty()) {
            contentViews.push_back(TypingIndicator{});
        } else {
            auto parts = parseContent(msg.content);
            for (const auto& part : parts) {
                if (part.isCode) {
                    contentViews.push_back(CodeBlock{
                        .code = part.text,
                        .language = part.lang,
                        .expansionBias = 1.0f
                    });
                } else {
                    contentViews.push_back(Text{
                        .value = part.text,
                        .horizontalAlignment = HorizontalAlignment::leading
                    });
                }
            }
        }

        std::vector<View> rowChildren;
        if (!isUser) {
            rowChildren.push_back(VStack{
                .spacing = 8.0f,
                .backgroundColor = bubbleBg,
                .padding = EdgeInsets(12.0f, 16.0f, 12.0f, 16.0f),
                .cornerRadius = 8.0f,
                .borderColor = Colors::gray,
                .borderWidth = 1.0f,
                .expansionBias = 1.0f,
                .maxWidth = 600.0f,
                .children = std::move(contentViews)
            });
            rowChildren.push_back(Spacer{});
        } else {
            rowChildren.push_back(Spacer{});
            rowChildren.push_back(VStack{
                .spacing = 8.0f,
                .backgroundColor = bubbleBg,
                .padding = EdgeInsets(12.0f, 16.0f, 12.0f, 16.0f),
                .cornerRadius = 8.0f,
                .borderColor = Colors::gray,
                .borderWidth = 1.0f,
                .expansionBias = 1.0f,
                .maxWidth = 600.0f,
                .children = std::move(contentViews)
            });
        }

        return HStack{
            .spacing = 0.0f,
            .padding = EdgeInsets(4.0f, 16.0f, 4.0f, 16.0f),
            .children = std::move(rowChildren)
        };
    }

private:
    struct ContentPart {
        std::string text;
        std::string lang;
        bool isCode = false;
    };

    static std::vector<ContentPart> parseContent(const std::string& content) {
        std::vector<ContentPart> parts;
        size_t pos = 0;
        while (pos < content.size()) {
            size_t fenceStart = content.find("```", pos);
            if (fenceStart == std::string::npos) {
                std::string remaining = content.substr(pos);
                if (!remaining.empty()) parts.push_back({remaining, "", false});
                break;
            }

            if (fenceStart > pos) {
                parts.push_back({content.substr(pos, fenceStart - pos), "", false});
            }

            size_t langEnd = content.find('\n', fenceStart + 3);
            if (langEnd == std::string::npos) {
                parts.push_back({content.substr(fenceStart), "", false});
                break;
            }

            std::string lang = content.substr(fenceStart + 3, langEnd - fenceStart - 3);

            size_t fenceEnd = content.find("```", langEnd + 1);
            if (fenceEnd == std::string::npos) {
                parts.push_back({content.substr(langEnd + 1), lang, true});
                break;
            }

            parts.push_back({content.substr(langEnd + 1, fenceEnd - langEnd - 1), lang, true});
            pos = fenceEnd + 3;
            if (pos < content.size() && content[pos] == '\n') pos++;
        }

        if (parts.empty()) parts.push_back({"", "", false});
        return parts;
    }
};

} // namespace llm_studio
