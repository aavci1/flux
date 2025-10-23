#include <Flux.hpp>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    Property counter = 0;

    Window window({
        .size = {400, 300},
        .title = "Counter"
    });

    window.setRootView(
        VStack {
            .padding = EdgeInsets{32},
            .children = {
                Text {
                    .value = "Counter",
                    .fontSize = 24
                },
                Text {
                    .expansionBias = 1.0f,
                    .value = [&]() {
                        return std::format("{}", counter);
                    },
                    .fontSize = 32
                },
                HStack {
                    .spacing = 16,
                    .children = {
                        Button {
                            .text = "-",
                            .expansionBias = 1.0f,
                            .onClick = [&]() { counter--; }
                        },
                        Button {
                            .text = "Reset",
                            .expansionBias = 1.0f,
                            .onClick = [&]() { counter = 0; }
                        },
                        Button {
                            .text = "+",
                            .expansionBias = 1.0f,
                            .onClick = [&]() { counter++; }
                        }
                    }
                }
            }
        }
    );

    return app.exec();
}
