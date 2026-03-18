#include <Flux.hpp>
#include <format>

using namespace flux;

int main(int argc, char* argv[]) {
    Runtime runtime(argc, argv);

    Property<float> scrollPos = 0.0f;

    auto& window = runtime.createWindow({
        .size = {500, 500},
        .title = "Test: ScrollArea"
    });

    std::vector<View> items;
    for (int i = 0; i < 40; i++) {
        items.push_back(
            Text {
                .value = std::format("scroll-item-{}", i),
                .fontSize = 14,
                .color = Colors::black,
                .backgroundColor = (i % 2 == 0) ? Color::hex(0xF0F0F0) : Color::hex(0xFFFFFF),
                .padding = EdgeInsets(8, 12, 8, 12)
            }
        );
    }

    window.setRootView(
        VStack {
            .padding = 16,
            .spacing = 10,
            .children = {
                Text {
                    .value = "scroll-test-root",
                    .fontSize = 18,
                    .fontWeight = FontWeight::bold,
                    .color = Colors::black
                },

                Text {
                    .value = [&]() { return std::format("scroll-y:{:.0f}", static_cast<float>(scrollPos)); },
                    .fontSize = 12,
                    .color = Colors::black
                },

                ScrollArea {
                    .expansionBias = 1.0f,
                    .backgroundColor = Colors::white,
                    .borderColor = Colors::lightGray,
                    .borderWidth = 1,
                    .cornerRadius = 4,
                    .children = std::move(items),
                    .onChange = [&]() {
                        // We can't easily read scrollY from ScrollArea directly,
                        // but the test can verify items shift via bounds
                    }
                }
            }
        }
    );

    return runtime.run();
}
