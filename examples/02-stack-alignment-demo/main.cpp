#include <Flux.hpp>

using namespace flux;
int main(int argc, char* argv[]) {
    Application app(argc, argv);

    Window window({
        .size = {600, 600},
        .title = "Layouts and Text Alignment"
    });

    auto verticalAlignment = std::vector<VerticalAlignment>({
        VerticalAlignment::top,
        VerticalAlignment::center,
        VerticalAlignment::bottom
    });

    auto horizontalAlignment = std::vector<HorizontalAlignment>({
        HorizontalAlignment::leading,
        HorizontalAlignment::center,
        HorizontalAlignment::trailing
    });

    window.setRootView(
        VStack {
            .children = map(verticalAlignment, [&](VerticalAlignment verticalAlignment) {
                return HStack {
                    .expansionBias = 1.0f,
                    .children = map(horizontalAlignment, [&](HorizontalAlignment horizontalAlignment) {
                        return Text {
                            .expansionBias = 1.0f,
                            .value = "Hello, world!",
                            .backgroundColor = Colors::lightGray,
                            .borderColor = Colors::gray,
                            .borderWidth = 1,
                            .horizontalAlignment = horizontalAlignment,
                            .verticalAlignment = verticalAlignment
                        };
                    })
                };
            })
        }
    );

    return app.exec();
}
