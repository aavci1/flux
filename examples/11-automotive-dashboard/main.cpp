#include <Flux.hpp>
#include <cmath>
#include <ranges>

using namespace flux;

// Battery Icon Component
struct BatterySegment {
    FLUX_VIEW_PROPERTIES;

    Size preferredSize(TextMeasurement& textMeasurer) const {
        return Size{1, 1};
    }
};

struct BatteryTerminal {
    FLUX_VIEW_PROPERTIES;

    Size preferredSize(TextMeasurement& textMeasurer) const {
        return Size{2, 1};
    }
};

struct BatteryIcon {
    FLUX_VIEW_PROPERTIES;

    Property<int> chargeLevel = 60; // 0-100
    Property<Size> size = Size{50, 18};
    Property<Color> batteryColor = Color::hex(0xFEFDFF);
    Property<Color> chargeColor = Color::hex(0x37B564);
    Property<Color> batteryBorderColor = Color::hex(0xD5D5D5);

    View body() const {
        int chargeLevel = this->chargeLevel;
        Color batteryColor = this->batteryColor;
        Color chargeColor = this->chargeColor;

        return HStack {
            .children = {
                HStack {
                    .borderWidth = 1,
                    .borderColor = Colors::gray,
                    .cornerRadius = 2,
                    .expansionBias = 1,
                    .spacing = 1,
                    .padding = 2,
                    .children = [&, chargeLevel, batteryColor, chargeColor]() {
                        std::vector<View> segments;
                        segments.reserve(5);

                        for (int i = 0; i < 5; i++) {
                            segments.push_back(
                                BatterySegment {
                                    .expansionBias = 1,
                                    .backgroundColor = (chargeLevel / 20) > i ? chargeColor : batteryColor
                                }
                            );
                        }

                        return segments;
                    }
                },
                VStack {
                    .children = {
                        Spacer {},
                        BatteryTerminal {
                            .compressionBias = 0,
                            .expansionBias = 1,
                            .backgroundColor = Colors::gray,
                            .margin = EdgeInsets{ 4, 0, 4, 0 }
                        },
                        Spacer {},
                    }
                }
            }
        };
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        return size;
    }
};

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    Window window({
        .size = {1200, 800},
        .title = "Automotive Dashboard"
    });

    State chargeLevel = 100;

    window.setRootView(
        VStack {
            .backgroundColor = Color::hex(0xf8f9fa),
            .spacing = 24,
            .padding = 32,
            .children = {
                // Top Bar
                VStack {
                    .compressionBias = 0,
                    .expansionBias = 0,
                    .spacing = 8,
                    .children = {
                        HStack {
                            .children = {
                                Text {
                                    .value = "9:45 am",
                                    .fontSize = 24,
                                    .fontWeight = FontWeight::bold
                                },
                                Spacer {},
                                Text {
                                    .value = "Mayad Ahmed",
                                    .fontSize = 24,
                                    .fontWeight = FontWeight::bold
                                }
                            }
                        },
                        HStack {
                            .children = {
                                Text {
                                    // .backgroundColor = Colors::green,
                                    .value = "Sunday | February 19, 2023",
                                    .fontSize = 16,
                                    .color = Color::hex(0x7f8c8d)
                                },
                                Spacer {
                                },
                                 HStack {
                                    .compressionBias = 0,
                                    .expansionBias = 0,
                                    .spacing = 8,
                                    .alignItems = AlignItems::center,
                                    .children = {
                                        Text {
                                            // .backgroundColor = Colors::red,
                                            .value = [&chargeLevel]() { return std::format("{}%", chargeLevel); },
                                            .fontSize = 16,
                                            .color = Color::hex(0x7f8c8d)
                                        },
                                        BatteryIcon {
                                            // .backgroundColor = Colors::blue,
                                            .margin = 8,
                                            .compressionBias = 0,
                                            .expansionBias = 1,
                                            .chargeLevel = chargeLevel,
                                            .size = Size{32, 12},
                                        }
                                     }
                                 }
                            }
                        }
                    }
                },
                HStack {
                    .compressionBias = 0,
                    .expansionBias = 1,
                    .spacing = 24,
                    .children = {
                        VStack {
                            .expansionBias = 1,
                            .spacing = 24,
                            .children = {
                                Text {
                                    .expansionBias = 1,
                                    .backgroundColor = Color::hex(0xffffff),
                                    .borderWidth = 1,
                                    .borderColor = Colors::lightGray,
                                    .cornerRadius = 16,
                                    .padding = 24,
                                    .value = "Media Player"
                                },
                                VStack {
                                    .expansionBias = 2,
                                    .backgroundColor = Color::hex(0xffffff),
                                    .borderWidth = 1,
                                    .borderColor = Colors::lightGray,
                                    .cornerRadius = 16,
                                    .padding = 24,
                                    .spacing = 24,
                                    .children = {
                                        HStack {
                                            .expansionBias = 1,
                                            .spacing = 24,
                                            .children = {
                                                Text {
                                                    .backgroundColor = Color::hex(0xf8f9fa),
                                                    .borderWidth = 1,
                                                    .borderColor = Colors::lightGray,
                                                    .cornerRadius = 8,
                                                    .expansionBias = 1,
                                                    .value = "Phone"
                                                },
                                                Text {
                                                    .backgroundColor = Color::hex(0xf8f9fa),
                                                    .borderWidth = 1,
                                                    .borderColor = Colors::lightGray,
                                                    .cornerRadius = 8,
                                                    .expansionBias = 1,
                                                    .value = "Music"
                                                }
                                            }
                                        },
                                        HStack {
                                            .expansionBias = 1,
                                            .spacing = 24,
                                            .children = {
                                                Text {
                                                    .backgroundColor = Color::hex(0xf8f9fa),
                                                    .borderWidth = 1,
                                                    .borderColor = Colors::lightGray,
                                                    .cornerRadius = 8,
                                                    .expansionBias = 1,
                                                    .value = "Bluetooth"
                                                },
                                                Text {
                                                    .backgroundColor = Color::hex(0xf8f9fa),
                                                    .borderWidth = 1,
                                                    .borderColor = Colors::lightGray,
                                                    .cornerRadius = 8,
                                                    .expansionBias = 1,
                                                    .value = "Settings"
                                                }
                                            }
                                        },
                                        HStack {
                                            .expansionBias = 1,
                                            .spacing = 24,
                                            .children = {
                                                Text {
                                                    .backgroundColor = Color::hex(0xf8f9fa),
                                                    .borderWidth = 1,
                                                    .borderColor = Colors::lightGray,
                                                    .cornerRadius = 8,
                                                    .expansionBias = 1,
                                                    .value = "Wind"
                                                },
                                                Text {
                                                    .backgroundColor = Color::hex(0x297AFE),
                                                    .color = Colors::white,
                                                    .cornerRadius = 8,
                                                    .expansionBias = 1,
                                                    .value = "Maps"
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        },
                        VStack {
                            .expansionBias = 2,
                            .backgroundColor = Color::hex(0xffffff),
                            .borderWidth = 1,
                            .borderColor = Colors::lightGray,
                            .cornerRadius = 16,
                            .padding = 24,
                            .spacing = 24,
                            .children = {
                                Text {
                                    .backgroundColor = Colors::lightGray,
                                    .expansionBias = 1,
                                    .cornerRadius = 8,
                                    .value = "Map"
                                },
                                Text {
                                    .expansionBias = 0,
                                    .horizontalAlignment = HorizontalAlignment::leading,
                                    .value = "On the way to the Hotel Grand Palace"
                                },
                                HStack {
                                    .alignItems = AlignItems::center,
                                    .children = {
                                        VStack {
                                            .expansionBias = 1,
                                            .spacing = 4,
                                            .children = {
                                                Text {
                                                    .value = "45 minutes",
                                                    .fontSize = 32,
                                                    .fontWeight = FontWeight::bold,
                                                    .horizontalAlignment = HorizontalAlignment::leading
                                                },
                                                Text {
                                                    .value = "Estimated time for 32km",
                                                    .fontSize = 16,
                                                    .horizontalAlignment = HorizontalAlignment::leading
                                                }
                                            }
                                        },
                                        Text {
                                            .compressionBias = 0,
                                            .backgroundColor = Color::hex(0x297AFE),
                                            .color = Colors::white,
                                            .cornerRadius = 4,
                                            .padding = 16,
                                            .value = "A"
                                        },
                                        Text {
                                            .compressionBias = 0,
                                            .cornerRadius = 4,
                                            .padding = 16,
                                            .value = "B"
                                        },
                                        Text {
                                            .compressionBias = 0,
                                            .cornerRadius = 4,
                                            .padding = 16,
                                            .value = "C"
                                        }
                                    }
                                }
                            }
                        },
                        VStack {
                            .expansionBias = 1,
                            .spacing = 24,
                            .children = {
                                Text {
                                    .expansionBias = 2,
                                    .backgroundColor = Color::hex(0xffffff),
                                    .borderWidth = 1,
                                    .borderColor = Colors::lightGray,
                                    .cornerRadius = 16,
                                    .value = "Vehicle Status"
                                },
                                Text {
                                    .expansionBias = 1,
                                    .backgroundColor = Color::hex(0xffffff),
                                    .borderWidth = 1,
                                    .borderColor = Colors::lightGray,
                                    .cornerRadius = 16,
                                    .value = "Climate Control"
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
