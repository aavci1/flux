#include <Flux.hpp>
using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    // Create window
    Window window({
        .size = {800, 600},
        .title = "Focus & Keyboard Demo"
    });

    // Simple keyboard input demonstration
    window.setRootView(
        VStack {
            .padding = 40,
            .spacing = 20,
            .children = {
                // Title
                Text {
                    .value = "Focus & Keyboard Input Demo",
                    .fontSize = 32,
                    .fontWeight = FontWeight::bold,
                    .color = Colors::black,
                    .horizontalAlignment = HorizontalAlignment::center
                },
                
                // Instructions
                Text {
                    .value = "Press keys to see keyboard events in the console!",
                    .fontSize = 18,
                    .color = Colors::gray,
                    .horizontalAlignment = HorizontalAlignment::center
                },
                
                // Try these shortcuts
                VStack {
                    .padding = 20,
                    .spacing = 10,
                    .backgroundColor = Color::hex(0xF5F5F5),
                    .cornerRadius = 10,
                    .children = {
                        Text {
                            .value = "Available Keyboard Shortcuts:",
                            .fontSize = 20,
                            .fontWeight = FontWeight::bold,
                            .color = Colors::black,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },
                        Text {
                            .value = "• Tab - Focus navigation (buttons will highlight when focused)",
                            .fontSize = 16,
                            .color = Colors::darkGray,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },
                        Text {
                            .value = "• Shift+Tab - Reverse focus navigation",
                            .fontSize = 16,
                            .color = Colors::darkGray,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },
                        Text {
                            .value = "• Ctrl+Q - Quit application",
                            .fontSize = 16,
                            .color = Colors::darkGray,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },
                        Text {
                            .value = "• Ctrl+C - Copy (placeholder)",
                            .fontSize = 16,
                            .color = Colors::darkGray,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },
                        Text {
                            .value = "• Ctrl+V - Paste (placeholder)",
                            .fontSize = 16,
                            .color = Colors::darkGray,
                            .horizontalAlignment = HorizontalAlignment::leading
                        }
                    }
                },
                
                // Status
                Text {
                    .value = "Watch the console for keyboard event logs!",
                    .fontSize = 16,
                    .color = Colors::blue,
                    .horizontalAlignment = HorizontalAlignment::center
                },
                
                // Buttons for testing (focusable by default)
                HStack {
                    .spacing = 20,
                    .justifyContent = JustifyContent::center,
                    .children = {
                        Button {
                            .text = "Button 1",
                            .padding = EdgeInsets{12, 24},
                            .backgroundColor = Colors::blue,
                            .cornerRadius = 8,
                            .focusKey = "btn1",  // Explicit focus key for stable focus
                            .onClick = []() {
                                std::cout << "[DEMO] Button 1 clicked or activated!\n";
                            }
                        },
                        Button {
                            .text = "Button 2",
                            .padding = EdgeInsets{12, 24},
                            .backgroundColor = Colors::green,
                            .cornerRadius = 8,
                            .focusKey = "btn2",  // Explicit focus key for stable focus
                            .onClick = []() {
                                std::cout << "[DEMO] Button 2 clicked or activated!\n";
                            }
                        },
                        Button {
                            .text = "Button 3",
                            .padding = EdgeInsets{12, 24},
                            .backgroundColor = Colors::red,
                            .cornerRadius = 8,
                            .focusKey = "btn3",  // Explicit focus key for stable focus
                            .onClick = []() {
                                std::cout << "[DEMO] Button 3 clicked or activated!\n";
                            }
                        }
                    }
                }
            }
        }
    );

    std::cout << "\n=== Focus & Keyboard Demo Started ===\n";
    std::cout << "Try pressing keys and watch for console output!\n";
    std::cout << "- All keyboard events are logged\n";
    std::cout << "- Modifier keys (Ctrl, Shift, Alt) are tracked\n";
    std::cout << "- Global shortcuts are handled\n";
    std::cout << "- Tab navigation is implemented\n\n";

    return app.exec();
}

