#include <Flux.hpp>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    auto& window = app.createWindow({
        .size = {600, 300},
        .title = "Flux"
    });

    Theme theme = Theme::dark();

    window.setRootView(
        EnvironmentProvider<ThemeKey>{
            .value = Theme::dark(),
            .child = VStack {
                .padding = 16,
                .spacing = 16,
                .children = {
                    Spacer {},
                    VStack {
                        .backgroundColor = theme.inputBackground,
                        .borderColor = theme.inputBorderColor,
                        .borderWidth = theme.inputBorderWidth,
                        .cornerRadius = theme.inputCornerRadius,
                        .padding = 8,
                        .spacing = 8,
                        .children = {
                            TextArea {
                                .borderWidth = 0,
                                .bgColor = theme.inputBackground,
                                .textColor = theme.inputForeground,
                                .value = "Hello, World!"
                            },
                            HStack {
                                .spacing = 8,
                                .children = {
                                    Button {
                                        .text = "Agent",
                                        .backgroundColor = theme.inputBackground,
                                        .textColor = theme.inputForeground
                                    },
                                    Button {
                                        .text = "Auto",
                                        .backgroundColor = theme.inputBackground,
                                        .textColor = theme.inputForeground,
                                    },
                                    Spacer {},
                                    Button {
                                        .text = "C",
                                        .backgroundColor = theme.inputBackground,
                                        .textColor = theme.inputForeground,
                                    },
                                    Button {
                                        .text = "I",
                                        .backgroundColor = theme.inputBackground,
                                        .textColor = theme.inputForeground,
                                    },
                                    Button {
                                        .text = "S",
                                        .backgroundColor = theme.inputBackground,
                                        .textColor = theme.inputForeground,
                                    }
                                }
                            }
                        }
                    },
                    Spacer {},
                }
            }
        }
    );

    return app.exec();
}