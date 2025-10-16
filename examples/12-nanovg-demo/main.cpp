#include <Flux.hpp>
#include <iostream>

using namespace flux;

int main(int argc, char* argv[]) {
    try {
        // Create application
        Application app(argc, argv);

        // Create window with GLFW + NanoVG backend
        Window window({
            .size = {1280, 720},
            .title = "Flux GLFW + NanoVG Demo",
            .backend = WindowBackend::Default // Explicitly request default backend
        });

        // Create a simple UI
        auto rootView = VStack {
            .children = {
                Text {
                    .fontSize = 32,
                    .value = "Hello, GLFW + NanoVG!",
                    .color = Color::rgb(0, 0, 255) // Blue color
                },
                Spacer(),
                HStack {
                    .children = {
                        Text {
                            .value = "Left side"
                        },
                        Spacer(),
                        Text {
                            .value = "Right side"
                        }
                    }
                },
                Spacer(),
                Text {
                    .fontSize = 16,
                    .value = "This is rendered with GLFW for windowing and NanoVG for graphics!",
                    .color = Color::rgb(128, 128, 128) // Gray color
                }
            }
        };

        window.setRootView(rootView);

        std::cout << "GLFW + NanoVG demo running. Close the window to exit.\n";

        // Run the application
        return app.exec();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
