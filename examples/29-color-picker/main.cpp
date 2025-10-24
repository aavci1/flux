#include <Flux.hpp>
#include <string>
#include <array>
#include <cstdlib>
#include <format>

using namespace flux;

struct ColorInfo {
    std::string name;
    std::string hex;
};

constexpr std::array<ColorInfo, 80> beautifulColors = {{
    // Reds & Pinks
    {"Crimson", "#DC143C"},
    {"Rose", "#FF007F"},
    {"Cherry", "#DE3163"},
    {"Coral", "#FF7F50"},
    {"Salmon", "#FA8072"},
    {"Blush", "#DE5D83"},
    {"Raspberry", "#E30B5C"},
    {"Burgundy", "#800020"},
    {"Ruby", "#E0115F"},
    {"Watermelon", "#FC6C85"},
    
    // Oranges & Yellows
    {"Tangerine", "#F28500"},
    {"Peach", "#FFE5B4"},
    {"Apricot", "#FBCEB1"},
    {"Amber", "#FFBF00"},
    {"Gold", "#FFD700"},
    {"Honey", "#FFC30B"},
    {"Marigold", "#EAA221"},
    {"Sunflower", "#FFDA03"},
    {"Lemon", "#FFF44F"},
    {"Canary", "#FFFF99"},
    
    // Greens
    {"Mint", "#98FF98"},
    {"Sage", "#9DC183"},
    {"Emerald", "#50C878"},
    {"Forest", "#228B22"},
    {"Jade", "#00A86B"},
    {"Olive", "#808000"},
    {"Lime", "#CCFF00"},
    {"Seafoam", "#93E9BE"},
    {"Pistachio", "#93C572"},
    {"Chartreuse", "#7FFF00"},
    
    // Blues
    {"Sky", "#87CEEB"},
    {"Azure", "#007FFF"},
    {"Cerulean", "#007BA7"},
    {"Cobalt", "#0047AB"},
    {"Navy", "#000080"},
    {"Teal", "#008080"},
    {"Turquoise", "#40E0D0"},
    {"Aquamarine", "#7FFFD4"},
    {"Powder", "#B0E0E6"},
    {"Steel", "#4682B4"},
    
    // Purples
    {"Lavender", "#E6E6FA"},
    {"Lilac", "#C8A2C8"},
    {"Violet", "#8F00FF"},
    {"Purple", "#800080"},
    {"Plum", "#8E4585"},
    {"Mauve", "#E0B0FF"},
    {"Orchid", "#DA70D6"},
    {"Magenta", "#FF00FF"},
    {"Indigo", "#4B0082"},
    {"Amethyst", "#9966CC"},
    
    // Browns & Neutrals
    {"Chocolate", "#7B3F00"},
    {"Coffee", "#6F4E37"},
    {"Caramel", "#FFD59A"},
    {"Tan", "#D2B48C"},
    {"Beige", "#F5F5DC"},
    {"Cream", "#FFFDD0"},
    {"Ivory", "#FFFFF0"},
    {"Taupe", "#483C32"},
    {"Sepia", "#704214"},
    {"Sand", "#C2B280"},
    
    // Grays & Blacks
    {"Charcoal", "#36454F"},
    {"Slate", "#708090"},
    {"Silver", "#C0C0C0"},
    {"Pearl", "#EAE0C8"},
    {"Ash", "#B2BEB5"},
    {"Smoke", "#738276"},
    {"Graphite", "#383428"},
    {"Onyx", "#353839"},
    {"Ebony", "#555D50"},
    {"Pewter", "#96A8A1"},
    
    // Unique & Exotic
    {"Champagne", "#F7E7CE"},
    {"Rose Gold", "#B76E79"},
    {"Periwinkle", "#CCCCFF"},
    {"Peacock", "#33A1C9"},
    {"Saffron", "#F4C430"},
    {"Mulberry", "#C54B8C"},
    {"Vermillion", "#E34234"},
    {"Cyan", "#00FFFF"},
    {"Fuchsia", "#FF00FF"},
    {"Scarlet", "#FF2400"}      
}};

// Helper function to convert hex string to Color
Color hexToColor(const std::string& hex) {
    std::string hexClean = hex;
    if (hexClean[0] == '#') {
        hexClean = hexClean.substr(1);
    }
    
    unsigned int r, g, b;
    sscanf(hexClean.c_str(), "%02x%02x%02x", &r, &g, &b);
    
    return Color{
        static_cast<float>(r) / 255.0f,
        static_cast<float>(g) / 255.0f,
        static_cast<float>(b) / 255.0f,
        1.0f
    };
}

struct ColorCard {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> name = "";
    Property<std::string> hex = "#000000";
    Property<Color> color = Colors::black;

    View body() const {
        return VStack {
            .cursor = CursorType::Pointer,
            .children = {
                Text {
                    .backgroundColor = color,
                    .cornerRadius = CornerRadius{8, 8, 0, 0},
                    .expansionBias = 1.0
                },
                VStack {
                    .backgroundColor = Colors::white,
                    .borderWidth = 1,
                    .borderColor = Colors::lightGray,
                    .cornerRadius = CornerRadius{0, 0, 8, 8},
                    .justifyContent = JustifyContent::center,
                    .alignItems = AlignItems::end,
                    .padding = 8,
                    .children = {
                        Text {
                            .value = name,
                            .color = Colors::black,
                            .fontSize = 12,
                            .fontWeight = FontWeight::bold,
                        },
                        Text {
                            .value = hex,
                            .color = Colors::black,
                            .fontSize = 10,
                            .fontWeight = FontWeight::medium,
                        }
                    }
                }
            }
        };
    }
};

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    // State to track last copied color
    Property<std::string> lastCopied = std::string("");

    Window window({
        .size = {1200, 900},
        .title = "Color Picker - Click a color to copy its hex code"
    });

    // Create color cells
    std::vector<View> colorCells;
    for (const auto& color : beautifulColors) {
        colorCells.push_back(ColorCard {
            .name = color.name,
            .hex = color.hex,
            .color = hexToColor(color.hex)
        });
    }

    window.setRootView(
        Grid {
            .padding = 16,
            .spacing = 16,
            .rows = 8,
            .columns = 10,
            .children = colorCells
        }
    );

    return app.exec();
}

