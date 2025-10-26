#include <Flux.hpp>
#include <fstream>
#include <iostream>
#include <string>

using namespace flux;


std::string readSVGFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << filename << "'" << std::endl;
        return "";
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    if (content.empty()) {
        std::cerr << "Error: File '" << filename << "' is empty" << std::endl;
        return "";
    }
    
    return content;
}

int main(int argc, char* argv[]) {
    // Check command line arguments
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <svg_filename>" << std::endl;
        std::cerr << "Example: " << argv[0] << " nemo.svg" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    std::string svgContent = readSVGFile(filename);
    
    if (svgContent.empty()) {
        std::cerr << "Failed to load SVG file: " << filename << std::endl;
        return 1;
    }

    Application app(argc, argv);

    Window window({
        .size = {800, 600},
        .title = "SVG Demo - " + filename
    });

    window.setRootView(
        VStack {
            .spacing = 24,
            .padding = 24,
            .children = {
                Text {
                    .value = "SVG Demo with NanoSVG",
                    .fontSize = 24,
                    .fontWeight = FontWeight::bold,
                    .color = Colors::black
                },
                Text {
                    .value = "Loading: " + filename,
                    .fontSize = 16,
                    .color = Colors::gray
                },
                SVG {
                    .clip = true,
                    .expansionBias = true,
                    .padding = 8,
                    .content = svgContent
                }
            }
        }
    );

    std::cout << "SVG Demo started. Loading file: " << filename << std::endl;
    std::cout << "Press Ctrl+C to exit." << std::endl;

    return app.exec();
}