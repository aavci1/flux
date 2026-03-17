#pragma once

#include "../AppState.hpp"
#include <string>
#include <chrono>
#include <thread>
#include <format>

namespace llm_studio {

class ActionDispatcher {
public:
    explicit ActionDispatcher(AppState& state) : state_(state) {}

    std::string dispatch(const std::string& action) {
        if (action.starts_with("navigate:")) {
            return navigate(action.substr(9));
        }
        if (action.starts_with("select_model:")) {
            return selectModel(std::stoi(action.substr(13)));
        }
        if (action.starts_with("type_chat:")) {
            state_.chatInput = action.substr(10);
            return R"({"ok":true,"action":"type_chat"})";
        }
        if (action == "send_message") {
            return sendMessage();
        }
        if (action == "clear_chat") {
            state_.messages = std::vector<ChatMessage>{};
            return R"({"ok":true,"action":"clear_chat"})";
        }
        if (action == "toggle_sidebar") {
            state_.sidebarExpanded = !static_cast<bool>(state_.sidebarExpanded);
            return R"({"ok":true,"action":"toggle_sidebar"})";
        }
        if (action.starts_with("search_hub:")) {
            return searchHub(action.substr(11));
        }
        if (action == "show_clear_dialog") {
            state_.showClearDialog = true;
            return R"({"ok":true,"action":"show_clear_dialog"})";
        }
        if (action == "dismiss_clear_dialog") {
            state_.showClearDialog = false;
            return R"({"ok":true,"action":"dismiss_clear_dialog"})";
        }
        if (action == "confirm_clear") {
            state_.messages = std::vector<ChatMessage>{};
            state_.showClearDialog = false;
            return R"({"ok":true,"action":"confirm_clear"})";
        }
        if (action.starts_with("type_hub_search:")) {
            state_.hubSearchQuery = action.substr(16);
            return R"({"ok":true,"action":"type_hub_search"})";
        }
        return R"({"ok":false,"error":"unknown action"})";
    }

private:
    std::string navigate(const std::string& view) {
        if (view == "CHAT") state_.currentView = AppView::CHAT;
        else if (view == "IMAGE") state_.currentView = AppView::IMAGE;
        else if (view == "HUB") state_.currentView = AppView::HUB;
        else if (view == "SETTINGS") state_.currentView = AppView::SETTINGS;
        else return R"({"ok":false,"error":"unknown view"})";
        return std::format(R"({{"ok":true,"action":"navigate","view":"{}"}})", view);
    }

    std::string selectModel(int index) {
        auto models = static_cast<std::vector<ModelInfo>>(state_.installedModels);
        if (index < 0 || index >= static_cast<int>(models.size())) {
            return R"({"ok":false,"error":"model index out of range"})";
        }
        state_.activeModel = models[index];
        state_.loadState = ModelLoadState::READY;
        return std::format(R"({{"ok":true,"action":"select_model","name":"{}"}})",
            escapeJson(models[index].name));
    }

    std::string sendMessage() {
        std::string input = state_.chatInput;
        if (input.empty()) {
            return R"({"ok":false,"error":"empty input"})";
        }

        auto msgs = static_cast<std::vector<ChatMessage>>(state_.messages);
        msgs.push_back(ChatMessage{
            .role = ChatMessage::Role::User,
            .content = input,
            .timestamp = std::chrono::system_clock::now()
        });
        state_.messages = std::move(msgs);
        state_.chatInput = std::string("");
        state_.isGenerating = true;
        state_.streamingToken = std::string("");

        std::thread([this]() {
            std::string response = "This is a simulated response from the model. "
                "In production, tokens would stream from llama.cpp.\n\n"
                "```python\ndef hello():\n    print(\"Hello from LLM Studio!\")\n```\n\n"
                "Testing complete.";

            std::string accumulated;
            for (size_t i = 0; i < response.size(); i++) {
                if (!static_cast<bool>(state_.isGenerating)) break;
                accumulated += response[i];
                state_.streamingToken = accumulated;
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }

            auto msgs = static_cast<std::vector<ChatMessage>>(state_.messages);
            msgs.push_back(ChatMessage{
                .role = ChatMessage::Role::Assistant,
                .content = accumulated,
                .timestamp = std::chrono::system_clock::now()
            });
            state_.messages = std::move(msgs);
            state_.isGenerating = false;
            state_.streamingToken = std::string("");
        }).detach();

        return R"({"ok":true,"action":"send_message"})";
    }

    std::string searchHub(const std::string& query) {
        state_.hubSearchQuery = query;
        state_.searchResults = std::vector<ModelCard>{
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
        return std::format(R"({{"ok":true,"action":"search_hub","query":"{}","resultCount":5}})",
            escapeJson(query));
    }

    static std::string escapeJson(const std::string& s) {
        std::string out;
        out.reserve(s.size() + 16);
        for (char c : s) {
            switch (c) {
                case '"':  out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default:   out += c;
            }
        }
        return out;
    }

    AppState& state_;
};

} // namespace llm_studio
