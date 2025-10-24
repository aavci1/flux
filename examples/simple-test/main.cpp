#include <Flux.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        flux::Application app(argc, argv);
        
        auto window = std::make_unique<flux::Window>(flux::WindowConfig{
            .size = {400, 300},
            .title = "Simple Test",
            .resizable = true
        });
        
        window->setRootView(
            flux::Text {
                .value = "Hello World",
                .fontSize = 24,
                .color = flux::Colors::black,
                .horizontalAlignment = flux::HorizontalAlignment::center,
                .verticalAlignment = flux::VerticalAlignment::center
            }
        );
        
        app.registerWindow(window.get());
        
        std::cout << "Starting app loop...\n";
        return app.exec();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
