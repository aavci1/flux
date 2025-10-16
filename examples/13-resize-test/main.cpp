#include <Flux.hpp>
#include <iostream>
#include <thread>
#include <chrono>

using namespace flux;

int main(int argc, char* argv[]) {
    try {
        // Create application
        Application app(argc, argv);

        // Create window with GLFW + NanoVG backend
        Window window({
            .size = {800, 600},
            .title = "Flux Resize Test",
            .backend = WindowBackend::Default
        });

        // Create a simple UI that shows the current window size
        auto rootView = VStack {
            .children = {
                Text {
                    .fontSize = 24,
                    .value = "Resize Test Window",
                    .color = Color::rgb(0, 0, 255)
                },
                Spacer(),
                Text {
                    .fontSize = 16,
                    .value = "Try resizing this window!",
                    .color = Color::rgb(128, 128, 128)
                },
                Spacer(),
                Text {
                    .fontSize = 14,
                    .value = "The content should redraw properly when you resize.",
                    .color = Color::rgb(64, 64, 64)
                }
            }
        };

        window.setRootView(rootView);

        std::cout << "Resize test running. Try resizing the window to test the fix.\n";
        std::cout << "Close the window to exit.\n";

        // Run the application
        return app.exec();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
