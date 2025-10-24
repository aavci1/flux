#include <Flux.hpp>
#include <format>
#include <cmath>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    // State for scroll tracking
    Property<std::string> scrollLog = "No scroll events yet";
    Property<float> totalScrollY = 0.0f;
    Property<float> totalScrollX = 0.0f;
    Property<int> scrollCount = 0;
    Property<float> zoomLevel = 1.0f;
    Property<float> imageOffsetX = 0.0f;
    Property<float> imageOffsetY = 0.0f;
    Property<std::string> mouseScrollPos = "0, 0";

    Window window({
        .size = {1000, 800},
        .title = "Flux Scroll/Wheel Events Demo"
    });

    window.setRootView(
        VStack {
            .padding = 20,
            .spacing = 20,
            .backgroundColor = Color::hex(0xf5f5f5),
            .children = {
                // Title
                Text {
                    .value = "Scroll/Wheel Events Demo",
                    .fontSize = 32,
                    .fontWeight = FontWeight::bold,
                    .color = Colors::black,
                    .horizontalAlignment = HorizontalAlignment::center
                },

                // Basic Scroll Events Section
                VStack {
                    .padding = 16,
                    .spacing = 12,
                    .backgroundColor = Colors::white,
                    .borderColor = Colors::lightGray,
                    .borderWidth = 1,
                    .cornerRadius = 8,
                    .children = {
                        Text {
                            .value = "Basic Scroll Events",
                            .fontSize = 20,
                            .fontWeight = FontWeight::bold,
                            .color = Colors::black
                        },
                        
                        Text {
                            .value = "Hover over the box below and use your mouse wheel or touchpad",
                            .fontSize = 14,
                            .color = Colors::gray
                        },

                        // Scroll detector box
                        VStack {
                            .padding = 40,
                            .backgroundColor = Color::hex(0xe3f2fd),
                            .borderColor = Colors::blue,
                            .borderWidth = 2,
                            .cornerRadius = 8,
                            .onScroll = [&scrollLog, &totalScrollY, &totalScrollX, &scrollCount, &mouseScrollPos]
                                       (float x, float y, float deltaX, float deltaY) {
                                scrollCount = static_cast<int>(scrollCount) + 1;
                                totalScrollY = static_cast<float>(totalScrollY) + deltaY;
                                totalScrollX = static_cast<float>(totalScrollX) + deltaX;
                                mouseScrollPos = std::format("{:.0f}, {:.0f}", x, y);
                                
                                std::string direction;
                                if (std::abs(deltaY) > std::abs(deltaX)) {
                                    direction = deltaY > 0 ? "Down" : "Up";
                                } else {
                                    direction = deltaX > 0 ? "Right" : "Left";
                                }
                                
                                scrollLog = std::format("Scrolled {} (ΔX={:.1f}, ΔY={:.1f})", 
                                                       direction, deltaX, deltaY);
                            },
                            .children = {
                                Text {
                                    .value = "Scroll Here 🖱️",
                                    .fontSize = 24,
                                    .fontWeight = FontWeight::bold,
                                    .color = Colors::blue,
                                    .horizontalAlignment = HorizontalAlignment::center
                                },
                                Text {
                                    .value = [&mouseScrollPos]() {
                                        return "Mouse: " + static_cast<std::string>(mouseScrollPos);
                                    },
                                    .fontSize = 14,
                                    .color = Colors::gray,
                                    .horizontalAlignment = HorizontalAlignment::center
                                }
                            }
                        },

                        // Scroll statistics
                        VStack {
                            .padding = 12,
                            .spacing = 8,
                            .backgroundColor = Color::hex(0xf0f0f0),
                            .cornerRadius = 4,
                            .children = {
                                Text {
                                    .value = [&scrollLog]() {
                                        return "Last event: " + static_cast<std::string>(scrollLog);
                                    },
                                    .fontSize = 14,
                                    .color = Colors::blue
                                },
                                Text {
                                    .value = [&scrollCount]() {
                                        return std::format("Total scroll events: {}", static_cast<int>(scrollCount));
                                    },
                                    .fontSize = 12,
                                    .color = Colors::gray
                                },
                                Text {
                                    .value = [&totalScrollY, &totalScrollX]() {
                                        return std::format("Accumulated: X={:.1f}, Y={:.1f}", 
                                                         static_cast<float>(totalScrollX),
                                                         static_cast<float>(totalScrollY));
                                    },
                                    .fontSize = 12,
                                    .color = Colors::gray
                                }
                            }
                        }
                    }
                },

                // Zoom Demo Section
                VStack {
                    .padding = 16,
                    .spacing = 12,
                    .backgroundColor = Colors::white,
                    .borderColor = Colors::lightGray,
                    .borderWidth = 1,
                    .cornerRadius = 8,
                    .children = {
                        Text {
                            .value = "Scroll to Zoom Demo",
                            .fontSize = 20,
                            .fontWeight = FontWeight::bold,
                            .color = Colors::black
                        },
                        
                        Text {
                            .value = "Scroll over the colored box to zoom in/out",
                            .fontSize = 14,
                            .color = Colors::gray
                        },

                        // Zoomable box
                        VStack {
                            .padding = 60,
                            .backgroundColor = Color::hex(0xf0f0f0),
                            .cornerRadius = 8,
                            .children = {
                                VStack {
                                    .scaleX = [&zoomLevel]() { return static_cast<float>(zoomLevel); },
                                    .scaleY = [&zoomLevel]() { return static_cast<float>(zoomLevel); },
                                    .padding = 30,
                                    .backgroundColor = Color::hex(0xff9800),
                                    .cornerRadius = 8,
                                    .onScroll = [&zoomLevel](float x, float y, float deltaX, float deltaY) {
                                        (void)x; (void)y; (void)deltaX;
                                        // Zoom in when scrolling up, zoom out when scrolling down
                                        float zoom = static_cast<float>(zoomLevel);
                                        zoom -= deltaY * 0.01f;
                                        zoom = std::max(0.5f, std::min(3.0f, zoom));
                                        zoomLevel = zoom;
                                    },
                                    .children = {
                                        Text {
                                            .value = [&zoomLevel]() {
                                                return std::format("Zoom: {:.1f}x", static_cast<float>(zoomLevel));
                                            },
                                            .fontSize = 18,
                                            .fontWeight = FontWeight::bold,
                                            .color = Colors::white,
                                            .horizontalAlignment = HorizontalAlignment::center
                                        }
                                    }
                                }
                            }
                        }
                    }
                },

                // Pan Demo Section
                VStack {
                    .padding = 16,
                    .spacing = 12,
                    .backgroundColor = Colors::white,
                    .borderColor = Colors::lightGray,
                    .borderWidth = 1,
                    .cornerRadius = 8,
                    .children = {
                        Text {
                            .value = "Scroll to Pan Demo",
                            .fontSize = 20,
                            .fontWeight = FontWeight::bold,
                            .color = Colors::black
                        },
                        
                        Text {
                            .value = "Use mouse wheel to pan the colored box",
                            .fontSize = 14,
                            .color = Colors::gray
                        },

                        // Pannable area
                        VStack {
                            .padding = 60,
                            .backgroundColor = Color::hex(0xf0f0f0),
                            .cornerRadius = 8,
                            .clip = true,
                            .onScroll = [&imageOffsetX, &imageOffsetY](float x, float y, float deltaX, float deltaY) {
                                (void)x; (void)y;
                                imageOffsetX = static_cast<float>(imageOffsetX) - deltaX;
                                imageOffsetY = static_cast<float>(imageOffsetY) - deltaY;
                            },
                            .children = {
                                VStack {
                                    .offset = [&imageOffsetX, &imageOffsetY]() {
                                        return Point{static_cast<float>(imageOffsetX), 
                                                    static_cast<float>(imageOffsetY)};
                                    },
                                    .padding = 30,
                                    .backgroundColor = Color::hex(0x4caf50),
                                    .cornerRadius = 8,
                                    .children = {
                                        Text {
                                            .value = [&imageOffsetX, &imageOffsetY]() {
                                                return std::format("Offset: ({:.0f}, {:.0f})", 
                                                                 static_cast<float>(imageOffsetX),
                                                                 static_cast<float>(imageOffsetY));
                                            },
                                            .fontSize = 16,
                                            .fontWeight = FontWeight::bold,
                                            .color = Colors::white,
                                            .horizontalAlignment = HorizontalAlignment::center
                                        }
                                    }
                                }
                            }
                        }
                    }
                },

                // Reset buttons
                HStack {
                    .spacing = 12,
                    .children = {
                        Button {
                            .text = "Reset Scroll Stats",
                            .backgroundColor = Colors::blue,
                            .cornerRadius = 6,
                            .padding = 12,
                            .onClick = [&scrollLog, &totalScrollY, &totalScrollX, &scrollCount]() {
                                scrollLog = "No scroll events yet";
                                totalScrollY = 0.0f;
                                totalScrollX = 0.0f;
                                scrollCount = 0;
                            }
                        },
                        Button {
                            .text = "Reset Zoom",
                            .backgroundColor = Colors::green,
                            .cornerRadius = 6,
                            .padding = 12,
                            .onClick = [&zoomLevel]() {
                                zoomLevel = 1.0f;
                            }
                        },
                        Button {
                            .text = "Reset Pan",
                            .backgroundColor = Color::hex(0xff9800),
                            .cornerRadius = 6,
                            .padding = 12,
                            .onClick = [&imageOffsetX, &imageOffsetY]() {
                                imageOffsetX = 0.0f;
                                imageOffsetY = 0.0f;
                            }
                        },
                        Button {
                            .text = "Reset All",
                            .backgroundColor = Colors::red,
                            .cornerRadius = 6,
                            .padding = 12,
                            .onClick = [&]() {
                                scrollLog = "No scroll events yet";
                                totalScrollY = 0.0f;
                                totalScrollX = 0.0f;
                                scrollCount = 0;
                                zoomLevel = 1.0f;
                                imageOffsetX = 0.0f;
                                imageOffsetY = 0.0f;
                            }
                        }
                    }
                },

                // Instructions
                Text {
                    .value = "💡 Tip: Scroll/wheel events provide deltaX and deltaY for smooth interactions like zoom and pan!",
                    .fontSize = 12,
                    .color = Colors::gray,
                    .horizontalAlignment = HorizontalAlignment::center,
                    .padding = 12
                }
            }
        }
    );

    return app.exec();
}

