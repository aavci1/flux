#include <Flux.hpp>
#include "Theme.hpp"
#include "AppState.hpp"
#include "Views/IconRail.hpp"
#include "Views/Sidebar.hpp"
#include "Views/ChatView.hpp"
#include "Views/HubView.hpp"
#include "Views/SettingsView.hpp"
#include "Views/ModelConfigSidebar.hpp"

using namespace flux;
using namespace llm_studio;

struct MainPanel {
    FLUX_VIEW_PROPERTIES;

    AppState* state = nullptr;

    View body() const {
        if (!state) return VStack{};

        AppPage page = state->currentPage;
        switch (page) {
            case AppPage::CHAT:
                return ChatView{.state = state, .expansionBias = 1.0f};
            case AppPage::MODELS:
                return HubView{.state = state, .expansionBias = 1.0f};
        }
        return VStack{};
    }
};

struct AppRoot {
    FLUX_VIEW_PROPERTIES;

    AppState* state = nullptr;

    View body() const {
        if (!state) return VStack{};

        bool showSettings = state->showSettingsDialog;

        if (showSettings) {
            return SettingsDialog{
                .state = state,
                .expansionBias = 1.0f
            };
        }

        return HStack{
            .spacing = 0.0f,
            .alignItems = AlignItems::stretch,
            .backgroundColor = Theme::Background,
            .expansionBias = 1.0f,
            .children = {
                IconRailView{.state = state},
                SidebarView{.state = state},
                MainPanel{.state = state, .expansionBias = 1.0f},
                ModelConfigSidebar{.state = state}
            }
        };
    }
};

int main(int argc, char* argv[]) {
    Runtime runtime(argc, argv);

    AppState state;

    state.installedModels = std::vector<ModelInfo>{
        ModelInfo{
            .id = "meta-llama/Llama-3.1-8B-Instruct",
            .name = "Llama 3.1 8B",
            .quantization = "Q4_K_M",
            .type = "text",
            .sizeBytes = 4650000000ULL,
            .isLoaded = false
        },
        ModelInfo{
            .id = "mistralai/Mistral-7B-Instruct-v0.3",
            .name = "Mistral 7B",
            .quantization = "Q4_K_M",
            .type = "text",
            .sizeBytes = 4100000000ULL,
            .isLoaded = false
        },
        ModelInfo{
            .id = "microsoft/Phi-3.5-mini-instruct",
            .name = "Phi 3.5 Mini",
            .quantization = "Q5_K_M",
            .type = "text",
            .sizeBytes = 2600000000ULL,
            .isLoaded = false
        }
    };

    state.activeModel = state.installedModels.get()[0];
    state.loadState = ModelLoadState::READY;

    auto& window = runtime.createWindow({
        .size = {1280, 800},
        .title = "LLM Studio"
    });

    window.setRootView(
        AppRoot{.state = &state, .expansionBias = 1.0f}
    );

    return runtime.run();
}
