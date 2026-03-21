#include <Flux.hpp>
#include <format>

using namespace flux;

int main(int argc, char* argv[]) {
    Runtime runtime(argc, argv);

    auto cb1 = Property<bool>::shared(false);
    auto cb2 = Property<bool>::shared(true);
    auto cb3 = Property<bool>::shared(false);
    auto tg1 = Property<bool>::shared(false);
    auto tg2 = Property<bool>::shared(true);

    auto& window = runtime.createWindow({
        .size = {600, 500},
        .title = "Test: Checkbox & Toggle"
    });

    window.setRootView(
        VStack {
            .padding = 20,
            .spacing = 16,
            .children = {
                Text {
                    .value = "checkbox-toggle-test-root",
                    .fontSize = 18,
                    .fontWeight = FontWeight::bold,
                    .color = Colors::black
                },

                // Checkboxes section
                VStack {
                    .spacing = 8,
                    .alignItems = AlignItems::start,
                    .children = {
                        Text { .value = "section-checkboxes", .fontSize = 14, .fontWeight = FontWeight::bold, .color = Colors::black },
                        Checkbox {
                            .checked = cb1,
                            .label = "Checkbox One",
                            .focusKey = "cb-1"
                        },
                        Checkbox {
                            .checked = cb2,
                            .label = "Checkbox Two",
                            .focusKey = "cb-2"
                        },
                        Checkbox {
                            .checked = cb3,
                            .label = "Checkbox Three (leading)",
                            .labelPosition = LabelPosition::leading,
                            .focusKey = "cb-3"
                        }
                    }
                },

                // Checkbox state labels
                VStack {
                    .spacing = 4,
                    .children = {
                        Text {
                            .value = [&]() { return std::format("cb1-state:{}", static_cast<bool>(cb1) ? "checked" : "unchecked"); },
                            .fontSize = 12,
                            .color = Colors::black
                        },
                        Text {
                            .value = [&]() { return std::format("cb2-state:{}", static_cast<bool>(cb2) ? "checked" : "unchecked"); },
                            .fontSize = 12,
                            .color = Colors::black
                        },
                        Text {
                            .value = [&]() { return std::format("cb3-state:{}", static_cast<bool>(cb3) ? "checked" : "unchecked"); },
                            .fontSize = 12,
                            .color = Colors::black
                        }
                    }
                },

                Divider {},

                // Toggles section
                VStack {
                    .spacing = 8,
                    .alignItems = AlignItems::start,
                    .children = {
                        Text { .value = "section-toggles", .fontSize = 14, .fontWeight = FontWeight::bold, .color = Colors::black },
                        Toggle {
                            .isOn = tg1,
                            .label = "Toggle One",
                            .focusKey = "tg-1"
                        },
                        Toggle {
                            .isOn = tg2,
                            .label = "Toggle Two",
                            .focusKey = "tg-2"
                        }
                    }
                },

                // Toggle state labels
                VStack {
                    .spacing = 4,
                    .children = {
                        Text {
                            .value = [&]() { return std::format("tg1-state:{}", static_cast<bool>(tg1) ? "on" : "off"); },
                            .fontSize = 12,
                            .color = Colors::black
                        },
                        Text {
                            .value = [&]() { return std::format("tg2-state:{}", static_cast<bool>(tg2) ? "on" : "off"); },
                            .fontSize = 12,
                            .color = Colors::black
                        }
                    }
                }
            }
        }
    );

    return runtime.run();
}
