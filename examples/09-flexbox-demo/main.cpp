#include <Flux.hpp>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    Window window({
        .size = {1000, 800},
        .title = "Flexbox Layout Demo"
    });

    window.setRootView(
        VStack {
            .padding = 30,
            .spacing = 30,
            .children = {
                // Title
                Text {
                    .value = "Flux Flexbox Layout Demo",
                    .fontSize = 32,
                    .fontWeight = FontWeight::bold,
                    .horizontalAlignment = HorizontalAlignment::center,
                    .color = Colors::black
                },

                // Demo 1: Equal Expansion (expansionBias = 1.0)
                VStack {
                    .spacing = 15,
                    .children = {
                        Text {
                            .value = "Demo 1: Equal Expansion (expansionBias = 1.0)",
                            .fontSize = 20,
                            .fontWeight = FontWeight::medium,
                            .color = Colors::black
                        },
                        HStack {
                            .spacing = 10,
                            .children = {
                                Text {
                                    .value = "A",
                                    .expansionBias = 1.0f,
                                    .backgroundColor = Colors::red,
                                    .padding = 20,
                                    .cornerRadius = 8,
                                    .color = Colors::white,
                                    .fontSize = 18,
                                    .fontWeight = FontWeight::bold,
                                    .horizontalAlignment = HorizontalAlignment::center
                                },
                                Text {
                                    .value = "B",
                                    .expansionBias = 1.0f,
                                    .backgroundColor = Colors::green,
                                    .padding = 20,
                                    .cornerRadius = 8,
                                    .color = Colors::white,
                                    .fontSize = 18,
                                    .fontWeight = FontWeight::bold,
                                    .horizontalAlignment = HorizontalAlignment::center
                                },
                                Text {
                                    .value = "C",
                                    .expansionBias = 1.0f,
                                    .backgroundColor = Colors::blue,
                                    .padding = 20,
                                    .cornerRadius = 8,
                                    .color = Colors::white,
                                    .fontSize = 18,
                                    .fontWeight = FontWeight::bold,
                                    .horizontalAlignment = HorizontalAlignment::center
                                }
                            }
                        }
                    }
                },

                // Demo 2: Different Expansion Ratios
                VStack {
                    .spacing = 15,
                    .children = {
                        Text {
                            .value = "Demo 2: Different Expansion Ratios (1x, 2x, 1x)",
                            .fontSize = 20,
                            .fontWeight = FontWeight::medium,
                            .color = Colors::black
                        },
                        HStack {
                            .spacing = 10,
                            .children = {
                                Text {
                                    .value = "1x",
                                    .expansionBias = 1.0f,
                                    .backgroundColor = Colors::red,
                                    .padding = 20,
                                    .cornerRadius = 8,
                                    .color = Colors::white,
                                    .fontSize = 18,
                                    .fontWeight = FontWeight::bold,
                                    .horizontalAlignment = HorizontalAlignment::center
                                },
                                Text {
                                    .value = "2x",
                                    .expansionBias = 2.0f,
                                    .backgroundColor = Colors::green,
                                    .padding = 20,
                                    .cornerRadius = 8,
                                    .color = Colors::white,
                                    .fontSize = 18,
                                    .fontWeight = FontWeight::bold,
                                    .horizontalAlignment = HorizontalAlignment::center
                                },
                                Text {
                                    .value = "1x",
                                    .expansionBias = 1.0f,
                                    .backgroundColor = Colors::blue,
                                    .padding = 20,
                                    .cornerRadius = 8,
                                    .color = Colors::white,
                                    .fontSize = 18,
                                    .fontWeight = FontWeight::bold,
                                    .horizontalAlignment = HorizontalAlignment::center
                                }
                            }
                        }
                    }
                },

                // Demo 3: Mixed Expansion (Fixed + Flexible)
                VStack {
                    .spacing = 15,
                    .children = {
                        Text {
                            .value = "Demo 3: Mixed Expansion (Fixed + Flexible)",
                            .fontSize = 20,
                            .fontWeight = FontWeight::medium,
                            .color = Colors::black
                        },
                        HStack {
                            .spacing = 10,
                            .children = {
                                Text {
                                    .value = "Fixed",
                                    .expansionBias = 0.0f,
                                    .backgroundColor = Colors::red,
                                    .padding = 20,
                                    .cornerRadius = 8,
                                    .color = Colors::white,
                                    .fontSize = 18,
                                    .fontWeight = FontWeight::bold,
                                    .horizontalAlignment = HorizontalAlignment::center
                                },
                                Text {
                                    .value = "Flexible",
                                    .expansionBias = 1.0f,
                                    .backgroundColor = Colors::green,
                                    .padding = 20,
                                    .cornerRadius = 8,
                                    .color = Colors::white,
                                    .fontSize = 18,
                                    .fontWeight = FontWeight::bold,
                                    .horizontalAlignment = HorizontalAlignment::center
                                },
                                Text {
                                    .value = "Fixed",
                                    .expansionBias = 0.0f,
                                    .backgroundColor = Colors::blue,
                                    .padding = 20,
                                    .cornerRadius = 8,
                                    .color = Colors::white,
                                    .fontSize = 18,
                                    .fontWeight = FontWeight::bold,
                                    .horizontalAlignment = HorizontalAlignment::center
                                }
                            }
                        }
                    }
                },

                // Demo 4: Compression Test
                VStack {
                    .spacing = 15,
                    .children = {
                        Text {
                            .value = "Demo 4: Compression Test (compressionBias)",
                            .fontSize = 20,
                            .fontWeight = FontWeight::medium,
                            .color = Colors::black
                        },
                        HStack {
                            .spacing = 10,
                            .children = {
                                Text {
                                    .value = "Long text that should compress",
                                    .compressionBias = 1.0f,
                                    .backgroundColor = Colors::red,
                                    .padding = 20,
                                    .cornerRadius = 8,
                                    .color = Colors::white,
                                    .fontSize = 16,
                                    .fontWeight = FontWeight::medium,
                                    .horizontalAlignment = HorizontalAlignment::center
                                },
                                Text {
                                    .value = "Short",
                                    .compressionBias = 0.0f,
                                    .backgroundColor = Colors::green,
                                    .padding = 20,
                                    .cornerRadius = 8,
                                    .color = Colors::white,
                                    .fontSize = 16,
                                    .fontWeight = FontWeight::medium,
                                    .horizontalAlignment = HorizontalAlignment::center
                                },
                                Text {
                                    .value = "Medium length text",
                                    .compressionBias = 0.0f,
                                    .backgroundColor = Colors::blue,
                                    .padding = 20,
                                    .cornerRadius = 8,
                                    .color = Colors::white,
                                    .fontSize = 16,
                                    .fontWeight = FontWeight::medium,
                                    .horizontalAlignment = HorizontalAlignment::center
                                }
                            }
                        }
                    }
                },

                // Demo 5: Vertical Stack with Expansion
                VStack {
                    .spacing = 15,
                    .children = {
                        Text {
                            .value = "Demo 5: Vertical Stack Expansion",
                            .fontSize = 20,
                            .fontWeight = FontWeight::medium,
                            .color = Colors::black
                        },
                        HStack {
                            .spacing = 20,
                            .children = {
                                VStack {
                                    .expansionBias = 1.0f,
                                    .spacing = 10,
                                    .children = {
                                        Text {
                                            .value = "VStack 1",
                                            .backgroundColor = Colors::red,
                                            .padding = 15,
                                            .cornerRadius = 8,
                                            .color = Colors::white,
                                            .fontSize = 16,
                                            .fontWeight = FontWeight::bold,
                                            .horizontalAlignment = HorizontalAlignment::center
                                        },
                                        Text {
                                            .value = "Equal height",
                                            .backgroundColor = Colors::red,
                                            .padding = 15,
                                            .cornerRadius = 8,
                                            .color = Colors::white,
                                            .fontSize = 16,
                                            .fontWeight = FontWeight::bold,
                                            .horizontalAlignment = HorizontalAlignment::center
                                        }
                                    }
                                },
                                VStack {
                                    .expansionBias = 1.0f,
                                    .spacing = 10,
                                    .children = {
                                        Text {
                                            .value = "VStack 2",
                                            .backgroundColor = Colors::green,
                                            .padding = 15,
                                            .cornerRadius = 8,
                                            .color = Colors::white,
                                            .fontSize = 16,
                                            .fontWeight = FontWeight::bold,
                                            .horizontalAlignment = HorizontalAlignment::center
                                        },
                                        Text {
                                            .value = "Equal height",
                                            .backgroundColor = Colors::green,
                                            .padding = 15,
                                            .cornerRadius = 8,
                                            .color = Colors::white,
                                            .fontSize = 16,
                                            .fontWeight = FontWeight::bold,
                                            .horizontalAlignment = HorizontalAlignment::center
                                        },
                                        Text {
                                            .value = "With more content",
                                            .backgroundColor = Colors::green,
                                            .padding = 15,
                                            .cornerRadius = 8,
                                            .color = Colors::white,
                                            .fontSize = 16,
                                            .fontWeight = FontWeight::bold,
                                            .horizontalAlignment = HorizontalAlignment::center
                                        }
                                    }
                                },
                                VStack {
                                    .expansionBias = 1.0f,
                                    .spacing = 10,
                                    .children = {
                                        Text {
                                            .value = "VStack 3",
                                            .backgroundColor = Colors::blue,
                                            .padding = 15,
                                            .cornerRadius = 8,
                                            .color = Colors::white,
                                            .fontSize = 16,
                                            .fontWeight = FontWeight::bold,
                                            .horizontalAlignment = HorizontalAlignment::center
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
