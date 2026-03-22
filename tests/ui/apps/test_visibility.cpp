#include <Flux.hpp>
#include <format>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    auto showBox = Property<bool>::shared(true);
    auto boxOpacity = Property<float>::shared(1.0f);
    auto toggleCount = Property<int>::shared(0);

    auto& window = app.createWindow({
        .size = {600, 400},
        .title = "Test: Visibility & Opacity"
    });

    window.setRootView(
        VStack {
            .padding = 20,
            .spacing = 16,
            .children = {
                Text {
                    .value = "visibility-test-root",
                    .fontSize = 18,
                    .fontWeight = FontWeight::bold,
                    .color = Colors::black
                },

                HStack {
                    .spacing = 10,
                    .children = {
                        Button {
                            .text = "Toggle Visibility",
                            .backgroundColor = Colors::blue,
                            .padding = EdgeInsets(8, 16, 8, 16),
                            .cornerRadius = 4,
                            .focusKey = "btn-toggle-vis",
                            .onClick = [&]() {
                                showBox = !static_cast<bool>(showBox);
                                toggleCount = static_cast<int>(toggleCount) + 1;
                            }
                        },
                        Button {
                            .text = "Set Opacity 0.5",
                            .backgroundColor = Colors::green,
                            .padding = EdgeInsets(8, 16, 8, 16),
                            .cornerRadius = 4,
                            .focusKey = "btn-opacity-half",
                            .onClick = [&]() {
                                boxOpacity = 0.5f;
                            }
                        },
                        Button {
                            .text = "Set Opacity 1.0",
                            .backgroundColor = Colors::red,
                            .padding = EdgeInsets(8, 16, 8, 16),
                            .cornerRadius = 4,
                            .focusKey = "btn-opacity-full",
                            .onClick = [&]() {
                                boxOpacity = 1.0f;
                            }
                        }
                    }
                },

                // The togglable box
                VStack {
                    .visible = showBox,
                    .backgroundColor = Colors::blue,
                    .padding = 20,
                    .cornerRadius = 8,
                    .opacity = boxOpacity,
                    .children = {
                        Text {
                            .value = "visible-box-content",
                            .fontSize = 14,
                            .color = Colors::white
                        }
                    }
                },

                // Below-box marker to detect layout shift
                Text {
                    .value = "below-box-marker",
                    .fontSize = 14,
                    .color = Colors::black,
                    .backgroundColor = Color::hex(0xEEEEEE),
                    .padding = 8
                },

                VStack {
                    .spacing = 4,
                    .children = {
                        Text {
                            .value = [&]() { return std::format("box-visible:{}", static_cast<bool>(showBox) ? "true" : "false"); },
                            .fontSize = 12,
                            .color = Colors::black
                        },
                        Text {
                            .value = [&]() { return std::format("box-opacity:{:.1f}", static_cast<float>(boxOpacity)); },
                            .fontSize = 12,
                            .color = Colors::black
                        },
                        Text {
                            .value = [&]() { return std::format("toggle-count:{}", static_cast<int>(toggleCount)); },
                            .fontSize = 12,
                            .color = Colors::black
                        }
                    }
                }
            }
        }
    );

    return app.exec();
}
