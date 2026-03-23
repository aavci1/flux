#include <Flux.hpp>

using namespace flux;

struct ChatBubble {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;
    Property<std::string> message;

    View body() const {
        const Theme& theme = Application::instance().theme();

        std::vector<DropdownMenuItem> modeItems = {
            {.label = "Agent", .subtitle = "Plan, search, build anything"},
            {.label = "Plan", .subtitle = "Create detailed plans for accomplishing tasks"},
            {.label = "Debug", .subtitle = "Systematically diagnose and fix bugs using runtime traces"},
            {.label = "Ask", .subtitle = "Ask questions about the workspace"},
        };

        std::vector<DropdownMenuItem> modelItems = {
            {.label = "Composer 2"},
            {.label = "GPT-5.3 Codex"},
            {.label = "Sonnet 4.6"},
            {.label = "Opus 4.6"},
        };

        return VStack {
            .backgroundColor = theme.inputBackground,
            .borderColor = theme.inputBorderColor,
            .borderWidth = theme.inputBorderWidth,
            .cornerRadius = theme.inputCornerRadius,
            .padding = 8,
            .spacing = 8,
            .children = {
                TextArea {
                    .outlineWidth = 0,
                    .padding = 0,
                    .bgColor = theme.inputBackground,
                    .textColor = theme.inputForeground,
                    .value = message
                },
                HStack {
                    .spacing = 8,
                    .children = {
                        DropdownMenu {
                            .label = std::string("Agent"),
                            .bgColor = theme.inputBackground,
                            .dropdownBgColor = theme.surfaceElevated,
                            .borderColor_ = theme.borderStrong,
                            .textColor_ = theme.foreground,
                            .mutedColor = theme.secondaryForeground,
                            .items = std::move(modeItems)
                        },
                        DropdownMenu {
                            .label = std::string("Model"),
                            .bgColor = theme.inputBackground,
                            .dropdownBgColor = theme.surfaceElevated,
                            .borderColor_ = theme.borderStrong,
                            .textColor_ = theme.foreground,
                            .mutedColor = theme.secondaryForeground,
                            .items = std::move(modelItems)
                        },
                        Spacer {},
                        Text {
                            .value = std::string("\u25D4"),
                            .fontSize = 24.0f,
                            .color = theme.secondaryForeground,
                            .horizontalAlignment = HorizontalAlignment::center,
                        },
                        Text {
                            .value = std::string("\u25A4"),
                            .fontSize = 24.0f,
                            .color = theme.secondaryForeground,
                            .horizontalAlignment = HorizontalAlignment::center,
                        },
                        Text {
                            .value = std::string("\u2191"),
                            .fontSize = 24.0f,
                            .color = theme.secondaryForeground,
                            .horizontalAlignment = HorizontalAlignment::center,
                        },
                    }
                }
            }
        };
    }
};

struct MainView {
    FLUX_VIEW_PROPERTIES;

    View body() const {
        const Theme& theme = Application::instance().theme();
        const bool dark = (theme == Theme::dark());

        return VStack {
            .backgroundColor = theme.background,
            .padding = 16,
            .spacing = 16,
            .children = {
                Toggle {
                    .isOn = dark,
                    .label = std::string("Dark Mode"),
                    .labelColor = theme.foreground,
                    .onChange = []() {
                        const Theme& current = Application::instance().theme();
                        Application::instance().setTheme(current == Theme::light() ? Theme::dark()
                                                                                  : Theme::light());
                    }
                },
                Spacer {},
                ChatBubble {
                    .message = "Hello, World!"
                },
                Spacer {},
            }
        };
    }
};

int main(int argc, char* argv[]) {
    Application app(argc, argv);
    app.setTheme(Theme::dark());

    auto& window = app.createWindow({
        .size = {800, 400},
        .title = "Flux"
    });

    window.setRootView(
        MainView {}
    );

    return app.exec();
}
