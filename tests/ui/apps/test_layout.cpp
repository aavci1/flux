#include <Flux.hpp>
#include <format>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    auto& window = app.createWindow({
        .size = {1200, 900},
        .title = "Test: Layout System"
    });

    window.setRootView(
        VStack {
            .padding = 10,
            .spacing = 10,
            .children = {
                Text {
                    .value = "layout-test-root",
                    .fontSize = 16,
                    .fontWeight = FontWeight::bold,
                    .color = Colors::black
                },

                HStack {
                    .spacing = 10,
                    .expansionBias = 1.0f,
                    .children = {
                        // Section 1: VStack basic
                        VStack {
                            .spacing = 5,
                            .expansionBias = 1.0f,
                            .backgroundColor = Color::hex(0xF0F0F0),
                            .padding = 8,
                            .cornerRadius = 4,
                            .children = {
                                Text { .value = "section-vstack", .fontSize = 11, .color = Colors::black },
                                VStack {
                                    .spacing = 4,
                                    .children = {
                                        Text { .value = "vstack-child-0", .fontSize = 12, .color = Colors::black, .backgroundColor = Color::hex(0xDDEEFF), .padding = 4 },
                                        Text { .value = "vstack-child-1", .fontSize = 12, .color = Colors::black, .backgroundColor = Color::hex(0xDDFFDD), .padding = 4 },
                                        Text { .value = "vstack-child-2", .fontSize = 12, .color = Colors::black, .backgroundColor = Color::hex(0xFFDDDD), .padding = 4 }
                                    }
                                }
                            }
                        },

                        // Section 2: HStack basic
                        VStack {
                            .spacing = 5,
                            .expansionBias = 1.0f,
                            .backgroundColor = Color::hex(0xF0F0F0),
                            .padding = 8,
                            .cornerRadius = 4,
                            .children = {
                                Text { .value = "section-hstack", .fontSize = 11, .color = Colors::black },
                                HStack {
                                    .spacing = 4,
                                    .children = {
                                        Text { .value = "hstack-child-0", .fontSize = 12, .color = Colors::black, .backgroundColor = Color::hex(0xDDEEFF), .padding = 4 },
                                        Text { .value = "hstack-child-1", .fontSize = 12, .color = Colors::black, .backgroundColor = Color::hex(0xDDFFDD), .padding = 4 },
                                        Text { .value = "hstack-child-2", .fontSize = 12, .color = Colors::black, .backgroundColor = Color::hex(0xFFDDDD), .padding = 4 }
                                    }
                                }
                            }
                        },

                        // Section 3: Grid 2x2
                        VStack {
                            .spacing = 5,
                            .expansionBias = 1.0f,
                            .backgroundColor = Color::hex(0xF0F0F0),
                            .padding = 8,
                            .cornerRadius = 4,
                            .children = {
                                Text { .value = "section-grid-2x2", .fontSize = 11, .color = Colors::black },
                                Grid {
                                    .columns = 2,
                                    .rows = 2,
                                    .spacing = 4,
                                    .children = {
                                        Text { .value = "grid-cell-0", .fontSize = 12, .color = Colors::black, .backgroundColor = Color::hex(0xDDEEFF), .padding = 4 },
                                        Text { .value = "grid-cell-1", .fontSize = 12, .color = Colors::black, .backgroundColor = Color::hex(0xDDFFDD), .padding = 4 },
                                        Text { .value = "grid-cell-2", .fontSize = 12, .color = Colors::black, .backgroundColor = Color::hex(0xFFDDDD), .padding = 4 },
                                        Text { .value = "grid-cell-3", .fontSize = 12, .color = Colors::black, .backgroundColor = Color::hex(0xFFFFDD), .padding = 4 }
                                    }
                                }
                            }
                        }
                    }
                },

                HStack {
                    .spacing = 10,
                    .expansionBias = 1.0f,
                    .children = {
                        // Section 4: Justify content
                        VStack {
                            .spacing = 5,
                            .expansionBias = 1.0f,
                            .backgroundColor = Color::hex(0xF0F0F0),
                            .padding = 8,
                            .cornerRadius = 4,
                            .children = {
                                Text { .value = "section-justify", .fontSize = 11, .color = Colors::black },
                                HStack {
                                    .justifyContent = JustifyContent::center,
                                    .backgroundColor = Color::hex(0xE0E0E0),
                                    .padding = 4,
                                    .minHeight = 30,
                                    .children = {
                                        Text { .value = "justify-center-item", .fontSize = 11, .color = Colors::black, .backgroundColor = Color::hex(0xCCDDFF), .padding = 2 }
                                    }
                                },
                                HStack {
                                    .justifyContent = JustifyContent::end,
                                    .backgroundColor = Color::hex(0xE0E0E0),
                                    .padding = 4,
                                    .minHeight = 30,
                                    .children = {
                                        Text { .value = "justify-end-item", .fontSize = 11, .color = Colors::black, .backgroundColor = Color::hex(0xCCDDFF), .padding = 2 }
                                    }
                                },
                                HStack {
                                    .justifyContent = JustifyContent::spaceBetween,
                                    .backgroundColor = Color::hex(0xE0E0E0),
                                    .padding = 4,
                                    .minHeight = 30,
                                    .children = {
                                        Text { .value = "sb-left", .fontSize = 11, .color = Colors::black, .backgroundColor = Color::hex(0xCCDDFF), .padding = 2 },
                                        Text { .value = "sb-right", .fontSize = 11, .color = Colors::black, .backgroundColor = Color::hex(0xFFCCDD), .padding = 2 }
                                    }
                                }
                            }
                        },

                        // Section 5: AlignItems
                        VStack {
                            .spacing = 5,
                            .expansionBias = 1.0f,
                            .backgroundColor = Color::hex(0xF0F0F0),
                            .padding = 8,
                            .cornerRadius = 4,
                            .children = {
                                Text { .value = "section-align", .fontSize = 11, .color = Colors::black },
                                HStack {
                                    .alignItems = AlignItems::start,
                                    .backgroundColor = Color::hex(0xE0E0E0),
                                    .padding = 4,
                                    .minHeight = 60,
                                    .children = {
                                        Text { .value = "align-start-item", .fontSize = 11, .color = Colors::black, .backgroundColor = Color::hex(0xCCDDFF), .padding = 2 }
                                    }
                                },
                                HStack {
                                    .alignItems = AlignItems::center,
                                    .backgroundColor = Color::hex(0xE0E0E0),
                                    .padding = 4,
                                    .minHeight = 60,
                                    .children = {
                                        Text { .value = "align-center-item", .fontSize = 11, .color = Colors::black, .backgroundColor = Color::hex(0xCCDDFF), .padding = 2 }
                                    }
                                },
                                HStack {
                                    .alignItems = AlignItems::end,
                                    .backgroundColor = Color::hex(0xE0E0E0),
                                    .padding = 4,
                                    .minHeight = 60,
                                    .children = {
                                        Text { .value = "align-end-item", .fontSize = 11, .color = Colors::black, .backgroundColor = Color::hex(0xCCDDFF), .padding = 2 }
                                    }
                                }
                            }
                        },

                        // Section 6: ExpansionBias
                        VStack {
                            .spacing = 5,
                            .expansionBias = 1.0f,
                            .backgroundColor = Color::hex(0xF0F0F0),
                            .padding = 8,
                            .cornerRadius = 4,
                            .children = {
                                Text { .value = "section-expansion", .fontSize = 11, .color = Colors::black },
                                VStack {
                                    .spacing = 4,
                                    .minHeight = 200,
                                    .children = {
                                        Text { .value = "expand-1x", .fontSize = 11, .color = Colors::black, .backgroundColor = Color::hex(0xCCDDFF), .padding = 4, .expansionBias = 1.0f },
                                        Text { .value = "expand-2x", .fontSize = 11, .color = Colors::black, .backgroundColor = Color::hex(0xFFCCDD), .padding = 4, .expansionBias = 2.0f },
                                        Text { .value = "expand-none", .fontSize = 11, .color = Colors::black, .backgroundColor = Color::hex(0xDDFFDD), .padding = 4 }
                                    }
                                }
                            }
                        }
                    }
                },

                // Section 7: Grid with colspan
                VStack {
                    .spacing = 5,
                    .backgroundColor = Color::hex(0xF0F0F0),
                    .padding = 8,
                    .cornerRadius = 4,
                    .children = {
                        Text { .value = "section-grid-colspan", .fontSize = 11, .color = Colors::black },
                        Grid {
                            .columns = 3,
                            .rows = 2,
                            .spacing = 4,
                            .children = {
                                Text { .value = "colspan-2-cell", .fontSize = 12, .color = Colors::black, .backgroundColor = Color::hex(0xDDEEFF), .padding = 4, .colspan = 2 },
                                Text { .value = "normal-cell", .fontSize = 12, .color = Colors::black, .backgroundColor = Color::hex(0xDDFFDD), .padding = 4 },
                                Text { .value = "row2-cell-0", .fontSize = 12, .color = Colors::black, .backgroundColor = Color::hex(0xFFDDDD), .padding = 4 },
                                Text { .value = "row2-cell-1", .fontSize = 12, .color = Colors::black, .backgroundColor = Color::hex(0xFFFFDD), .padding = 4 },
                                Text { .value = "row2-cell-2", .fontSize = 12, .color = Colors::black, .backgroundColor = Color::hex(0xDDDDFF), .padding = 4 }
                            }
                        }
                    }
                },

                // Section 8: Nested layout
                VStack {
                    .spacing = 5,
                    .backgroundColor = Color::hex(0xF0F0F0),
                    .padding = 8,
                    .cornerRadius = 4,
                    .children = {
                        Text { .value = "section-nested", .fontSize = 11, .color = Colors::black },
                        HStack {
                            .spacing = 8,
                            .children = {
                                VStack {
                                    .spacing = 4,
                                    .expansionBias = 1.0f,
                                    .backgroundColor = Color::hex(0xE8E8E8),
                                    .padding = 6,
                                    .children = {
                                        Text { .value = "nested-left-top", .fontSize = 11, .color = Colors::black },
                                        Text { .value = "nested-left-bottom", .fontSize = 11, .color = Colors::black }
                                    }
                                },
                                VStack {
                                    .spacing = 4,
                                    .expansionBias = 2.0f,
                                    .backgroundColor = Color::hex(0xE8E8E8),
                                    .padding = 6,
                                    .children = {
                                        Text { .value = "nested-right-top", .fontSize = 11, .color = Colors::black },
                                        HStack {
                                            .spacing = 4,
                                            .children = {
                                                Text { .value = "nested-deep-a", .fontSize = 11, .color = Colors::black, .backgroundColor = Color::hex(0xDDEEFF), .padding = 2 },
                                                Text { .value = "nested-deep-b", .fontSize = 11, .color = Colors::black, .backgroundColor = Color::hex(0xFFDDDD), .padding = 2 }
                                            }
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
