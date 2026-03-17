#pragma once

#include <Flux/Core/Property.hpp>
#include <string>
#include <vector>
#include <chrono>
#include <optional>
#include <filesystem>
#include <cstdint>

namespace llm_studio {

enum class AppView { CHAT, IMAGE, HUB, SETTINGS };
enum class ModelLoadState { UNLOADED, LOADING, READY, ERROR };

struct ModelInfo {
    std::string id;
    std::string name;
    std::string quantization;
    std::string type;          // "text" | "image"
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

struct ImageGenParams {
    std::string prompt;
    std::string negativePrompt;
    int steps = 20;
    float cfgScale = 7.5f;
    int width = 512;
    int height = 512;
    int seed = -1;
};

struct ChatParams {
    float temperature = 0.7f;
    float topP = 0.9f;
    int maxTokens = 2048;
    std::string systemPrompt;
};

struct GeneratedImage {
    std::filesystem::path path;
    std::string prompt;
    std::chrono::system_clock::time_point timestamp;
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
    float temperature = 0.7f;
    std::string theme = "dark";
    float fontSize = 14.0f;
    bool sidebarDefault = true;
    std::filesystem::path modelDirectory = "~/.local/share/llm-studio/models";
    std::filesystem::path imageDirectory = "~/.local/share/llm-studio/images";
    std::string hfToken;
};

struct AppState {
    flux::Property<std::vector<ModelInfo>> installedModels;
    flux::Property<std::vector<ModelCard>> searchResults;
    flux::Property<std::optional<ModelInfo>> activeModel = std::optional<ModelInfo>(std::nullopt);
    flux::Property<ModelLoadState> loadState = ModelLoadState::UNLOADED;

    flux::Property<AppView> currentView = AppView::CHAT;
    flux::Property<bool> sidebarExpanded = true;

    flux::Property<std::vector<ChatMessage>> messages;
    flux::Property<bool> isGenerating = false;
    flux::Property<std::string> streamingToken = std::string("");
    flux::Property<std::string> chatInput = std::string("");

    flux::Property<ImageGenParams> imageParams = ImageGenParams{};
    flux::Property<std::vector<GeneratedImage>> imageHistory;
    flux::Property<bool> isGeneratingImage = false;
    flux::Property<float> imageGenProgress = 0.0f;

    flux::Property<std::optional<DownloadJob>> activeDownload = std::optional<DownloadJob>(std::nullopt);
    flux::Property<std::string> hubSearchQuery = std::string("");

    flux::Property<AppSettings> settings = AppSettings{};

    flux::Property<bool> showClearDialog = false;
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
