#include <Flux.hpp>
#include <iostream>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    // Cursor types and names
    struct CursorInfo {
        CursorType type;
        const char* name;
    };

    const CursorInfo cursors[] = {
        {CursorType::Default, "Default"},
        {CursorType::Pointer, "Pointer"},
        {CursorType::Text, "Text"},
        {CursorType::Crosshair, "Crosshair"},
        {CursorType::Move, "Move"},
        {CursorType::ResizeNS, "Resize NS"},
        {CursorType::ResizeEW, "Resize EW"},
        {CursorType::ResizeNESW, "Resize NESW"},
        {CursorType::ResizeNWSE, "Resize NWSE"},
        {CursorType::NotAllowed, "Not Allowed"},
        {CursorType::Wait, "Wait"},
        {CursorType::Progress, "Progress"},
        {CursorType::Help, "Help"},
        {CursorType::Grab, "Grab"},
        {CursorType::Grabbing, "Grabbing"},
        {CursorType::ZoomIn, "Zoom In"},
        {CursorType::ZoomOut, "Zoom Out"},
    };

    const int cursorCount = sizeof(cursors) / sizeof(cursors[0]);

    Window window({
        .size = {600, 500},
        .title = "Cursor Demo"
    });

    window.setRootView(
        VStack {
            .padding = EdgeInsets{32},
            .spacing = 24,
            .backgroundColor = Color::hex(0xF5F5F5),
            .children = {
                Text {
                    .value = "Cursor Demo",
                    .fontSize = 32,
                    .fontWeight = FontWeight::bold,
                    .color = Color::hex(0x333333)
                },
                
                Text {
                    .value = "Click buttons to change the cursor",
                    .fontSize = 16,
                    .color = Color::hex(0x666666)
                },

                // Grid of cursor buttons
                Grid {
                    .columns = 3,
                    .spacing = 12,
                    .expansionBias = 1.0f,
                    .children = [&]() {
                        std::vector<View> buttons;
                        for (int i = 0; i < cursorCount; i++) {
                            buttons.push_back(View(Button {
                                .text = cursors[i].name,
                                .padding = EdgeInsets(12, 16),
                                .backgroundColor = Color::hex(0x2196F3),
                                .cornerRadius = 8,
                                .onClick = [&window, &cursors, i]() {
                                    window.setCursor(cursors[i].type);
                                    std::cout << "[CursorDemo] Changed cursor to: " 
                                              << cursors[i].name << std::endl;
                                }
                            }));
                        }
                        return buttons;
                    }()
                },

                // Reset button
                Button {
                    .text = "Reset to Default",
                    .padding = EdgeInsets(12, 24),
                    .backgroundColor = Color::hex(0x4CAF50),
                    .cornerRadius = 8,
                    .onClick = [&window]() {
                        window.setCursor(CursorType::Default);
                        std::cout << "[CursorDemo] Reset to default cursor\n";
                    }
                }
            }
        }
    );

    // Run the application
    return app.exec();
}
