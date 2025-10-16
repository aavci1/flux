#include <Flux.hpp>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    Window window({
        .size = {900, 700},
        .title = "JustifyContent Demo"
    });

    window.setRootView(
        VStack {
            .padding = 20,
            .spacing = 20,
            .backgroundColor = Colors::lightGray,
            .children = {
                // Title
                Text {
                    .value = "JustifyContent Demo",
                    .fontSize = 24,
                    .fontWeight = FontWeight::bold,
                    .horizontalAlignment = HorizontalAlignment::center
                },

                // HStack Examples
                VStack {
                    .expansionBias = 1.0f,
                    .spacing = 15,
                    .children = {
                        VStack {
                            .spacing = 5,
                            .children = {
                                Text {
                                    .value = "HStack - start",
                                    .fontSize = 12,
                                    .color = Colors::darkGray
                                },
                                HStack {
                                    .justifyContent = JustifyContent::start,
                                    .spacing = 5,
                                    .padding = 10,
                                    .backgroundColor = Colors::lightGray,
                                    .borderColor = Colors::gray,
                                    .borderWidth = 1,
                                    .cornerRadius = 4,
                                    .children = {
                                        Text { .value = "A", .padding = 10, .backgroundColor = Colors::blue, .color = Colors::white, .cornerRadius = 4 },
                                        Text { .value = "B", .padding = 10, .backgroundColor = Colors::blue, .color = Colors::white, .cornerRadius = 4 },
                                        Text { .value = "C", .padding = 10, .backgroundColor = Colors::green, .color = Colors::white, .cornerRadius = 4 }
                                    }
                                }
                            }
                        },
                        VStack {
                            .spacing = 5,
                            .children = {
                                Text {
                                    .value = "HStack - center",
                                    .fontSize = 12,
                                    .color = Colors::darkGray
                                },
                                HStack {
                                    .justifyContent = JustifyContent::center,
                                    .spacing = 5,
                                    .padding = 10,
                                    .backgroundColor = Colors::lightGray,
                                    .borderColor = Colors::gray,
                                    .borderWidth = 1,
                                    .cornerRadius = 4,
                                    .children = {
                                        Text { .value = "A", .padding = 10, .backgroundColor = Colors::blue, .color = Colors::white, .cornerRadius = 4 },
                                        Text { .value = "B", .padding = 10, .backgroundColor = Colors::blue, .color = Colors::white, .cornerRadius = 4 },
                                        Text { .value = "C", .padding = 10, .backgroundColor = Colors::green, .color = Colors::white, .cornerRadius = 4 }
                                    }
                                }
                            }
                        },
                        VStack {
                            .spacing = 5,
                            .children = {
                                Text {
                                    .value = "HStack - end",
                                    .fontSize = 12,
                                    .color = Colors::darkGray
                                },
                                HStack {
                                    .justifyContent = JustifyContent::end,
                                    .spacing = 5,
                                    .padding = 10,
                                    .backgroundColor = Colors::lightGray,
                                    .borderColor = Colors::gray,
                                    .borderWidth = 1,
                                    .cornerRadius = 4,
                                    .children = {
                                        Text { .value = "A", .padding = 10, .backgroundColor = Colors::blue, .color = Colors::white, .cornerRadius = 4 },
                                        Text { .value = "B", .padding = 10, .backgroundColor = Colors::blue, .color = Colors::white, .cornerRadius = 4 },
                                        Text { .value = "C", .padding = 10, .backgroundColor = Colors::green, .color = Colors::white, .cornerRadius = 4 }
                                    }
                                }
                            }
                        },
                        VStack {
                            .spacing = 5,
                            .children = {
                                Text {
                                    .value = "HStack - spaceBetween",
                                    .fontSize = 12,
                                    .color = Colors::darkGray
                                },
                                HStack {
                                    .justifyContent = JustifyContent::spaceBetween,
                                    .spacing = 5,
                                    .padding = 10,
                                    .backgroundColor = Colors::lightGray,
                                    .borderColor = Colors::gray,
                                    .borderWidth = 1,
                                    .cornerRadius = 4,
                                    .children = {
                                        Text { .value = "A", .padding = 10, .backgroundColor = Colors::blue, .color = Colors::white, .cornerRadius = 4 },
                                        Text { .value = "B", .padding = 10, .backgroundColor = Colors::blue, .color = Colors::white, .cornerRadius = 4 },
                                        Text { .value = "C", .padding = 10, .backgroundColor = Colors::green, .color = Colors::white, .cornerRadius = 4 }
                                    }
                                }
                            }
                        },
                        VStack {
                            .spacing = 5,
                            .children = {
                                Text {
                                    .value = "HStack - spaceAround",
                                    .fontSize = 12,
                                    .color = Colors::darkGray
                                },
                                HStack {
                                    .justifyContent = JustifyContent::spaceAround,
                                    .spacing = 5,
                                    .padding = 10,
                                    .backgroundColor = Colors::lightGray,
                                    .borderColor = Colors::gray,
                                    .borderWidth = 1,
                                    .cornerRadius = 4,
                                    .children = {
                                        Text { .value = "A", .padding = 10, .backgroundColor = Colors::blue, .color = Colors::white, .cornerRadius = 4 },
                                        Text { .value = "B", .padding = 10, .backgroundColor = Colors::blue, .color = Colors::white, .cornerRadius = 4 },
                                        Text { .value = "C", .padding = 10, .backgroundColor = Colors::green, .color = Colors::white, .cornerRadius = 4 }
                                    }
                                }
                            }
                        },
                        VStack {
                            .spacing = 5,
                            .children = {
                                Text {
                                    .value = "HStack - spaceEvenly",
                                    .fontSize = 12,
                                    .color = Colors::darkGray
                                },
                                HStack {
                                    .justifyContent = JustifyContent::spaceEvenly,
                                    .spacing = 5,
                                    .padding = 10,
                                    .backgroundColor = Colors::lightGray,
                                    .borderColor = Colors::gray,
                                    .borderWidth = 1,
                                    .cornerRadius = 4,
                                    .children = {
                                        Text { .value = "A", .padding = 10, .backgroundColor = Colors::blue, .color = Colors::white, .cornerRadius = 4 },
                                        Text { .value = "B", .padding = 10, .backgroundColor = Colors::blue, .color = Colors::white, .cornerRadius = 4 },
                                        Text { .value = "C", .padding = 10, .backgroundColor = Colors::green, .color = Colors::white, .cornerRadius = 4 }
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
