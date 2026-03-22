#include <Flux.hpp>
#include <format>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    auto dialogVisible = Property<bool>::shared(false);
    auto dialogResult = Property<std::string>::shared(std::string("none"));
    auto dropdownOpen = Property<bool>::shared(false);
    auto dropdownResult = Property<std::string>::shared(std::string("none"));

    auto& window = app.createWindow({
        .size = {700, 500},
        .title = "Test: Dialog & Dropdown"
    });

    window.setRootView(
        VStack {
            .padding = 20,
            .spacing = 16,
            .children = {
                Text {
                    .value = "dialog-test-root",
                    .fontSize = 18,
                    .fontWeight = FontWeight::bold,
                    .color = Colors::black
                },

                // Dialog controls
                HStack {
                    .spacing = 10,
                    .children = {
                        Button {
                            .text = "Open Dialog",
                            .backgroundColor = Colors::blue,
                            .padding = EdgeInsets(8, 16, 8, 16),
                            .cornerRadius = 4,
                            .focusKey = "btn-open-dialog",
                            .onClick = [&]() {
                                dialogVisible = true;
                                dialogResult = "none";
                            }
                        },
                        Button {
                            .text = "Reset",
                            .backgroundColor = Colors::darkGray,
                            .padding = EdgeInsets(8, 16, 8, 16),
                            .cornerRadius = 4,
                            .focusKey = "btn-reset",
                            .onClick = [&]() {
                                dialogResult = "none";
                                dropdownResult = "none";
                            }
                        }
                    }
                },

                VStack {
                    .spacing = 4,
                    .children = {
                        Text {
                            .value = [&]() { return std::format("dialog-visible:{}", static_cast<bool>(dialogVisible) ? "true" : "false"); },
                            .fontSize = 12,
                            .color = Colors::black
                        },
                        Text {
                            .value = [&]() { return std::format("dialog-result:{}", static_cast<std::string>(dialogResult)); },
                            .fontSize = 12,
                            .color = Colors::black
                        }
                    }
                },

                Divider {},

                // Dropdown section
                VStack {
                    .spacing = 8,
                    .alignItems = AlignItems::start,
                    .children = {
                        Text { .value = "section-dropdown", .fontSize = 14, .fontWeight = FontWeight::bold, .color = Colors::black },
                        DropdownMenu {
                            .label = "Actions",
                            .focusKey = "dropdown-1",
                            .items = std::vector<DropdownMenuItem>{
                                {.label = "Cut", .onClick = [&]() { dropdownResult = "cut"; }},
                                {.label = "Copy", .onClick = [&]() { dropdownResult = "copy"; }},
                                {.label = "Paste", .onClick = [&]() { dropdownResult = "paste"; }},
                                {.label = "Disabled", .enabled = false}
                            }
                        },
                        Text {
                            .value = [&]() { return std::format("dropdown-result:{}", static_cast<std::string>(dropdownResult)); },
                            .fontSize = 12,
                            .color = Colors::black
                        }
                    }
                },

                // Dialog overlay (rendered last so it appears on top)
                Dialog {
                    .isVisible = dialogVisible,
                    .title = "Confirm Action",
                    .message = "Are you sure you want to proceed?",
                    .focusKey = "dialog-1",
                    .buttons = {
                        {.label = "Cancel", .onClick = [&]() {
                            dialogVisible = false;
                            dialogResult = "cancelled";
                        }},
                        {.label = "Confirm", .isPrimary = true, .onClick = [&]() {
                            dialogVisible = false;
                            dialogResult = "confirmed";
                        }}
                    }
                }
            }
        }
    );

    return app.exec();
}
