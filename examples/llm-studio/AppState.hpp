#pragma once

#include <Flux/Core/Property.hpp>
#include <Flux/Core/Typography.hpp>
#include <string>
#include <vector>
#include <chrono>
#include <optional>
#include <filesystem>
#include <cstdint>
#include <format>
#include <random>

namespace llm_studio {

enum class AppPage { CHAT, MODELS, SETTINGS };
enum class ModelLoadState { UNLOADED, LOADING, READY, ERROR };

struct ModelInfo {
    std::string id;
    std::string name;
    std::string quantization;
    std::string type;          // "text" | "image" | "multimodal"
    uint64_t sizeBytes = 0;
    std::filesystem::path localPath;
    bool isLoaded = false;
};

struct ChatMessage {
    enum class Role { User, Assistant, System };
    Role role;
    std::string content;
    std::chrono::system_clock::time_point timestamp;
};

struct ChatParams {
    float temperature = 0.7f;
    float topP = 0.9f;
    int maxTokens = 2048;
    std::string systemPrompt;
};

struct ChatSession {
    std::string id;
    std::string title;
    std::vector<ChatMessage> messages;
    std::optional<ModelInfo> model;
    ChatParams params;
    std::chrono::system_clock::time_point createdAt;
};

struct ModelCard {
    std::string repoId;
    std::string name;
    int likes = 0;
    std::string format;
    std::string parameterSize;
    uint64_t sizeBytes = 0;
};

struct ModelVariant {
    std::string filename;
    std::string quantization;
    uint64_t sizeBytes = 0;
};

struct DownloadJob {
    std::string modelId;
    std::string variant;
    float progress = 0.0f;
    float speedBytesPerSec = 0.0f;
    uint64_t totalBytes = 0;
    uint64_t downloadedBytes = 0;
};

struct AppSettings {
    std::string backendPath = "/usr/local/bin/llama-server";
    int contextLength = 4096;
    int gpuLayers = 0;
    int threads = 4;
    std::string theme = "dark";
    float fontSize = flux::Typography::body;
    std::filesystem::path modelDirectory = "~/.local/share/llm-studio/models";
    std::string hfToken;
};

struct AppState {
    flux::Property<std::vector<ModelInfo>> installedModels;
    flux::Property<std::vector<ModelCard>> searchResults;
    flux::Property<std::optional<ModelInfo>> activeModel = std::optional<ModelInfo>(std::nullopt);
    flux::Property<ModelLoadState> loadState = ModelLoadState::UNLOADED;

    flux::Property<AppPage> currentPage = AppPage::CHAT;

    flux::Property<std::vector<ChatSession>> chatSessions;
    flux::Property<std::string> activeChatId = std::string("");

    flux::Property<bool> leftSidebarExpanded = true;

    flux::Property<bool> isGenerating = false;
    flux::Property<std::string> streamingToken = std::string("");
    flux::Property<std::string> chatInput = std::string("");

    flux::Property<std::optional<DownloadJob>> activeDownload = std::optional<DownloadJob>(std::nullopt);
    flux::Property<std::string> hubSearchQuery = std::string("");

    flux::Property<AppSettings> settings = AppSettings{};

    void updateSettings(const std::function<void(AppSettings&)>& updater) {
        auto s = settings.get();
        updater(s);
        settings = std::move(s);
    }

    ChatSession* getActiveSession() {
        std::string activeId = activeChatId;
        if (activeId.empty()) return nullptr;
        auto sessions = chatSessions.get();
        for (auto& s : sessions) {
            if (s.id == activeId) {
                auto allSessions = chatSessions.get();
                for (size_t i = 0; i < allSessions.size(); i++) {
                    if (allSessions[i].id == activeId) {
                        return nullptr;
                    }
                }
            }
        }
        return nullptr;
    }

    void updateActiveSession(const std::function<void(ChatSession&)>& updater) {
        std::string activeId = activeChatId;
        if (activeId.empty()) return;
        auto sessions = chatSessions.get();
        for (size_t i = 0; i < sessions.size(); i++) {
            if (sessions[i].id == activeId) {
                updater(sessions[i]);
                chatSessions = std::move(sessions);
                return;
            }
        }
    }

    std::optional<ChatSession> getActiveSessionCopy() const {
        std::string activeId = activeChatId;
        if (activeId.empty()) return std::nullopt;
        auto sessions = chatSessions.get();
        for (const auto& s : sessions) {
            if (s.id == activeId) return s;
        }
        return std::nullopt;
    }

    std::string createNewChat() {
        static std::mt19937 rng(std::random_device{}());
        static std::uniform_int_distribution<uint64_t> dist;
        std::string id = std::format("chat_{}", dist(rng));

        ChatSession session{
            .id = id,
            .title = "New Chat",
            .messages = {},
            .model = static_cast<std::optional<ModelInfo>>(activeModel),
            .params = ChatParams{},
            .createdAt = std::chrono::system_clock::now()
        };

        auto sessions = chatSessions.get();
        sessions.insert(sessions.begin(), session);
        chatSessions = std::move(sessions);
        activeChatId = id;
        currentPage = AppPage::CHAT;
        return id;
    }
};

inline std::string formatBytes(uint64_t bytes) {
    if (bytes >= 1073741824ULL)
        return std::format("{:.1f} GB", bytes / 1073741824.0);
    if (bytes >= 1048576ULL)
        return std::format("{:.1f} MB", bytes / 1048576.0);
    if (bytes >= 1024ULL)
        return std::format("{:.1f} KB", bytes / 1024.0);
    return std::format("{} B", bytes);
}

inline std::string formatTime(std::chrono::system_clock::time_point tp) {
    auto t = std::chrono::system_clock::to_time_t(tp);
    auto tm = *std::localtime(&t);
    char buf[6];
    std::strftime(buf, sizeof(buf), "%H:%M", &tm);
    return std::string(buf);
}

} // namespace llm_studio
