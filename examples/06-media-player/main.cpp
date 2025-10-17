#include <Flux.hpp>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    Window window({
        .size = {400, 300},
        .title = "Media Player"
    });

    window.setRootView(
        VStack {
            .backgroundColor = Color::hex(0xf8f9fa),
            .spacing = 24,
            .padding = 32,
        }
    );

    return app.exec();
}