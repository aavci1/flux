#include <Flux.hpp>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    Window window({
        .size = {400, 400},
        .title = "Flux App"
    });

    window.setRootView(
        Text {
            .fontSize = 48,
            .value = "Hello, world!"
        }
    );

    return app.exec();
}
