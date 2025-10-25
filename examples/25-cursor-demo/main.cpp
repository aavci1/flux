#include <Flux.hpp>
#include <iostream>

using namespace flux;

template <typename T, typename U, typename F>
std::vector<U> map(const std::vector<T>& arr, F fn) {
    std::vector<U> result;
    std::transform(arr.begin(), arr.end(), std::back_inserter(result), fn);
    return result;
}

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    // Cursor types and names
    struct CursorInfo {
        CursorType type;
        const char* name;
    };

    const std::vector<CursorInfo> cursors = {
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
                    .value = "Hover over the buttons to see the cursor",
                    .fontSize = 16,
                    .color = Color::hex(0x666666)
                },

                // Grid of cursor buttons
                Grid {
                    .rows = 4,
                    .columns = 4,
                    .spacing = 12,
                    .expansionBias = 1.0f,
                    .children = map(cursors, [](const CursorInfo& cursor) {
                        return Button {
                            .cursor = cursor.type,
                            .text = cursor.name,
                            .cornerRadius = 8
                        };
                    })
                }
            }
        }
    );

    // Run the application
    return app.exec();
}
