#pragma once

#include "../AppState.hpp"
#include <string>
#include <format>

namespace llm_studio {

inline std::string escapeJson(const std::string& s) {
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

inline std::string viewToString(AppView v) {
    switch (v) {
        case AppView::CHAT: return "CHAT";
        case AppView::IMAGE: return "IMAGE";
        case AppView::HUB: return "HUB";
        case AppView::SETTINGS: return "SETTINGS";
    }
    return "UNKNOWN";
}

inline std::string loadStateToString(ModelLoadState s) {
    switch (s) {
        case ModelLoadState::UNLOADED: return "UNLOADED";
        case ModelLoadState::LOADING: return "LOADING";
        case ModelLoadState::READY: return "READY";
        case ModelLoadState::ERROR: return "ERROR";
    }
    return "UNKNOWN";
}

inline std::string serializeState(const AppState& state) {
    auto models = static_cast<std::vector<ModelInfo>>(state.installedModels);
    auto msgs = static_cast<std::vector<ChatMessage>>(state.messages);
    auto active = static_cast<std::optional<ModelInfo>>(state.activeModel);
    auto dl = static_cast<std::optional<DownloadJob>>(state.activeDownload);
    auto searchResults = static_cast<std::vector<ModelCard>>(state.searchResults);
    auto imageHist = static_cast<std::vector<GeneratedImage>>(state.imageHistory);

    std::string modelsJson = "[";
    for (size_t i = 0; i < models.size(); i++) {
        if (i > 0) modelsJson += ",";
        modelsJson += std::format(
            R"({{"id":"{}","name":"{}","quantization":"{}","type":"{}","sizeBytes":{},"isLoaded":{}}})",
            escapeJson(models[i].id), escapeJson(models[i].name),
            escapeJson(models[i].quantization), escapeJson(models[i].type),
            models[i].sizeBytes, models[i].isLoaded ? "true" : "false"
        );
    }
    modelsJson += "]";

    std::string msgsJson = "[";
    for (size_t i = 0; i < msgs.size(); i++) {
        if (i > 0) msgsJson += ",";
        std::string role;
        switch (msgs[i].role) {
            case ChatMessage::Role::User: role = "user"; break;
            case ChatMessage::Role::Assistant: role = "assistant"; break;
            case ChatMessage::Role::System: role = "system"; break;
        }
        msgsJson += std::format(
            R"({{"role":"{}","content":"{}","time":"{}"}})",
            role, escapeJson(msgs[i].content), formatTime(msgs[i].timestamp)
        );
    }
    msgsJson += "]";

    std::string searchJson = "[";
    for (size_t i = 0; i < searchResults.size(); i++) {
        if (i > 0) searchJson += ",";
        searchJson += std::format(
            R"({{"repoId":"{}","name":"{}","likes":{},"format":"{}","parameterSize":"{}","sizeBytes":{}}})",
            escapeJson(searchResults[i].repoId), escapeJson(searchResults[i].name),
            searchResults[i].likes, escapeJson(searchResults[i].format),
            escapeJson(searchResults[i].parameterSize), searchResults[i].sizeBytes
        );
    }
    searchJson += "]";

    std::string dlJson = "null";
    if (dl.has_value()) {
        dlJson = std::format(
            R"({{"modelId":"{}","variant":"{}","progress":{},"speedBytesPerSec":{},"totalBytes":{},"downloadedBytes":{}}})",
            escapeJson(dl->modelId), escapeJson(dl->variant),
            dl->progress, dl->speedBytesPerSec, dl->totalBytes, dl->downloadedBytes
        );
    }

    return std::format(
        R"({{)"
        R"("currentView":"{}",)"
        R"("sidebarExpanded":{},)"
        R"("activeModel":{},)"
        R"("loadState":"{}",)"
        R"("isGenerating":{},)"
        R"("isGeneratingImage":{},)"
        R"("imageGenProgress":{},)"
        R"("chatInput":"{}",)"
        R"("hubSearchQuery":"{}",)"
        R"("streamingToken":"{}",)"
        R"("showClearDialog":{},)"
        R"("messageCount":{},)"
        R"("installedModelCount":{},)"
        R"("searchResultCount":{},)"
        R"("imageHistoryCount":{},)"
        R"("installedModels":{},)"
        R"("messages":{},)"
        R"("searchResults":{},)"
        R"("activeDownload":{})"
        R"(}})",
        viewToString(state.currentView),
        static_cast<bool>(state.sidebarExpanded) ? "true" : "false",
        active.has_value()
            ? std::format(R"("{}")", escapeJson(active->name))
            : std::string("null"),
        loadStateToString(state.loadState),
        static_cast<bool>(state.isGenerating) ? "true" : "false",
        static_cast<bool>(state.isGeneratingImage) ? "true" : "false",
        static_cast<float>(state.imageGenProgress),
        escapeJson(static_cast<std::string>(state.chatInput)),
        escapeJson(static_cast<std::string>(state.hubSearchQuery)),
        escapeJson(static_cast<std::string>(state.streamingToken)),
        static_cast<bool>(state.showClearDialog) ? "true" : "false",
        msgs.size(),
        models.size(),
        searchResults.size(),
        imageHist.size(),
        modelsJson,
        msgsJson,
        searchJson,
        dlJson
    );
}

} // namespace llm_studio
