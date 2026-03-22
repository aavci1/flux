#include <Flux.hpp>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    auto& window = app.createWindow({
        .size = {400, 300},
        .title = "Flux"
    });


    window.setRootView(
        Text {
            .value = "Hello, World!"
        }
    );

    return app.exec();
}