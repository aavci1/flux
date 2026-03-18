#include <Flux.hpp>
#include <format>

using namespace flux;

int main(int argc, char* argv[]) {
    Runtime runtime(argc, argv);

    Property<int> clickCountA = 0;
    Property<int> clickCountB = 0;
    Property<int> clickCountC = 0;
    Property<std::string> lastActivated = "none";

    auto& window = runtime.createWindow({
        .size = {600, 400},
        .title = "Test: Button Interactivity"
    });

    window.setRootView(
        VStack {
            .padding = 20,
            .spacing = 16,
            .children = {
                Text {
                    .value = "button-test-root",
                    .fontSize = 18,
                    .fontWeight = FontWeight::bold,
                    .color = Colors::black
                },

                HStack {
                    .spacing = 12,
                    .children = {
                        Button {
                            .text = "Button A",
                            .backgroundColor = Colors::blue,
                            .padding = EdgeInsets(10, 20, 10, 20),
                            .cornerRadius = 6,
                            .focusKey = "btn-a",
                            .onClick = [&]() {
                                clickCountA = static_cast<int>(clickCountA) + 1;
                                lastActivated = "A";
                            }
                        },
                        Button {
                            .text = "Button B",
                            .backgroundColor = Colors::green,
                            .padding = EdgeInsets(10, 20, 10, 20),
                            .cornerRadius = 6,
                            .focusKey = "btn-b",
                            .onClick = [&]() {
                                clickCountB = static_cast<int>(clickCountB) + 1;
                                lastActivated = "B";
                            }
                        },
                        Button {
                            .text = "Button C",
                            .backgroundColor = Colors::red,
                            .padding = EdgeInsets(10, 20, 10, 20),
                            .cornerRadius = 6,
                            .focusKey = "btn-c",
                            .onClick = [&]() {
                                clickCountC = static_cast<int>(clickCountC) + 1;
                                lastActivated = "C";
                            }
                        }
                    }
                },

                VStack {
                    .spacing = 6,
                    .children = {
                        Text {
                            .value = [&]() { return std::format("count-a:{}", static_cast<int>(clickCountA)); },
                            .fontSize = 14,
                            .color = Colors::black
                        },
                        Text {
                            .value = [&]() { return std::format("count-b:{}", static_cast<int>(clickCountB)); },
                            .fontSize = 14,
                            .color = Colors::black
                        },
                        Text {
                            .value = [&]() { return std::format("count-c:{}", static_cast<int>(clickCountC)); },
                            .fontSize = 14,
                            .color = Colors::black
                        },
                        Text {
                            .value = [&]() { return std::format("last-activated:{}", static_cast<std::string>(lastActivated)); },
                            .fontSize = 14,
                            .color = Colors::black
                        }
                    }
                },

                // Reset button
                Button {
                    .text = "Reset All",
                    .backgroundColor = Colors::darkGray,
                    .padding = EdgeInsets(8, 16, 8, 16),
                    .cornerRadius = 4,
                    .focusKey = "btn-reset",
                    .onClick = [&]() {
                        clickCountA = 0;
                        clickCountB = 0;
                        clickCountC = 0;
                        lastActivated = "none";
                    }
                }
            }
        }
    );

    return runtime.run();
}
