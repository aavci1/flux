#include <Flux.hpp>
#include <format>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    Property<int> counter = 0;
    Property<std::string> history = "";

    auto& window = app.createWindow({
        .size = {600, 500},
        .title = "Test: Nested State"
    });

    window.setRootView(
        VStack {
            .padding = 20,
            .spacing = 16,
            .children = {
                Text {
                    .value = "nested-state-test-root",
                    .fontSize = 18,
                    .fontWeight = FontWeight::bold,
                    .color = Colors::black
                },

                // Counter display
                VStack {
                    .spacing = 6,
                    .backgroundColor = Color::hex(0xF5F5F5),
                    .padding = 16,
                    .cornerRadius = 8,
                    .children = {
                        Text {
                            .value = [&]() { return std::format("counter:{}", static_cast<int>(counter)); },
                            .fontSize = 24,
                            .fontWeight = FontWeight::bold,
                            .color = Colors::black,
                            .horizontalAlignment = HorizontalAlignment::center
                        },
                        Text {
                            .value = [&]() { return std::format("doubled:{}", static_cast<int>(counter) * 2); },
                            .fontSize = 14,
                            .color = Colors::darkGray,
                            .horizontalAlignment = HorizontalAlignment::center
                        },
                        Text {
                            .value = [&]() { return std::format("tripled:{}", static_cast<int>(counter) * 3); },
                            .fontSize = 14,
                            .color = Colors::darkGray,
                            .horizontalAlignment = HorizontalAlignment::center
                        },
                        Text {
                            .value = [&]() {
                                int c = counter;
                                if (c == 0) return std::string("parity:even");
                                return std::format("parity:{}", (c % 2 == 0) ? "even" : "odd");
                            },
                            .fontSize = 14,
                            .color = Colors::darkGray,
                            .horizontalAlignment = HorizontalAlignment::center
                        }
                    }
                },

                // Buttons
                HStack {
                    .spacing = 10,
                    .justifyContent = JustifyContent::center,
                    .children = {
                        Button {
                            .text = "Decrement",
                            .backgroundColor = Colors::red,
                            .padding = EdgeInsets(10, 20, 10, 20),
                            .cornerRadius = 6,
                            .focusKey = "btn-dec",
                            .onClick = [&]() {
                                counter = static_cast<int>(counter) - 1;
                                history = static_cast<std::string>(history) + "D";
                            }
                        },
                        Button {
                            .text = "Increment",
                            .backgroundColor = Colors::green,
                            .padding = EdgeInsets(10, 20, 10, 20),
                            .cornerRadius = 6,
                            .focusKey = "btn-inc",
                            .onClick = [&]() {
                                counter = static_cast<int>(counter) + 1;
                                history = static_cast<std::string>(history) + "I";
                            }
                        },
                        Button {
                            .text = "Reset",
                            .backgroundColor = Colors::darkGray,
                            .padding = EdgeInsets(10, 20, 10, 20),
                            .cornerRadius = 6,
                            .focusKey = "btn-reset",
                            .onClick = [&]() {
                                counter = 0;
                                history = static_cast<std::string>(history) + "R";
                            }
                        }
                    }
                },

                Divider {},

                // Badge that changes based on counter
                HStack {
                    .spacing = 10,
                    .alignItems = AlignItems::center,
                    .children = {
                        Text {
                            .value = "Status:",
                            .fontSize = 14,
                            .color = Colors::black
                        },
                        Badge {
                            .text = [&]() {
                                int c = counter;
                                if (c < 0) return std::string("negative");
                                if (c == 0) return std::string("zero");
                                if (c < 5) return std::string("low");
                                if (c < 10) return std::string("medium");
                                return std::string("high");
                            },
                            .badgeColor = [&]() {
                                int c = counter;
                                if (c < 0) return Colors::red;
                                if (c == 0) return Colors::gray;
                                if (c < 5) return Colors::blue;
                                if (c < 10) return Color::hex(0xFF9800);
                                return Colors::green;
                            }
                        }
                    }
                },

                // History log
                Text {
                    .value = [&]() { return std::format("history:{}", static_cast<std::string>(history)); },
                    .fontSize = 12,
                    .color = Colors::darkGray
                }
            }
        }
    );

    return app.exec();
}
