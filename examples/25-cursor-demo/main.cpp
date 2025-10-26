#include <Flux.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

using namespace flux;

template <typename T, typename U, typename F>
std::vector<U> map(const std::vector<T>& arr, F fn) {
    std::vector<U> result;
    std::transform(arr.begin(), arr.end(), std::back_inserter(result), fn);
    return result;
}

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    // Cursor types with names and colors
    struct CursorInfo {
        CursorType type;
        const char* name;
        uint32_t backgroundColor;
        uint32_t textColor;
    };

    const std::vector<CursorInfo> cursors = {
        {CursorType::Pointer, "Pointer", 0xF5F5F5, 0x333333},
        {CursorType::Text, "Text", 0xE3F2FD, 0x1565C0},
        {CursorType::Crosshair, "Crosshair", 0xFFF3E0, 0xE65100},
        {CursorType::Move, "Move", 0xE8F5E9, 0x2E7D32},
        {CursorType::ResizeNS, "Resize NS", 0xE1BEE7, 0x6A1B9A},
        {CursorType::ResizeEW, "Resize EW", 0xFFECB3, 0xF57F17},
        {CursorType::ResizeNESW, "Resize NESW", 0xB2DFDB, 0x00695C},
        {CursorType::ResizeNWSE, "Resize NWSE", 0xC5E1A5, 0x558B2F},
        {CursorType::NotAllowed, "Not Allowed", 0xFFEBEE, 0xC62828},
        {CursorType::Wait, "Wait", 0xF3E5F5, 0x6A1B9A},
        {CursorType::Progress, "Progress", 0xFFE0B2, 0xE65100},
        {CursorType::Help, "Help", 0xE0F2F1, 0x00695C},
        {CursorType::Grab, "Grab", 0xE3F2FD, 0x1565C0},
        {CursorType::Grabbing, "Grabbing", 0xE1BEE7, 0x7B1FA2},
        {CursorType::ZoomIn, "Zoom In", 0xC8E6C9, 0x2E7D32},
        {CursorType::ZoomOut, "Zoom Out", 0xDCEDC8, 0x33691E},
    };

    Window window({
        .size = {700, 600},
        .title = "Cursor Demo"
    });

    window.setRootView(
        VStack {
            .padding = EdgeInsets{32},
            .spacing = 24,
            .backgroundColor = Color::hex(0xFAFAFA),
            .children = {
                VStack {
                    .spacing = 12,
                    .alignItems = AlignItems::center,
                    .children = {
                        Text {
                            .value = "Cursor Demo",
                            .fontSize = 36,
                            .fontWeight = FontWeight::bold,
                            .color = Color::hex(0x212121)
                        },
                        
                        Text {
                            .value = "Hover over the colored boxes to see different cursor types",
                            .fontSize = 14,
                            .color = Color::hex(0x757575)
                        }
                    }
                },

                // Grid of all cursor types
                Grid {
                    .rows = 4,
                    .columns = 4,
                    .spacing = 12,
                    .expansionBias = 1.0f,
                    .children = map(cursors, [](const CursorInfo& cursor) {
                        return Text {
                            .backgroundColor = Color::hex(cursor.backgroundColor),
                            .color = Color::hex(cursor.textColor),
                            .cornerRadius = 12,
                            .cursor = cursor.type,
                            .fontSize = 14,
                            .fontWeight = FontWeight::bold,
                            .padding = EdgeInsets{20},
                            .value = cursor.name
                        };
                    })
                }
            }
        }
    );

    return app.exec();
}
