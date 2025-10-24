#include <Flux.hpp>
#include <format>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    // State for various event tracking
    Property<std::string> mouseEventLog = "No mouse events yet";
    Property<std::string> keyEventLog = "No key events yet";
    Property<bool> isHovered = false;
    Property<int> clickCount = 0;
    Property<std::string> textInput = "";
    Property<std::string> mousePosition = "0, 0";

    Window window({
        .size = {900, 700},
        .title = "Flux Event System Demo"
    });

    window.setRootView(
        VStack {
            .padding = 20,
            .spacing = 20,
            .backgroundColor = Color::hex(0xf5f5f5),
            .children = {
                // Title
                Text {
                    .value = "Event System Demo",
                    .fontSize = 32,
                    .fontWeight = FontWeight::bold,
                    .color = Colors::black,
                    .horizontalAlignment = HorizontalAlignment::center
                },

                // Mouse Events Section
                VStack {
                    .padding = 16,
                    .spacing = 12,
                    .backgroundColor = Colors::white,
                    .borderColor = Colors::lightGray,
                    .borderWidth = 1,
                    .cornerRadius = 8,
                    .children = {
                        Text {
                            .value = "Mouse Events",
                            .fontSize = 20,
                            .fontWeight = FontWeight::bold,
                            .color = Colors::black
                        },
                        
                        HStack {
                            .spacing = 12,
                            .children = {
                                // Interactive box with hover
                                VStack {
                                    .expansionBias = 1,
                                    .padding = 20,
                                    .backgroundColor = [&isHovered]() {
                                        return isHovered ? Color::hex(0xe3f2fd) : Color::hex(0xffffff);
                                    },
                                    .borderColor = [&isHovered]() {
                                        return isHovered ? Colors::blue : Colors::lightGray;
                                    },
                                    .borderWidth = 2,
                                    .cornerRadius = 8,
                                    .onMouseEnter = [&isHovered, &mouseEventLog]() {
                                        isHovered = true;
                                        mouseEventLog = "Mouse entered!";
                                    },
                                    .onMouseLeave = [&isHovered, &mouseEventLog]() {
                                        isHovered = false;
                                        mouseEventLog = "Mouse left!";
                                    },
                                    .onMouseMove = [&mousePosition](float x, float y) {
                                        mousePosition = std::format("{:.0f}, {:.0f}", x, y);
                                    },
                                    .onClick = [&mouseEventLog, &clickCount]() {
                                        clickCount = static_cast<int>(clickCount) + 1;
                                        mouseEventLog = std::format("Clicked! Count: {}", static_cast<int>(clickCount));
                                    },
                                    .onDoubleClick = [&mouseEventLog]() {
                                        mouseEventLog = "Double-clicked!";
                                    },
                                    .children = {
                                        Text {
                                            .value = "Hover and Click",
                                            .fontSize = 16,
                                            .fontWeight = FontWeight::medium,
                                            .color = Colors::black,
                                            .horizontalAlignment = HorizontalAlignment::center
                                        },
                                        Text {
                                            .value = [&mousePosition]() {
                                                return "Mouse: " + static_cast<std::string>(mousePosition);
                                            },
                                            .fontSize = 12,
                                            .color = Colors::gray,
                                            .horizontalAlignment = HorizontalAlignment::center
                                        }
                                    }
                                },

                                // Right-click detector
                                VStack {
                                    .expansionBias = 1,
                                    .padding = 20,
                                    .backgroundColor = Color::hex(0xfff3e0),
                                    .borderColor = Color::hex(0xff9800),
                                    .borderWidth = 2,
                                    .cornerRadius = 8,
                                    .onMouseDown = [&mouseEventLog](float x, float y, int button) {
                                        std::string buttonName;
                                        if (button == 0) buttonName = "Left";
                                        else if (button == 1) buttonName = "Middle";
                                        else if (button == 2) buttonName = "Right";
                                        else buttonName = std::format("Button {}", button);
                                        
                                        mouseEventLog = std::format("{} click at ({:.0f}, {:.0f})", 
                                                                    buttonName, x, y);
                                    },
                                    .children = {
                                        Text {
                                            .value = "Try Right-Click",
                                            .fontSize = 16,
                                            .fontWeight = FontWeight::medium,
                                            .color = Colors::black,
                                            .horizontalAlignment = HorizontalAlignment::center
                                        }
                                    }
                                }
                            }
                        },

                        // Event log
                        Text {
                            .value = [&mouseEventLog]() {
                                return "Last event: " + static_cast<std::string>(mouseEventLog);
                            },
                            .fontSize = 14,
                            .color = Colors::blue,
                            .padding = 8,
                            .backgroundColor = Color::hex(0xf0f0f0),
                            .cornerRadius = 4
                        }
                    }
                },

                // Keyboard Events Section
                VStack {
                    .padding = 16,
                    .spacing = 12,
                    .backgroundColor = Colors::white,
                    .borderColor = Colors::lightGray,
                    .borderWidth = 1,
                    .cornerRadius = 8,
                    .children = {
                        Text {
                            .value = "Keyboard Events",
                            .fontSize = 20,
                            .fontWeight = FontWeight::bold,
                            .color = Colors::black
                        },

                        Text {
                            .value = "Click the box below and type or press keys",
                            .fontSize = 14,
                            .color = Colors::gray
                        },

                        // Keyboard input box
                        VStack {
                            .padding = 16,
                            .backgroundColor = Color::hex(0xfafafa),
                            .borderColor = Colors::blue,
                            .borderWidth = 2,
                            .cornerRadius = 8,
                            .focusable = true,
                            .cursor = CursorType::Text,
                            .onFocus = [&keyEventLog]() {
                                keyEventLog = "Focused! Start typing...";
                            },
                            .onBlur = [&keyEventLog]() {
                                keyEventLog = "Lost focus";
                            },
                            .onTextInput = [&textInput, &keyEventLog](const std::string& text) {
                                textInput = static_cast<std::string>(textInput) + text;
                                keyEventLog = std::format("Text input: '{}'", text);
                            },
                            .onKeyDown = [&textInput, &keyEventLog](const KeyEvent& e) {
                                if (e.key == Key::Backspace) {
                                    std::string current = textInput;
                                    if (!current.empty()) {
                                        textInput = current.substr(0, current.length() - 1);
                                        keyEventLog = "Backspace pressed";
                                    }
                                    return true;
                                } else if (e.key == Key::Enter) {
                                    keyEventLog = "Enter pressed!";
                                    return true;
                                } else if (e.key == Key::Escape) {
                                    textInput = "";
                                    keyEventLog = "Escape pressed - cleared input";
                                    return true;
                                }
                                
                                // Log special keys
                                if (e.key == Key::Up) keyEventLog = "Up arrow pressed";
                                else if (e.key == Key::Down) keyEventLog = "Down arrow pressed";
                                else if (e.key == Key::Left) keyEventLog = "Left arrow pressed";
                                else if (e.key == Key::Right) keyEventLog = "Right arrow pressed";
                                else if (e.key == Key::Tab) {
                                    keyEventLog = "Tab pressed";
                                    return true;
                                }
                                
                                return false;
                            },
                            .children = {
                                Text {
                                    .value = [&textInput]() {
                                        std::string text = textInput;
                                        return text.empty() ? "(Type here...)" : text;
                                    },
                                    .fontSize = 16,
                                    .color = [&textInput]() {
                                        return static_cast<std::string>(textInput).empty() ? 
                                               Colors::gray : Colors::black;
                                    },
                                    .horizontalAlignment = HorizontalAlignment::leading
                                }
                            }
                        },

                        // Keyboard event log
                        Text {
                            .value = [&keyEventLog]() {
                                return "Last event: " + static_cast<std::string>(keyEventLog);
                            },
                            .fontSize = 14,
                            .color = Colors::green,
                            .padding = 8,
                            .backgroundColor = Color::hex(0xf0f0f0),
                            .cornerRadius = 4
                        }
                    }
                },

                // Multi-Event Button Demo
                VStack {
                    .padding = 16,
                    .spacing = 12,
                    .backgroundColor = Colors::white,
                    .borderColor = Colors::lightGray,
                    .borderWidth = 1,
                    .cornerRadius = 8,
                    .children = {
                        Text {
                            .value = "Combined Events",
                            .fontSize = 20,
                            .fontWeight = FontWeight::bold,
                            .color = Colors::black
                        },

                        HStack {
                            .spacing = 12,
                            .children = {
                                Button {
                                    .text = "Reset Mouse",
                                    .backgroundColor = Colors::blue,
                                    .cornerRadius = 6,
                                    .padding = 12,
                                    .onClick = [&mouseEventLog, &clickCount, &isHovered, &mousePosition]() {
                                        mouseEventLog = "No mouse events yet";
                                        clickCount = 0;
                                        isHovered = false;
                                        mousePosition = "0, 0";
                                    }
                                },

                                Button {
                                    .text = "Reset Keyboard",
                                    .backgroundColor = Colors::green,
                                    .cornerRadius = 6,
                                    .padding = 12,
                                    .onClick = [&keyEventLog, &textInput]() {
                                        keyEventLog = "No key events yet";
                                        textInput = "";
                                    }
                                },

                                Button {
                                    .text = "Reset All",
                                    .backgroundColor = Colors::red,
                                    .cornerRadius = 6,
                                    .padding = 12,
                                    .onClick = [&]() {
                                        mouseEventLog = "No mouse events yet";
                                        keyEventLog = "No key events yet";
                                        clickCount = 0;
                                        isHovered = false;
                                        textInput = "";
                                        mousePosition = "0, 0";
                                    }
                                }
                            }
                        }
                    }
                },

                // Instructions
                Text {
                    .value = "💡 Tip: This demo showcases the new event system with onClick, onMouseEnter/Leave, onMouseDown, onKeyDown, onTextInput, onFocus/Blur and more!",
                    .fontSize = 12,
                    .color = Colors::gray,
                    .horizontalAlignment = HorizontalAlignment::center,
                    .padding = 12
                }
            }
        }
    );

    return app.exec();
}

