#include <Flux.hpp>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    Window window({
        .size = {800, 600},
        .title = "Flux Spacer Grid Demo - Colspan & Rowspan"
    });

    window.setRootView(
        Grid {
            .columns = 4,
            .rows = 4,
            .spacing = 10,
            .padding = EdgeInsets{20, 20, 20, 20},
            .backgroundColor = Colors::lightGray,
            .children = {
                // Regular text cells
                Text{.value = "A", .backgroundColor = Colors::red, .fontSize = 24},
                Text{.value = "B", .backgroundColor = Colors::green, .fontSize = 24},
                Text{.value = "C", .backgroundColor = Colors::blue, .fontSize = 24},
                Text{.value = "D", .backgroundColor = Colors::yellow, .fontSize = 24},
                
                // Spacer spanning 2x2
                Spacer{.backgroundColor = Colors::darkGray, .colspan = 2, .rowspan = 2},
                
                // Spacer spanning 2 columns
                Spacer{.backgroundColor = Colors::gray, .colspan = 2},
                
                // Regular text cell
                Text{.value = "F", .backgroundColor = Colors::gray, .fontSize = 24},
                
                // Spacer spanning 2 rows
                Spacer{.backgroundColor = Colors::black, .rowspan = 2},
                
                // Regular text cells
                Text{.value = "H", .backgroundColor = Colors::white, .fontSize = 24},
                Text{.value = "I", .backgroundColor = Colors::red, .fontSize = 24},
                
                // Regular text cells
                Text{.value = "K", .backgroundColor = Colors::blue, .fontSize = 24},
                Text{.value = "L", .backgroundColor = Colors::yellow, .fontSize = 24}
            }
        }
    );

    return app.exec();
}
