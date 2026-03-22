#include <Flux.hpp>
#include <format>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    auto selectedRadio = Property<std::string>::shared(std::string("opt1"));
    auto selectedSelectIdx = Property<int>::shared(0);
    auto selectedSelectLabel = Property<std::string>::shared(std::string("Apple"));

    auto& window = app.createWindow({
        .size = {600, 500},
        .title = "Test: RadioButton & SelectInput"
    });

    window.setRootView(
        VStack {
            .padding = 20,
            .spacing = 16,
            .children = {
                Text {
                    .value = "radio-select-test-root",
                    .fontSize = 18,
                    .fontWeight = FontWeight::bold,
                    .color = Colors::black
                },

                // RadioButton section
                VStack {
                    .spacing = 8,
                    .alignItems = AlignItems::start,
                    .children = {
                        Text { .value = "section-radio", .fontSize = 14, .fontWeight = FontWeight::bold, .color = Colors::black },
                        RadioButton {
                            .selected = [&]() { return selectedRadio.get() == "opt1"; },
                            .value = "opt1",
                            .label = "Option One",
                            .focusKey = "radio-1",
                            .onChange = [&]() { selectedRadio = "opt1"; }
                        },
                        RadioButton {
                            .selected = [&]() { return selectedRadio.get() == "opt2"; },
                            .value = "opt2",
                            .label = "Option Two",
                            .focusKey = "radio-2",
                            .onChange = [&]() { selectedRadio = "opt2"; }
                        },
                        RadioButton {
                            .selected = [&]() { return selectedRadio.get() == "opt3"; },
                            .value = "opt3",
                            .label = "Option Three",
                            .focusKey = "radio-3",
                            .onChange = [&]() { selectedRadio = "opt3"; }
                        }
                    }
                },

                Text {
                    .value = [&]() { return std::format("radio-selected:{}", static_cast<std::string>(selectedRadio)); },
                    .fontSize = 12,
                    .color = Colors::black
                },

                Divider {},

                // SelectInput section
                VStack {
                    .spacing = 8,
                    .alignItems = AlignItems::start,
                    .children = {
                        Text { .value = "section-select", .fontSize = 14, .fontWeight = FontWeight::bold, .color = Colors::black },
                        SelectInput {
                            .selectedIndex = selectedSelectIdx,
                            .options = std::vector<std::string>{"Apple", "Banana", "Cherry", "Date"},
                            .focusKey = "select-1",
                            .onSelect = [&](int idx, const std::string& label) {
                                selectedSelectIdx = idx;
                                selectedSelectLabel = label;
                            }
                        }
                    }
                },

                Text {
                    .value = [&]() { return std::format("select-index:{}", static_cast<int>(selectedSelectIdx)); },
                    .fontSize = 12,
                    .color = Colors::black
                },
                Text {
                    .value = [&]() { return std::format("select-label:{}", static_cast<std::string>(selectedSelectLabel)); },
                    .fontSize = 12,
                    .color = Colors::black
                }
            }
        }
    );

    return app.exec();
}
