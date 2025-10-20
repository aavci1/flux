#include <Flux.hpp>
#include <cmath>
#include <ranges>
#include <thread>
#include <chrono>
#include <sstream>
#include <iomanip>

using namespace flux;

// TODO: add basic items like circle, rectangle etc. ?

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

struct MediaPlayerAlbumCover {
    FLUX_VIEW_PROPERTIES;

    View body() const {
        return Text {
            .backgroundColor = Color::hex(0x0000ff),
            .cornerRadius = 8
        };
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        return Size{100, 100};
    }
};

struct MediaPlayerButton {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> value = "X";

    View body() const {
        return Text {
            .value = this->value,
        };
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        auto radius = fmin(bounds.width, bounds.height) / 2;

        Path circlePath;
        circlePath.arc(bounds.center(), radius, 0, 2 * M_PI, false);
        circlePath.close();

        ctx.setFillStyle(FillStyle::solid(Color::hex(0x4A90E2).opacity(0.3)));
        ctx.setStrokeStyle(StrokeStyle::solid(Color::hex(0x4A90E2).opacity(0.7)));
        ctx.drawPath(circlePath);
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        return Size{48, 48};
    }
};

struct MediaPlayerTrack {
    FLUX_VIEW_PROPERTIES;

    Property<int> trackCurrentTimeInSeconds = 0;
    Property<int> trackDurationInSeconds = 335;

    View body() const {
        return HStack {
            .spacing = 8,
            .children = {
                Text {
                    .value = std::format("{:02}:{:02}", this->trackCurrentTimeInSeconds / 60, this->trackCurrentTimeInSeconds % 60),
                    .color = Colors::darkGray,
                    .fontSize = 14
                },
                Slider {
                    .expansionBias = 1,
                    .value = static_cast<float>(this->trackCurrentTimeInSeconds) / static_cast<float>(this->trackDurationInSeconds)
                },
                Text {
                    .value = std::format("{:02}:{:02}", this->trackDurationInSeconds / 60, this->trackDurationInSeconds % 60),
                    .color = Colors::darkGray,
                    .fontSize = 14
                }
            }
        };
    }
};

struct MediaPlayer {
    FLUX_VIEW_PROPERTIES;

    Property<float> spacing = 16;

    Property<std::string> trackTitle = std::string("Born of God");
    Property<std::string> trackArtist = std::string("@Daft Punk");
    Property<std::string> trackAlbum = std::string("Random Access Memories");
    Property<std::string> trackAlbumCover = std::string("album_cover.jpg");
    Property<int> trackCurrentTimeInSeconds = 0;
    Property<int> trackDurationInSeconds = 335;

    View body() const {
        return VStack {
            .backgroundColor = this->backgroundColor,
            .borderWidth = this->borderWidth,
            .borderColor = this->borderColor,
            .cornerRadius = this->cornerRadius,
            .compressionBias = this->compressionBias,
            .expansionBias = this->expansionBias,
            .padding = this->padding,
            .spacing = this->spacing,
            .children = {
                HStack {
                    // .backgroundColor = Color::hex(0x00ffff),
                    .expansionBias = 2,
                    .spacing = 16,
                    .alignItems = AlignItems::center,
                    .children = {
                        MediaPlayerAlbumCover {
                            // .backgroundColor = Color::hex(0x0000ff),
                            .cornerRadius = 8,
                            .expansionBias = 0
                        },
                        VStack {
                            // .backgroundColor = Color::hex(0xffff00),
                            .expansionBias = 2,
                            .spacing = 8,
                            .justifyContent = JustifyContent::center,
                            .children = {
                                Text {
                                    // .backgroundColor = Color::hex(0xff0000),
                                    .value = "Born of God",
                                    .fontSize = 24,
                                    .fontWeight = FontWeight::bold,
                                    .horizontalAlignment = HorizontalAlignment::leading
                                },
                                Text {
                                    // .backgroundColor = Color::hex(0x00ff00),
                                    .value = "@Daft Punk",
                                    .color = Color::hex(0x7f8c8d),
                                    .fontSize = 16,
                                    .fontWeight = FontWeight::bold,
                                    .horizontalAlignment = HorizontalAlignment::leading
                                }
                            }
                        }
                    }
                },
                HStack {
                    .expansionBias = 1,
                    .justifyContent = JustifyContent::spaceBetween,
                    .children = {
                        MediaPlayerButton {
                            .cornerRadius = 8,
                            .value = "|<"
                        },
                        Spacer {},
                        MediaPlayerButton {
                            .cornerRadius = 8,
                            .value = "<<"
                        },
                        Spacer {},
                        MediaPlayerButton {
                            .cornerRadius = 8,
                            .value = "||"
                        },
                        Spacer {},
                        MediaPlayerButton {
                            .cornerRadius = 8,
                            .value = ">>"
                        },
                        Spacer {},
                        MediaPlayerButton {
                            .cornerRadius = 8,
                            .value = ">|"
                        }
                    }
                },
                MediaPlayerTrack {
                    .expansionBias = 1,
                    .trackCurrentTimeInSeconds = trackCurrentTimeInSeconds,
                    .trackDurationInSeconds = trackDurationInSeconds
                }
            }
        };
    }
};

void timeout(std::function<void()> func, int interval) {
    std::thread([func, interval]()
    {
      while (true)
      {
        auto x = std::chrono::steady_clock::now() + std::chrono::milliseconds(interval);
        func();
        std::this_thread::sleep_until(x);
      }
    }).detach();
}

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    Window window({
        .size = {1600, 1200},
        .title = "Automotive Dashboard"
    });

    State date = std::string("");
    State time = std::string("");
    State name = std::string("Abdurrahman Avcı");
    State chargeLevel = 80;

    State trackTitle = std::string("Born of God");
    State trackArtist = std::string("@Daft Punk");
    State trackAlbum = std::string("Random Access Memories");
    State trackAlbumCover = std::string("album_cover.jpg");
    State trackCurrentTimeInSeconds = 0;
    State trackDurationInSeconds = 335;

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
                                    .value = time,
                                    .fontSize = 24,
                                    .fontWeight = FontWeight::bold,
                                    .horizontalAlignment = HorizontalAlignment::leading
                                },
                                Spacer {},
                                Text {
                                    .value = name,
                                    .fontSize = 24,
                                    .fontWeight = FontWeight::bold,
                                    .horizontalAlignment = HorizontalAlignment::trailing
                                }
                            }
                        },
                        HStack {
                            .children = {
                                Text {
                                    .value = date,
                                    .fontSize = 16,
                                    .color = Color::hex(0x7f8c8d),
                                    .horizontalAlignment = HorizontalAlignment::leading
                                },
                                Spacer {
                                },
                                 HStack {
                                    .compressionBias = 0,
                                    .expansionBias = 0,
                                    .spacing = 8,
                                    .alignItems = AlignItems::center,
                                    .justifyContent = JustifyContent::end,
                                    .children = {
                                        Text {
                                            .value = std::format("{}%", chargeLevel),
                                            .fontSize = 16,
                                            .color = Color::hex(0x7f8c8d)
                                        },
                                        BatteryIcon {
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
                Grid {
                    .columns = 4,
                    .rows = 3,
                    .compressionBias = 0,
                    .expansionBias = 1,
                    .spacing = 24,
                    .children = {
                        MediaPlayer {
                            .expansionBias = 1,
                            .backgroundColor = Color::hex(0xffffff),
                            .borderWidth = 1,
                            .borderColor = Colors::lightGray,
                            .cornerRadius = 16,
                            .padding = 24,
                            .trackTitle = trackTitle,
                            .trackArtist = trackArtist,
                            .trackAlbum = trackAlbum,
                            .trackAlbumCover = trackAlbumCover,
                            .trackCurrentTimeInSeconds = trackCurrentTimeInSeconds,
                            .trackDurationInSeconds = trackDurationInSeconds
                        },
                        VStack {
                            .expansionBias = 2,
                            .backgroundColor = Color::hex(0xffffff),
                            .borderWidth = 1,
                            .borderColor = Colors::lightGray,
                            .rowspan = 3,
                            .colspan = 2,
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
                        Text {
                            .expansionBias = 2,
                            .backgroundColor = Color::hex(0xffffff),
                            .borderWidth = 1,
                            .borderColor = Colors::lightGray,
                            .cornerRadius = 16,
                            .rowspan = 2,
                            .value = "Vehicle Status"
                        },
                        Grid {
                            .columns = 2,
                            .rows = 3,
                            .rowspan = 2,
                            .expansionBias = 2,
                            .backgroundColor = Color::hex(0xffffff),
                            .borderWidth = 1,
                            .borderColor = Colors::lightGray,
                            .cornerRadius = 16,
                            .padding = 24,
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
                                },
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
                                },
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
    );

    timeout([&date, &time, &trackCurrentTimeInSeconds, &trackDurationInSeconds]() {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);

        std::stringstream ds;
        ds << std::put_time(std::localtime(&in_time_t), "%A | %B %d, %Y");
        date = ds.str();

        std::stringstream ts;
        ts << std::put_time(std::localtime(&in_time_t), "%H:%M:%S");
        time = ts.str();

        trackCurrentTimeInSeconds++;
        if (trackCurrentTimeInSeconds >= trackDurationInSeconds) {
            trackCurrentTimeInSeconds = 0;
        }
    }, 1000);

    return app.exec();
}
