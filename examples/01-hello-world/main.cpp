#include <Flux.hpp>

using namespace flux;

int main(int argc, char* argv[]) {
    Runtime runtime(argc, argv);

    auto& window = runtime.createWindow({
        .size = {400, 400},
        .title = "Flux App"
    });

    window.setRootView(
        Text {
            .value = "Hello, world!"
        }
    );

    return runtime.run();
}
