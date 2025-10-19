#include <Flux.hpp>
using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    Window window({
        .size = {1200, 1600},
        .title = "Colors and Theming - Flux Demo"
    });

    window.setRootView(
        VStack {
            .padding = 40,
            .spacing = 40,
            .backgroundColor = Colors::lightGray,
            .children = {
                // Title
                Text {
                    .value = "Flux Color System",
                    .fontSize = 36,
                    .fontWeight = FontWeight::bold,
                    .color = Colors::black,
                    .horizontalAlignment = HorizontalAlignment::center
                },

                Text {
                    .value = "Clean color palette showcase",
                    .fontSize = 18,
                    .color = Colors::gray,
                    .horizontalAlignment = HorizontalAlignment::center
                },

                // Section 1: Base Colors
                VStack {
                    .spacing = 20,
                    .children = {
                        Text {
                            .value = "Base Colors",
                            .fontSize = 24,
                            .fontWeight = FontWeight::bold,
                            .color = Colors::black,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },

                        HStack {
                            .spacing = 16,
                            .children = {
                                VStack {
                                    .backgroundColor = Colors::white,
                                    .borderColor = Colors::gray,
                                    .borderWidth = 1,
                                    .cornerRadius = 12,
                                    .padding = 40,
                                },
                                VStack {
                                    .backgroundColor = Colors::black,
                                    .cornerRadius = 12,
                                    .padding = 40,
                                },
                                VStack {
                                    .backgroundColor = Colors::gray,
                                    .cornerRadius = 12,
                                    .padding = 40,
                                },
                                VStack {
                                    .backgroundColor = Colors::darkGray,
                                    .cornerRadius = 12,
                                    .padding = 40,
                                },
                                VStack {
                                    .backgroundColor = Colors::lightGray,
                                    .cornerRadius = 12,
                                    .padding = 40,
                                }
                            }
                        }
                    }
                },

                // Section 2: Semantic Colors
                VStack {
                    .spacing = 20,
                    .children = {
                        Text {
                            .value = "Semantic Colors",
                            .fontSize = 24,
                            .fontWeight = FontWeight::bold,
                            .color = Colors::black,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },

                        HStack {
                            .spacing = 16,
                            .children = {
                                VStack {
                                    .backgroundColor = Colors::blue,
                                    .cornerRadius = 12,
                                    .padding = 40,
                                },
                                VStack {
                                    .backgroundColor = Colors::gray,
                                    .cornerRadius = 12,
                                    .padding = 40,
                                },
                                VStack {
                                    .backgroundColor = Colors::blue,
                                    .cornerRadius = 12,
                                    .padding = 40,
                                },
                                VStack {
                                    .backgroundColor = Colors::black,
                                    .cornerRadius = 12,
                                    .padding = 40,
                                },
                                VStack {
                                    .backgroundColor = Colors::lightGray,
                                    .borderColor = Colors::gray,
                                    .borderWidth = 1,
                                    .cornerRadius = 12,
                                    .padding = 40,
                                }
                            }
                        }
                    }
                },

                // Section 3: Color Palette
                VStack {
                    .spacing = 20,
                    .children = {
                        Text {
                            .value = "Color Palette",
                            .fontSize = 24,
                            .fontWeight = FontWeight::bold,
                            .color = Colors::black,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },

                        HStack {
                            .spacing = 16,
                            .children = {
                                VStack {
                                    .backgroundColor = Colors::red,
                                    .cornerRadius = 12,
                                    .padding = 40,
                                },
                                VStack {
                                    .backgroundColor = Colors::blue,
                                    .cornerRadius = 12,
                                    .padding = 40,
                                },
                                VStack {
                                    .backgroundColor = Colors::green,
                                    .cornerRadius = 12,
                                    .padding = 40,
                                },
                                VStack {
                                    .backgroundColor = Colors::yellow,
                                    .cornerRadius = 12,
                                    .padding = 40,
                                }
                            }
                        }
                    }
                },

                // Section 4: Status Colors
                VStack {
                    .spacing = 20,
                    .children = {
                        Text {
                            .value = "Status Colors",
                            .fontSize = 24,
                            .fontWeight = FontWeight::bold,
                            .color = Colors::black,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },

                        HStack {
                            .spacing = 16,
                            .children = {
                                VStack {
                                    .backgroundColor = Colors::green,
                                    .cornerRadius = 12,
                                    .padding = 40,
                                },
                                VStack {
                                    .backgroundColor = Colors::yellow,
                                    .cornerRadius = 12,
                                    .padding = 40,
                                },
                                VStack {
                                    .backgroundColor = Colors::red,
                                    .cornerRadius = 12,
                                    .padding = 40,
                                }
                            }
                        }
                    }
                },

                // Section 5: Custom Colors
                VStack {
                    .spacing = 20,
                    .children = {
                        Text {
                            .value = "Custom Colors",
                            .fontSize = 24,
                            .fontWeight = FontWeight::bold,
                            .color = Colors::black,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },

                        HStack {
                            .spacing = 16,
                            .children = {
                                VStack {
                                    .backgroundColor = Color::hex(0xFF6B6B),
                                    .cornerRadius = 12,
                                    .padding = 40,
                                },
                                VStack {
                                    .backgroundColor = Color::hex(0x4ECDC4),
                                    .cornerRadius = 12,
                                    .padding = 40,
                                },
                                VStack {
                                    .backgroundColor = Color::hex(0xFFBE0B),
                                    .cornerRadius = 12,
                                    .padding = 40,
                                },
                                VStack {
                                    .backgroundColor = Color::rgb(138, 43, 226),
                                    .cornerRadius = 12,
                                    .padding = 40,
                                }
                            }
                        }
                    }
                },

                // Section 6: Opacity Variations
                VStack {
                    .spacing = 20,
                    .children = {
                        Text {
                            .value = "Opacity Variations",
                            .fontSize = 24,
                            .fontWeight = FontWeight::bold,
                            .color = Colors::black,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },

                        HStack {
                            .spacing = 16,
                            .children = {
                                VStack {
                                    .backgroundColor = Color::hex(0x2196F3).opacity(1.0),
                                    .cornerRadius = 8,
                                    .padding = 40
                                },
                                VStack {
                                    .backgroundColor = Color::hex(0x2196F3).opacity(0.8),
                                    .cornerRadius = 8,
                                    .padding = 40
                                },
                                VStack {
                                    .backgroundColor = Color::hex(0x2196F3).opacity(0.6),
                                    .cornerRadius = 8,
                                    .padding = 40
                                },
                                VStack {
                                    .backgroundColor = Color::hex(0x2196F3).opacity(0.4),
                                    .cornerRadius = 8,
                                    .padding = 40
                                },
                                VStack {
                                    .backgroundColor = Color::hex(0x2196F3).opacity(0.2),
                                    .cornerRadius = 8,
                                    .padding = 40
                                }
                            }
                        }
                    }
                },

                // Section 7: Darken Variations
                VStack {
                    .spacing = 20,
                    .children = {
                        Text {
                            .value = "Darken Variations",
                            .fontSize = 24,
                            .fontWeight = FontWeight::bold,
                            .color = Colors::black,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },

                        HStack {
                            .spacing = 16,
                            .children = {
                                VStack {
                                    .backgroundColor = Colors::green,
                                    .cornerRadius = 12,
                                    .padding = 40,
                                },
                                VStack {
                                    .backgroundColor = Color::hex(0x4CAF50).darken(0.1),
                                    .cornerRadius = 12,
                                    .padding = 40,
                                },
                                VStack {
                                    .backgroundColor = Color::hex(0x4CAF50).darken(0.3),
                                    .cornerRadius = 12,
                                    .padding = 40,
                                },
                                VStack {
                                    .backgroundColor = Color::hex(0x4CAF50).darken(0.5),
                                    .cornerRadius = 12,
                                    .padding = 40,
                                }
                            }
                        }
                    }
                },

                // Section 8: Practical Examples
                VStack {
                    .spacing = 20,
                    .children = {
                        Text {
                            .value = "Practical Examples",
                            .fontSize = 24,
                            .fontWeight = FontWeight::bold,
                            .color = Colors::black,
                            .horizontalAlignment = HorizontalAlignment::leading
                        },

                        HStack {
                            .spacing = 20,
                            .children = {
                                // Success Alert
                                VStack {
                                    .expansionBias = 1.0f,
                                    .backgroundColor = Color::hex(0x4CAF50).opacity(0.1),
                                    .borderColor = Colors::green,
                                    .borderWidth = 2,
                                    .cornerRadius = 12,
                                    .padding = 20,
                                    .spacing = 8,
                                    .children = {
                                        Text {
                                            .value = "✓ Success",
                                            .fontSize = 16,
                                            .fontWeight = FontWeight::bold,
                                            .color = Colors::green,
                                            .horizontalAlignment = HorizontalAlignment::leading
                                        },
                                        Text {
                                            .value = "Operation completed successfully",
                                            .fontSize = 14,
                                            .color = Colors::darkGray,
                                            .horizontalAlignment = HorizontalAlignment::leading
                                        }
                                    }
                                },

                                // Warning Alert
                                VStack {
                                    .expansionBias = 1.0f,
                                    .backgroundColor = Color::hex(0xFFA726).opacity(0.1),
                                    .borderColor = Colors::yellow,
                                    .borderWidth = 2,
                                    .cornerRadius = 12,
                                    .padding = 20,
                                    .spacing = 8,
                                    .children = {
                                        Text {
                                            .value = "⚠ Warning",
                                            .fontSize = 16,
                                            .fontWeight = FontWeight::bold,
                                            .color = Colors::yellow,
                                            .horizontalAlignment = HorizontalAlignment::leading
                                        },
                                        Text {
                                            .value = "Please review before proceeding",
                                            .fontSize = 14,
                                            .color = Colors::darkGray,
                                            .horizontalAlignment = HorizontalAlignment::leading
                                        }
                                    }
                                },

                                // Error Alert
                                VStack {
                                    .expansionBias = 1.0f,
                                    .backgroundColor = Color::hex(0xF44336).opacity(0.1),
                                    .borderColor = Colors::red,
                                    .borderWidth = 2,
                                    .cornerRadius = 12,
                                    .padding = 20,
                                    .spacing = 8,
                                    .children = {
                                        Text {
                                            .value = "✗ Error",
                                            .fontSize = 16,
                                            .fontWeight = FontWeight::bold,
                                            .color = Colors::red,
                                            .horizontalAlignment = HorizontalAlignment::leading
                                        },
                                        Text {
                                            .value = "Something went wrong",
                                            .fontSize = 14,
                                            .color = Colors::darkGray,
                                            .horizontalAlignment = HorizontalAlignment::leading
                                        }
                                    }
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