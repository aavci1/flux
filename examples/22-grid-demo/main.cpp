#include <Flux.hpp>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    Window window({
        .size = {800, 600},
        .title = "Flux Grid Demo - Colspan & Rowspan"
    });

    window.setRootView(
        Grid {
            .columns = 4,
            .rows = 4,
            .spacing = 10,
            .padding = EdgeInsets{20, 20, 20, 20},
            .backgroundColor = Colors::lightGray,
            .children = {
                // Regular 1x1 cells
                Text{.value = "A", .backgroundColor = Colors::red, .fontSize = 24},
                Text{.value = "B", .backgroundColor = Colors::green, .fontSize = 24},
                Text{.value = "C", .backgroundColor = Colors::blue, .fontSize = 24},
                Text{.value = "D", .backgroundColor = Colors::yellow, .fontSize = 24},
                
                // Large cell spanning 2x2 - place early to ensure space
                Text{.value = "J\n(2x2)", .backgroundColor = Colors::green, .fontSize = 18, .colspan = 2, .rowspan = 2},
                
                // Colspan example - spans 2 columns
                Text{.value = "E (2 cols)", .backgroundColor = Colors::darkGray, .fontSize = 20, .colspan = 2},
                
                // Regular cell
                Text{.value = "F", .backgroundColor = Colors::gray, .fontSize = 24},
                
                // Rowspan example - spans 2 rows
                Text{.value = "G\n(2 rows)", .backgroundColor = Colors::black, .fontSize = 20, .rowspan = 2},
                
                // Regular cells
                Text{.value = "H", .backgroundColor = Colors::white, .fontSize = 24},
                Text{.value = "I", .backgroundColor = Colors::red, .fontSize = 24},
                
                // Regular cells
                Text{.value = "K", .backgroundColor = Colors::blue, .fontSize = 24},
                Text{.value = "L", .backgroundColor = Colors::yellow, .fontSize = 24}
            }
        }
    );

    return app.exec();
}
