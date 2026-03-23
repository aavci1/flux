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

struct SvgDemoRoot {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> tigerSvg;
    Property<bool> loadFailed = false;

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
                    .padding = 20,
                    .children = {
                        VStack {
                            .spacing = 28,
                            .alignItems = AlignItems::stretch,
                            .children = {
                                Text {
                                    .value = "SVG component — Ghostscript Tiger",
                                    .fontSize = 28.0f,
                                    .fontWeight = FontWeight::bold,
                                    .color = theme.foreground,
                                    .horizontalAlignment = HorizontalAlignment::leading,
                                },
                                Text {
                                    .value = "Vector asset from Wikimedia Commons (public domain). The view scales "
                                             "content to the layout box; use preserveAspectRatio to letterbox or stretch.",
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
                                    "document's natural dimensions (900×900 for this file).",
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

    std::string tiger = loadTextFile(FLUX_SVG_DEMO_TIGER_PATH);
    const bool ok = !tiger.empty();

    auto& window = app.createWindow({
        .size = {880, 900},
        .title = "Flux — SVG demo",
    });

    window.setRootView(SvgDemoRoot {
        .tigerSvg = std::move(tiger),
        .loadFailed = !ok,
    });

    return app.exec();
}
