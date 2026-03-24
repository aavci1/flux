#include <Flux.hpp>

#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

using namespace flux;

#ifndef FLUX_SVG_DEMO_TIGER_PATH
#define FLUX_SVG_DEMO_TIGER_PATH "../examples/svg-demo/tiger.svg"
#endif

static std::string loadTextFile(const char* path) {
    std::ifstream file(path);
    if (!file) {
        return {};
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Small inline document used when `tiger.svg` cannot be read (wrong cwd, missing file). Kept simple for
// debugging; fills are light-on-dark so they read clearly on the demo's elevated frames.
static constexpr const char* kEmbeddedSimpleSvg = R"(<svg width="120px" height="120px" viewBox="0 0 120 120" fill="none" xmlns="http://www.w3.org/2000/svg"><g id="SVGRepo_bgCarrier" stroke-width="0"></g><g id="SVGRepo_tracerCarrier" stroke-linecap="round" stroke-linejoin="round"></g><g id="SVGRepo_iconCarrier"> <rect width="120" height="120" fill="#EFF1F3"></rect> <path fill-rule="evenodd" clip-rule="evenodd" d="M33.2503 38.4816C33.2603 37.0472 34.4199 35.8864 35.8543 35.875H83.1463C84.5848 35.875 85.7503 37.0431 85.7503 38.4816V80.5184C85.7403 81.9528 84.5807 83.1136 83.1463 83.125H35.8543C34.4158 83.1236 33.2503 81.957 33.2503 80.5184V38.4816ZM80.5006 41.1251H38.5006V77.8751L62.8921 53.4783C63.9172 52.4536 65.5788 52.4536 66.6039 53.4783L80.5006 67.4013V41.1251ZM43.75 51.6249C43.75 54.5244 46.1005 56.8749 49 56.8749C51.8995 56.8749 54.25 54.5244 54.25 51.6249C54.25 48.7254 51.8995 46.3749 49 46.3749C46.1005 46.3749 43.75 48.7254 43.75 51.6249Z" fill="#000000"></path> </g></svg>)";

namespace {

constexpr int kAspectPresetCount = 5;

static const char* kAspectHelp[kAspectPresetCount] = {
    "Uniform scale (preserveAspectRatio): one scale factor, centered in the layout box. Different aspect ratios "
    "produce letterboxing.",
    "Independent X/Y scale (preserveAspectRatio off): fills the layout box; the artwork may look stretched.",
    "Explicit size: preferred layout is fixed at 96×96 (typical toolbar-style icon).",
    "Intrinsic preferred size: uses the document’s natural dimensions inside at least a 200×200 box.",
    "Padding: EdgeInsets(16) insets the vector inside the frame (minimum box 280×160).",
};

SVG makePreviewSvg(const std::string& content, int preset) {
    switch (preset) {
        case 0:
            return SVG {
                .content = content,
                .preserveAspectRatio = true,
                .minWidth = 360.0f,
                .minHeight = 100.0f,
            };
        case 1:
            return SVG {
                .content = content,
                .preserveAspectRatio = false,
                .minWidth = 360.0f,
                .minHeight = 100.0f,
            };
        case 2:
            return SVG {
                .content = content,
                .preserveAspectRatio = true,
                .size = Size{96.0f, 96.0f},
            };
        case 3:
            return SVG {
                .content = content,
                .preserveAspectRatio = true,
                .minWidth = 200.0f,
                .minHeight = 200.0f,
            };
        case 4:
            return SVG {
                .content = content,
                .preserveAspectRatio = true,
                .padding = EdgeInsets(16),
                .minWidth = 280.0f,
                .minHeight = 160.0f,
            };
        default:
            return SVG {
                .content = content,
                .preserveAspectRatio = true,
                .minWidth = 360.0f,
                .minHeight = 100.0f,
            };
    }
}

} // namespace

struct SvgDemoRoot {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> tigerSvg;
    Property<bool> loadFailed = false;
    Property<bool> usedEmbeddedFallback = false;
    /// Shared with SelectInput; 0..kAspectPresetCount-1
    mutable Property<int> aspectPreset = Property<int>::shared(0);

    View body() const {
        const Theme& theme = Application::instance().theme();
        const std::string svg = static_cast<std::string>(tigerSvg);
        const bool failed = static_cast<bool>(loadFailed);
        const bool dark = (theme == Theme::dark());

        if (failed || svg.empty()) {
            return VStack {
                .backgroundColor = theme.background,
                .padding = 24,
                .spacing = 12,
                .children = {
                    Text {
                        .value = "Could not load SVG. Build with CMake so FLUX_SVG_DEMO_TIGER_PATH points at your "
                                 "asset (e.g. examples/svg-demo/tiger.svg), or place that file where the path resolves.",
                        .color = theme.foreground,
                        .horizontalAlignment = HorizontalAlignment::leading,
                    },
                }
            };
        }

        const Color frameBg = theme.surfaceElevated;
        const Color frameBorder = theme.borderStrong;

        auto framedSvg = [&](SVG s) {
            return VStack {
                .backgroundColor = frameBg,
                .borderColor = frameBorder,
                .borderWidth = 1,
                .cornerRadius = 8,
                .padding = 0,
                .children = { View(std::move(s)) },
            };
        };

        const int preset = std::clamp(static_cast<int>(aspectPreset), 0, kAspectPresetCount - 1);

        return VStack {
            .backgroundColor = theme.background,
            .spacing = 0,
            .children = {
                VStack {
                    .padding = EdgeInsets(12, 16),
                    .backgroundColor = theme.surfaceElevated,
                    .children = {
                        Toggle {
                            .isOn = dark,
                            .label = std::string("Dark mode"),
                            .labelColor = theme.foreground,
                            .onChange = []() {
                                const Theme& current = Application::instance().theme();
                                Application::instance().setTheme(
                                    current == Theme::light() ? Theme::dark() : Theme::light());
                            },
                        },
                    }
                },
                ScrollArea {
                    .expansionBias = 1.0f,
                    .padding = 20,
                    .children = {
                        VStack {
                            .spacing = 24,
                            .alignItems = AlignItems::stretch,
                            .children = {
                                Text {
                                    .value = static_cast<bool>(usedEmbeddedFallback)
                                        ? std::string("SVG component — embedded sample")
                                        : std::string("SVG component — demo asset"),
                                    .fontSize = 28.0f,
                                    .fontWeight = FontWeight::bold,
                                    .color = theme.foreground,
                                    .horizontalAlignment = HorizontalAlignment::leading,
                                },
                                Text {
                                    .value = static_cast<bool>(usedEmbeddedFallback)
                                        ? std::string(
                                              "Using a minimal inline SVG because the file was not loaded. Build with "
                                              "CMake (FLUX_SVG_DEMO_TIGER_PATH) or run from a directory where that path exists.")
                                        : std::string(
                                              "Pick a layout preset to see how scaling, fixed size, intrinsic size, and padding affect the same document."),
                                    .fontSize = Typography::body,
                                    .color = theme.secondaryForeground,
                                    .horizontalAlignment = HorizontalAlignment::leading,
                                    .lineHeightMultiplier = Typography::lineHeightBody,
                                },

                                VStack {
                                    .spacing = 10,
                                    .alignItems = AlignItems::stretch,
                                    .children = {
                                        Text {
                                            .value = std::string("Layout & aspect"),
                                            .fontSize = Typography::subheadline,
                                            .fontWeight = FontWeight::medium,
                                            .color = theme.secondaryForeground,
                                            .horizontalAlignment = HorizontalAlignment::leading,
                                        },
                                        SelectInput {
                                            .selectedIndex = aspectPreset,
                                            .options =
                                                std::vector<std::string> {
                                                    "Letterbox (uniform scale)",
                                                    "Stretch (independent X/Y)",
                                                    "Fixed 96×96",
                                                    "Intrinsic (min 200×200)",
                                                    "Padding 16px in frame",
                                                },
                                            .selectWidth = 420.0f,
                                            .bgColor = theme.inputBackground,
                                            .dropdownBgColor = theme.surfaceElevated,
                                            .borderColor_ = theme.borderStrong,
                                            .textColor_ = theme.foreground,
                                            .mutedColor = theme.secondaryForeground,
                                        },
                                        Text {
                                            .value = std::string(kAspectHelp[preset]),
                                            .fontSize = Typography::body,
                                            .color = theme.secondaryForeground,
                                            .horizontalAlignment = HorizontalAlignment::leading,
                                            .lineHeightMultiplier = Typography::lineHeightBody,
                                        },
                                    }
                                },

                                framedSvg(makePreviewSvg(svg, preset)),
                            }
                        }
                    }
                },
            }
        };
    }
};

int main(int argc, char* argv[]) {
    Application app(argc, argv);
    app.setTheme(Theme::dark());

    auto& window = app.createWindow({
        .size = {880, 900},
        .title = "Flux — SVG demo",
    });

    std::string tigerSvg = loadTextFile(FLUX_SVG_DEMO_TIGER_PATH);
    const bool usedEmbedded = tigerSvg.empty();
    if (usedEmbedded) {
        tigerSvg = kEmbeddedSimpleSvg;
    }

    window.setRootView(SvgDemoRoot {
        .tigerSvg = std::move(tigerSvg),
        .loadFailed = false,
        .usedEmbeddedFallback = usedEmbedded,
    });

    return app.exec();
}
