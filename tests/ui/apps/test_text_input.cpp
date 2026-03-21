#include <Flux.hpp>
#include <format>

using namespace flux;

int main(int argc, char* argv[]) {
    Runtime runtime(argc, argv);

    auto inputValue = Property<std::string>::shared(std::string(""));
    auto passwordValue = Property<std::string>::shared(std::string(""));
    auto limitedValue = Property<std::string>::shared(std::string(""));
    auto areaValue = Property<std::string>::shared(std::string(""));
    auto returnStatus = Property<std::string>::shared(std::string("no-return"));

    auto& window = runtime.createWindow({
        .size = {700, 600},
        .title = "Test: Text Input"
    });

    window.setRootView(
        VStack {
            .padding = 20,
            .spacing = 14,
            .children = {
                Text {
                    .value = "text-input-test-root",
                    .fontSize = 18,
                    .fontWeight = FontWeight::bold,
                    .color = Colors::black
                },

                // Basic TextInput
                VStack {
                    .spacing = 4,
                    .alignItems = AlignItems::start,
                    .children = {
                        Text { .value = "section-basic-input", .fontSize = 12, .fontWeight = FontWeight::bold, .color = Colors::black },
                        TextInput {
                            .value = inputValue,
                            .placeholder = "Type here...",
                            .focusKey = "input-basic",
                            .onValueChange = [&](const std::string& v) { inputValue = v; },
                            .onReturn = [&]() { returnStatus = "return-pressed"; }
                        },
                        Text {
                            .value = [&]() { return std::format("input-value:{}", static_cast<std::string>(inputValue)); },
                            .fontSize = 11,
                            .color = Colors::black
                        },
                        Text {
                            .value = [&]() { return std::format("input-length:{}", static_cast<std::string>(inputValue).size()); },
                            .fontSize = 11,
                            .color = Colors::black
                        },
                        Text {
                            .value = [&]() { return std::format("return-status:{}", static_cast<std::string>(returnStatus)); },
                            .fontSize = 11,
                            .color = Colors::black
                        }
                    }
                },

                Divider {},

                // Password TextInput
                VStack {
                    .spacing = 4,
                    .alignItems = AlignItems::start,
                    .children = {
                        Text { .value = "section-password", .fontSize = 12, .fontWeight = FontWeight::bold, .color = Colors::black },
                        TextInput {
                            .value = passwordValue,
                            .placeholder = "Password...",
                            .password = true,
                            .focusKey = "input-password",
                            .onValueChange = [&](const std::string& v) { passwordValue = v; }
                        },
                        Text {
                            .value = [&]() { return std::format("password-length:{}", static_cast<std::string>(passwordValue).size()); },
                            .fontSize = 11,
                            .color = Colors::black
                        }
                    }
                },

                Divider {},

                // Max-length TextInput
                VStack {
                    .spacing = 4,
                    .alignItems = AlignItems::start,
                    .children = {
                        Text { .value = "section-maxlength", .fontSize = 12, .fontWeight = FontWeight::bold, .color = Colors::black },
                        TextInput {
                            .value = limitedValue,
                            .placeholder = "Max 5 chars",
                            .maxLength = 5,
                            .focusKey = "input-limited",
                            .onValueChange = [&](const std::string& v) { limitedValue = v; }
                        },
                        Text {
                            .value = [&]() { return std::format("limited-value:{}", static_cast<std::string>(limitedValue)); },
                            .fontSize = 11,
                            .color = Colors::black
                        }
                    }
                },

                Divider {},

                // TextArea
                VStack {
                    .spacing = 4,
                    .alignItems = AlignItems::start,
                    .children = {
                        Text { .value = "section-textarea", .fontSize = 12, .fontWeight = FontWeight::bold, .color = Colors::black },
                        TextArea {
                            .value = areaValue,
                            .placeholder = "Multi-line...",
                            .focusKey = "input-area",
                            .onValueChange = [&](const std::string& v) { areaValue = v; }
                        },
                        Text {
                            .value = [&]() { return std::format("area-value:{}", static_cast<std::string>(areaValue)); },
                            .fontSize = 11,
                            .color = Colors::black
                        }
                    }
                },

                Button {
                    .text = "Reset All",
                    .backgroundColor = Colors::darkGray,
                    .padding = EdgeInsets(8, 16, 8, 16),
                    .cornerRadius = 4,
                    .focusKey = "btn-reset",
                    .onClick = [&]() {
                        inputValue = std::string("");
                        passwordValue = std::string("");
                        limitedValue = std::string("");
                        areaValue = std::string("");
                        returnStatus = "no-return";
                    }
                }
            }
        }
    );

    return runtime.run();
}
