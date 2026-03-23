#include <Flux.hpp>

using namespace flux;

const auto imageSvg = R"(<svg width="120px" height="120px" viewBox="0 0 120 120" fill="none" xmlns="http://www.w3.org/2000/svg"><g id="SVGRepo_bgCarrier" stroke-width="0"></g><g id="SVGRepo_tracerCarrier" stroke-linecap="round" stroke-linejoin="round"></g><g id="SVGRepo_iconCarrier"> <rect width="120" height="120" fill="#EFF1F3"></rect> <path fill-rule="evenodd" clip-rule="evenodd" d="M33.2503 38.4816C33.2603 37.0472 34.4199 35.8864 35.8543 35.875H83.1463C84.5848 35.875 85.7503 37.0431 85.7503 38.4816V80.5184C85.7403 81.9528 84.5807 83.1136 83.1463 83.125H35.8543C34.4158 83.1236 33.2503 81.957 33.2503 80.5184V38.4816ZM80.5006 41.1251H38.5006V77.8751L62.8921 53.4783C63.9172 52.4536 65.5788 52.4536 66.6039 53.4783L80.5006 67.4013V41.1251ZM43.75 51.6249C43.75 54.5244 46.1005 56.8749 49 56.8749C51.8995 56.8749 54.25 54.5244 54.25 51.6249C54.25 48.7254 51.8995 46.3749 49 46.3749C46.1005 46.3749 43.75 48.7254 43.75 51.6249Z" fill="#000000"></path> </g></svg>)";
const auto arrowSvg = R"(<svg width="15px" height="15px" viewBox="0 0 15 15" fill="none" xmlns="http://www.w3.org/2000/svg"><g id="SVGRepo_bgCarrier" stroke-width="0"></g><g id="SVGRepo_tracerCarrier" stroke-linecap="round" stroke-linejoin="round"></g><g id="SVGRepo_iconCarrier"> <path fill-rule="evenodd" clip-rule="evenodd" d="M7.49995 15C3.35782 15 -4.55953e-05 11.6421 -4.57764e-05 7.5C-4.59574e-05 3.35786 3.35782 1.81059e-07 7.49995 0C11.6421 -1.81059e-07 15 3.35786 15 7.5C15 11.6421 11.6421 15 7.49995 15ZM4.79292 6.50005L7.50003 3.79294L10.2071 6.50005L9.50003 7.20715L7.99995 5.70708V11H6.99995V5.70723L5.50003 7.20715L4.79292 6.50005Z" fill="#000000"></path> </g></svg>)";
// NanoSVG ignores <style>/class; the exporter’s transparent full-size rect inherited black and hid the glyph.
const auto contextSVG = R"(<svg fill="#000000" width="32px" height="32px" viewBox="0 0 32 32" xmlns="http://www.w3.org/2000/svg"><g id="SVGRepo_iconCarrier"><path d="M16,2A14,14,0,1,0,30,16,14.0158,14.0158,0,0,0,16,2Zm0,26A12,12,0,0,1,16,4V16l8.4812,8.4814A11.9625,11.9625,0,0,1,16,28Z"></path></g></svg>)";

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
                        SVG {
                            .content = contextSVG,
                            .size = Size{24.0f, 24.0f}
                        },
                        SVG {
                            .content = imageSvg,
                            .size = Size{24.0f, 24.0f}
                        },
                        SVG {
                            .content = arrowSvg,
                            .size = Size{24.0f, 24.0f}
                        }
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
