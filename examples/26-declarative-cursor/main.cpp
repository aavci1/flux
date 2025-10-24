#include <Flux.hpp>
#include <iostream>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    Window window({
        .size = {500, 400},
        .title = "Declarative Cursor Demo"
    });

    window.setRootView(
        VStack {
            .padding = EdgeInsets{32},
            .spacing = 24,
            .backgroundColor = Color::hex(0xF5F5F5),
            .children = {
                Text {
                    .value = "Declarative Cursor Demo",
                    .fontSize = 28,
                    .fontWeight = FontWeight::bold,
                    .color = Color::hex(0x333333)
                },
                
                Text {
                    .value = "Hover over the colored boxes to see different cursors",
                    .fontSize = 14,
                    .color = Color::hex(0x666666)
                },

                // Text cursor example
                VStack {
                    .padding = EdgeInsets{20},
                    .backgroundColor = Colors::white,
                    .borderColor = Color::hex(0xDDDDDD),
                    .borderWidth = 1,
                    .cornerRadius = 8,
                    .cursor = CursorType::Text,  // Declarative cursor!
                    .children = {
                        Text {
                            .value = "Text Cursor Area",
                            .fontSize = 16,
                            .fontWeight = FontWeight::bold,
                            .color = Color::hex(0x333333)
                        },
                        Text {
                            .value = "Hover here to see the text (I-beam) cursor",
                            .fontSize = 12,
                            .color = Color::hex(0x666666)
                        }
                    }
                },

                // Grid of different cursor examples
                Grid {
                    .columns = 3,
                    .rows = 2,
                    .spacing = 12,
                    .expansionBias = 1.0f,
                    .children = {
                        VStack {
                            .padding = EdgeInsets{16},
                            .backgroundColor = Color::hex(0xFFEBEE),
                            .cornerRadius = 8,
                            .cursor = CursorType::NotAllowed,
                            .children = {
                                Text {
                                    .value = "Not Allowed",
                                    .fontSize = 12,
                                    .fontWeight = FontWeight::bold,
                                    .color = Color::hex(0xC62828)
                                }
                            }
                        },
                        VStack {
                            .padding = EdgeInsets{16},
                            .backgroundColor = Color::hex(0xE3F2FD),
                            .cornerRadius = 8,
                            .cursor = CursorType::Help,
                            .children = {
                                Text {
                                    .value = "Help",
                                    .fontSize = 12,
                                    .fontWeight = FontWeight::bold,
                                    .color = Color::hex(0x1565C0)
                                }
                            }
                        },
                        VStack {
                            .padding = EdgeInsets{16},
                            .backgroundColor = Color::hex(0xE8F5E9),
                            .cornerRadius = 8,
                            .cursor = CursorType::Move,
                            .children = {
                                Text {
                                    .value = "Move",
                                    .fontSize = 12,
                                    .fontWeight = FontWeight::bold,
                                    .color = Color::hex(0x2E7D32)
                                }
                            }
                        },
                        VStack {
                            .padding = EdgeInsets{16},
                            .backgroundColor = Color::hex(0xFFF3E0),
                            .cornerRadius = 8,
                            .cursor = CursorType::Crosshair,
                            .children = {
                                Text {
                                    .value = "Crosshair",
                                    .fontSize = 12,
                                    .fontWeight = FontWeight::bold,
                                    .color = Color::hex(0xE65100)
                                }
                            }
                        },
                        VStack {
                            .padding = EdgeInsets{16},
                            .backgroundColor = Color::hex(0xF3E5F5),
                            .cornerRadius = 8,
                            .cursor = CursorType::Wait,
                            .children = {
                                Text {
                                    .value = "Wait",
                                    .fontSize = 12,
                                    .fontWeight = FontWeight::bold,
                                    .color = Color::hex(0x6A1B9A)
                                }
                            }
                        },
                        VStack {
                            .padding = EdgeInsets{16},
                            .backgroundColor = Color::hex(0xE0F2F1),
                            .cornerRadius = 8,
                            .cursor = CursorType::Grab,
                            .children = {
                                Text {
                                    .value = "Grab",
                                    .fontSize = 12,
                                    .fontWeight = FontWeight::bold,
                                    .color = Color::hex(0x00695C)
                                }
                            }
                        }
                    }
                }
            }
        }
    );

    return app.exec();
}
