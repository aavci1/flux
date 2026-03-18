#include <Flux.hpp>
#include <format>

using namespace flux;

int main(int argc, char* argv[]) {
    Runtime runtime(argc, argv);

    Property<std::string> focusLog = "none";
    Property<int> focusCount = 0;
    Property<int> blurCount = 0;

    auto& window = runtime.createWindow({
        .size = {600, 500},
        .title = "Test: Focus Navigation"
    });

    auto makeFocusable = [&](const std::string& label, const std::string& key, Color bg) -> View {
        return Button {
            .text = label,
            .backgroundColor = bg,
            .padding = EdgeInsets(12, 24, 12, 24),
            .cornerRadius = 6,
            .focusKey = key,
            .onFocus = [&focusLog, &focusCount, key]() {
                focusLog = key;
                focusCount = static_cast<int>(focusCount) + 1;
            },
            .onBlur = [&blurCount]() {
                blurCount = static_cast<int>(blurCount) + 1;
            },
            .onClick = [&focusLog, key]() {
                focusLog = key + "-clicked";
            }
        };
    };

    window.setRootView(
        VStack {
            .padding = 20,
            .spacing = 16,
            .children = {
                Text {
                    .value = "focus-test-root",
                    .fontSize = 18,
                    .fontWeight = FontWeight::bold,
                    .color = Colors::black
                },

                VStack {
                    .spacing = 8,
                    .children = {
                        makeFocusable("Item Alpha", "focus-alpha", Colors::blue),
                        makeFocusable("Item Beta", "focus-beta", Colors::green),
                        makeFocusable("Item Gamma", "focus-gamma", Colors::red),
                        makeFocusable("Item Delta", "focus-delta", Color::hex(0xFF9800)),
                        makeFocusable("Item Epsilon", "focus-epsilon", Color::hex(0x9C27B0))
                    }
                },

                Divider {},

                VStack {
                    .spacing = 4,
                    .children = {
                        Text {
                            .value = [&]() { return std::format("focus-current:{}", static_cast<std::string>(focusLog)); },
                            .fontSize = 12,
                            .color = Colors::black
                        },
                        Text {
                            .value = [&]() { return std::format("focus-count:{}", static_cast<int>(focusCount)); },
                            .fontSize = 12,
                            .color = Colors::black
                        },
                        Text {
                            .value = [&]() { return std::format("blur-count:{}", static_cast<int>(blurCount)); },
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
