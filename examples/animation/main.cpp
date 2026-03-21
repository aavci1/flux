#include <Flux.hpp>

using namespace flux;

struct AnimationDemo {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;
    FLUX_TRANSFORM_PROPERTIES;

    mutable Property<bool> showPanel = false;
    mutable Property<bool> hovered1 = false;
    mutable Property<bool> hovered2 = false;
    mutable Property<bool> hovered3 = false;
    mutable Property<bool> expanded = false;

    View body() const {
        bool panelVisible = showPanel;
        bool isHov1 = hovered1;
        bool isHov2 = hovered2;
        bool isHov3 = hovered3;
        bool isExpanded = expanded;

        return VStack {
            .padding = EdgeInsets(32),
            .spacing = 24,
            .backgroundColor = Color::hex(0x1a1a2e),
            .expansionBias = 1.0f,
            .children = {

                Text {
                    .value = "Flux Animation Demo",
                    .fontSize = 28,
                    .fontWeight = FontWeight::bold,
                    .color = Colors::white,
                    .horizontalAlignment = HorizontalAlignment::center
                },

                // --- Section 1: Fade + Slide (uses default implicit animation) ---
                Text {
                    .value = "1. Fade + Slide (default implicit animation)",
                    .fontSize = 16,
                    .fontWeight = FontWeight::semibold,
                    .color = Color::hex(0xe94560)
                },

                HStack {
                    .spacing = 16,
                    .children = {
                        Button {
                            .padding = EdgeInsets(10, 20),
                            .backgroundColor = Color::hex(0x0f3460),
                            .cornerRadius = 6,
                            .text = panelVisible ? "Hide Panel" : "Show Panel",
                            .onClick = [this] {
                                showPanel = !static_cast<bool>(showPanel);
                            }
                        },

                        // No .animation needed — the default implicit animation
                        // automatically interpolates opacity, backgroundColor, and offset.
                        VStack {
                            .padding = EdgeInsets(16),
                            .backgroundColor = panelVisible
                                ? Color::hex(0x16213e)
                                : Color::hex(0x16213e).opacity(0.0f),
                            .cornerRadius = 12,
                            .opacity = panelVisible ? 1.0f : 0.0f,
                            .offset = panelVisible ? Point{0, 0} : Point{-20, 0},
                            .children = {
                                Text {
                                    .value = "This panel fades and slides in!",
                                    .fontSize = 14,
                                    .color = Colors::white
                                }
                            }
                        }
                    }
                },

                Divider { .borderColor = Color::hex(0x333366) },

                // --- Section 2: Hover Color Transitions (default animation) ---
                Text {
                    .value = "2. Hover Color Transitions (default implicit animation)",
                    .fontSize = 16,
                    .fontWeight = FontWeight::semibold,
                    .color = Color::hex(0xe94560)
                },

                HStack {
                    .spacing = 12,
                    .children = {
                        // No .animation — color changes animate automatically.
                        VStack {
                            .padding = EdgeInsets(16, 24),
                            .backgroundColor = isHov1
                                ? Color::hex(0xe94560)
                                : Color::hex(0x16213e),
                            .cornerRadius = 8,
                            .onMouseEnter = [this] { hovered1 = true; },
                            .onMouseLeave = [this] { hovered1 = false; },
                            .children = {
                                Text {
                                    .value = "Hover me",
                                    .fontSize = 14,
                                    .fontWeight = FontWeight::medium,
                                    .color = Colors::white
                                }
                            }
                        },
                        VStack {
                            .padding = EdgeInsets(16, 24),
                            .backgroundColor = isHov2
                                ? Color::hex(0x0f3460)
                                : Color::hex(0x16213e),
                            .cornerRadius = 8,
                            .onMouseEnter = [this] { hovered2 = true; },
                            .onMouseLeave = [this] { hovered2 = false; },
                            .children = {
                                Text {
                                    .value = "Hover me too",
                                    .fontSize = 14,
                                    .fontWeight = FontWeight::medium,
                                    .color = Colors::white
                                }
                            }
                        },
                        VStack {
                            .padding = EdgeInsets(16, 24),
                            .backgroundColor = isHov3
                                ? Color::hex(0x533483)
                                : Color::hex(0x16213e),
                            .borderColor = isHov3
                                ? Color::hex(0x533483)
                                : Color::hex(0x333366),
                            .borderWidth = 2,
                            .cornerRadius = 8,
                            .onMouseEnter = [this] { hovered3 = true; },
                            .onMouseLeave = [this] { hovered3 = false; },
                            .children = {
                                Text {
                                    .value = "And me!",
                                    .fontSize = 14,
                                    .fontWeight = FontWeight::medium,
                                    .color = Colors::white
                                }
                            }
                        }
                    }
                },

                Divider { .borderColor = Color::hex(0x333366) },

                // --- Section 3: Spring Animation (explicit override) ---
                Text {
                    .value = "3. Spring Animation (explicit override)",
                    .fontSize = 16,
                    .fontWeight = FontWeight::semibold,
                    .color = Color::hex(0xe94560)
                },

                HStack {
                    .spacing = 16,
                    .children = {
                        Button {
                            .padding = EdgeInsets(10, 20),
                            .backgroundColor = Color::hex(0x0f3460),
                            .cornerRadius = 6,
                            .text = isExpanded ? "Collapse" : "Expand",
                            .onClick = [this] {
                                withAnimation(Animation::Spring(), [this] {
                                    expanded = !static_cast<bool>(expanded);
                                });
                            }
                        },

                        VStack {
                            .padding = EdgeInsets(16),
                            .backgroundColor = Color::hex(0x0f3460),
                            .cornerRadius = 12,
                            .opacity = isExpanded ? 1.0f : 0.3f,
                            .animation = Animation::Spring(),
                            .children = {
                                Text {
                                    .value = isExpanded
                                        ? "Expanded with spring physics!"
                                        : "...",
                                    .fontSize = 14,
                                    .color = Colors::white
                                }
                            }
                        }
                    }
                },

                Spacer {}
            }
        };
    }
};


int main(int argc, char* argv[]) {
    Runtime runtime(argc, argv);

    auto& window = runtime.createWindow({
        .size = {700, 650},
        .title = "Flux Animation Demo"
    });

    window.setRootView(AnimationDemo{});

    return runtime.run();
}
