#include <Flux.hpp>

#include <fstream>
#include <sstream>
#include <string>

using namespace flux;

#ifndef FLUX_SVG_DEMO_TIGER_PATH
#define FLUX_SVG_DEMO_TIGER_PATH "examples/svg-demo/tiger.svg"
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

struct SvgDemoRoot {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> tigerSvg;
    Property<bool> loadFailed = false;
    Property<bool> usedEmbeddedFallback = false;

    static VStack captionBlock(const Theme& theme, std::string caption, View content) {
        return VStack {
            .spacing = 8,
            .alignItems = AlignItems::stretch,
            .children = {
                Text {
                    .value = std::move(caption),
                    .fontSize = Typography::subheadline,
                    .fontWeight = FontWeight::medium,
                    .color = theme.secondaryForeground,
                    .horizontalAlignment = HorizontalAlignment::leading,
                    .lineHeightMultiplier = Typography::lineHeightTight,
                },
                std::move(content),
            }
        };
    }

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
                        .value = "Could not load tiger SVG. Build with CMake so FLUX_SVG_DEMO_TIGER_PATH "
                                 "points at examples/svg-demo/tiger.svg, or place that file next to the executable.",
                        .color = theme.foreground,
                        .horizontalAlignment = HorizontalAlignment::leading,
                    },
                }
            };
        }

        const Color frameBg = theme.surfaceElevated;
        const Color frameBorder = theme.borderStrong;

        auto framedSvg = [&](SVG svg) {
            return VStack {
                .backgroundColor = frameBg,
                .borderColor = frameBorder,
                .borderWidth = 1,
                .cornerRadius = 8,
                .padding = 0,
                .children = { View(std::move(svg)) },
            };
        };

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
                            .spacing = 28,
                            .alignItems = AlignItems::stretch,
                            .children = {
                                Text {
                                    .value = static_cast<bool>(usedEmbeddedFallback)
                                        ? std::string("SVG component — embedded sample")
                                        : std::string("SVG component — Ghostscript Tiger"),
                                    .fontSize = 28.0f,
                                    .fontWeight = FontWeight::bold,
                                    .color = theme.foreground,
                                    .horizontalAlignment = HorizontalAlignment::leading,
                                },
                                Text {
                                    .value = static_cast<bool>(usedEmbeddedFallback)
                                        ? std::string(
                                              "Using a minimal inline SVG because tiger.svg was not loaded. Build with "
                                              "CMake (FLUX_SVG_DEMO_TIGER_PATH) or run from a directory where that path "
                                              "exists to use the full Ghostscript Tiger asset.")
                                        : std::string(
                                              "Vector asset from Wikimedia Commons (public domain). The view scales "
                                              "content to the layout box; use preserveAspectRatio to letterbox or stretch."),
                                    .fontSize = Typography::body,
                                    .color = theme.secondaryForeground,
                                    .horizontalAlignment = HorizontalAlignment::leading,
                                    .lineHeightMultiplier = Typography::lineHeightBody,
                                },

                                captionBlock(theme,
                                    "Uniform scale, centered (preserveAspectRatio = true) — fits inside a wide box; "
                                    "empty bands appear where aspect ratios differ.",
                                    framedSvg(SVG {
                                        .content = svg,
                                        .preserveAspectRatio = true,
                                        .minHeight = 100,
                                        .minWidth = 360,
                                    })
                                ),

                                captionBlock(theme,
                                    "Independent X/Y scale (preserveAspectRatio = false) — fills the same box; "
                                    "the artwork is stretched to the viewport.",
                                    framedSvg(SVG {
                                        .content = svg,
                                        .preserveAspectRatio = false,
                                        .minHeight = 100,
                                        .minWidth = 360,
                                    })
                                ),

                                captionBlock(theme,
                                    "Side-by-side comparison (same layout size)",
                                    HStack {
                                        .spacing = 16,
                                        .alignItems = AlignItems::stretch,
                                        .children = {
                                            framedSvg(SVG {
                                                .content = svg,
                                                .preserveAspectRatio = true,
                                                .minHeight = 140,
                                                .minWidth = 160,
                                            }),
                                            framedSvg(SVG {
                                                .content = svg,
                                                .preserveAspectRatio = false,
                                                .minHeight = 140,
                                                .minWidth = 160,
                                            }),
                                        }
                                    }
                                ),

                                captionBlock(theme,
                                    "Explicit size property — preferred layout size is fixed (e.g. toolbar icons) "
                                    "instead of the SVG's intrinsic dimensions.",
                                    framedSvg(SVG {
                                        .content = svg,
                                        .preserveAspectRatio = true,
                                        .size = Size{96.0f, 96.0f},
                                    })
                                ),

                                captionBlock(theme,
                                    "Intrinsic preferred size — omit a positive width/height in size to use the "
                                    "document's natural dimensions (intrinsic size for this document).",
                                    framedSvg(SVG {
                                        .content = svg,
                                        .preserveAspectRatio = true,
                                        .minHeight = 200,
                                        .minWidth = 200,
                                    })
                                ),

                                captionBlock(theme,
                                    "Padding inside the SVG view — insets the vector inside the frame.",
                                    framedSvg(SVG {
                                        .content = svg,
                                        .preserveAspectRatio = true,
                                        .padding = EdgeInsets(16),
                                        .minHeight = 160,
                                        .minWidth = 280,
                                    })
                                ),
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
        .tigerSvg = tigerSvg,
        .loadFailed = false,
        .usedEmbeddedFallback = usedEmbedded,
    });

    return app.exec();
}
