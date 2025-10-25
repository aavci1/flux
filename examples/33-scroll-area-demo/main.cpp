#include <Flux.hpp>
#include <format>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    Window window({
        .size = {1000, 700},
        .title = "Flux ScrollArea Demo"
    });

    // Create content items for the scroll area
    std::vector<View> scrollContent;
    
    for (int i = 1; i <= 30; i++) {
        scrollContent.push_back(
            VStack {
                .padding = 16,
                .spacing = 8,
                .backgroundColor = (i % 2 == 0) ? Colors::lightGray : Colors::white,
                .children = {
                    Text {
                        .value = std::format("Item {}", i),
                        .fontSize = 18,
                        .fontWeight = FontWeight::bold,
                        .color = Colors::black,
                        .horizontalAlignment = HorizontalAlignment::leading
                    },
                    Text {
                        .value = std::format("This is content item number {}. It contains some text to demonstrate scrolling.", i),
                        .fontSize = 14,
                        .color = Colors::gray,
                        .horizontalAlignment = HorizontalAlignment::leading
                    }
                }
            }
        );
    }

    window.setRootView(
        VStack {
            .padding = 16,
            .spacing = 16,
            .backgroundColor = Color::hex(0xf5f5f5),
            .children = {
                // Title
                Text {
                    .value = "ScrollArea Component Demo",
                    .fontSize = 32,
                    .fontWeight = FontWeight::bold,
                    .color = Colors::black,
                    .horizontalAlignment = HorizontalAlignment::center
                },
                
                Text {
                    .value = "Use mouse wheel to scroll",
                    .fontSize = 14,
                    .color = Colors::gray,
                    .horizontalAlignment = HorizontalAlignment::center
                },

                // Main scroll area
                ScrollArea {
                    .children = scrollContent,
                    .backgroundColor = Colors::white,
                    .borderColor = Colors::lightGray,
                    .borderWidth = 2,
                    .cornerRadius = 8,
                    .expansionBias = 1.0f  // Allow the scroll area to expand
                }
            }
        }
    );

    return app.exec();
}

