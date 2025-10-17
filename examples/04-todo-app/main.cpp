#include <Flux.hpp>
#include <vector>
#include <string>
#include <random>
#include <algorithm>
#include <iostream>
#include <cmath>

using namespace flux;

// ProgressIcon - Circular progress indicator component
struct CalendarIcon {
    FLUX_VIEW_PROPERTIES;

    View body() {
        return SVG {
            .content = R"(<svg id="Layer_1" data-name="Layer 1" xmlns="http://www.w3.org/2000/svg" viewBox="0 0 122.88 121"><defs><style>.cls-1{fill:#ef4136;}.cls-1,.cls-3,.cls-4,.cls-6{fill-rule:evenodd;}.cls-2{fill:gray;}.cls-3{fill:#fff;}.cls-4{fill:#c72b20;}.cls-5,.cls-6{fill:#1a1a1a;}</style></defs><title>calendar-color</title><path class="cls-1" d="M11.52,6.67h99.84a11.57,11.57,0,0,1,11.52,11.52V44.94H0V18.19A11.56,11.56,0,0,1,11.52,6.67Zm24.79,9.75A9.31,9.31,0,1,1,27,25.73a9.31,9.31,0,0,1,9.31-9.31Zm49.79,0a9.31,9.31,0,1,1-9.31,9.31,9.31,9.31,0,0,1,9.31-9.31Z"/><path class="cls-2" d="M111.36,121H11.52A11.57,11.57,0,0,1,0,109.48V40H122.88v69.46A11.56,11.56,0,0,1,111.36,121Z"/><path class="cls-3" d="M12.75,117.31h97.38a9.1,9.1,0,0,0,9.06-9.06V40H3.69v68.23a9.09,9.09,0,0,0,9.06,9.06Z"/><path class="cls-4" d="M86.1,14.63a11.11,11.11,0,1,1-7.85,3.26l.11-.1a11.06,11.06,0,0,1,7.74-3.16Zm0,1.79a9.31,9.31,0,1,1-9.31,9.31,9.31,9.31,0,0,1,9.31-9.31Z"/><path class="cls-4" d="M36.31,14.63a11.11,11.11,0,1,1-7.85,3.26l.11-.1a11.08,11.08,0,0,1,7.74-3.16Zm0,1.79A9.31,9.31,0,1,1,27,25.73a9.31,9.31,0,0,1,9.31-9.31Z"/><path class="cls-5" d="M80.54,4.56C80.54,2,83,0,86.1,0s5.56,2,5.56,4.56V25.77c0,2.51-2.48,4.56-5.56,4.56s-5.56-2-5.56-4.56V4.56Z"/><path class="cls-5" d="M30.75,4.56C30.75,2,33.24,0,36.31,0s5.56,2,5.56,4.56V25.77c0,2.51-2.48,4.56-5.56,4.56s-5.56-2-5.56-4.56V4.56Z"/><path class="cls-6" d="M22,85.62H36a1.79,1.79,0,0,1,1.79,1.79v11.7A1.8,1.8,0,0,1,36,100.9H22a1.8,1.8,0,0,1-1.8-1.79V87.41A1.8,1.8,0,0,1,22,85.62Z"/><path class="cls-6" d="M54.58,85.62H68.64a1.79,1.79,0,0,1,1.79,1.79v11.7a1.8,1.8,0,0,1-1.79,1.79H54.58a1.8,1.8,0,0,1-1.79-1.79V87.41a1.8,1.8,0,0,1,1.79-1.79Z"/><path class="cls-6" d="M86.87,85.62h14.06a1.8,1.8,0,0,1,1.79,1.79v11.7a1.8,1.8,0,0,1-1.79,1.79H86.87a1.8,1.8,0,0,1-1.79-1.79V87.41a1.79,1.79,0,0,1,1.79-1.79Z"/><path class="cls-6" d="M22,56.42H36a1.8,1.8,0,0,1,1.79,1.8V69.91A1.8,1.8,0,0,1,36,71.7H22a1.8,1.8,0,0,1-1.8-1.79V58.22a1.81,1.81,0,0,1,1.8-1.8Z"/><path class="cls-6" d="M54.58,56.42H68.64a1.8,1.8,0,0,1,1.79,1.8V69.91a1.8,1.8,0,0,1-1.79,1.79H54.58a1.79,1.79,0,0,1-1.79-1.79V58.22a1.8,1.8,0,0,1,1.79-1.8Z"/><path class="cls-6" d="M86.87,56.42h14.06a1.8,1.8,0,0,1,1.79,1.8V69.91a1.8,1.8,0,0,1-1.79,1.79H86.87a1.79,1.79,0,0,1-1.79-1.79V58.22a1.8,1.8,0,0,1,1.79-1.8Z"/></svg>)"
        };
    }
    Size preferredSize(TextMeasurement& textMeasurer) const {
        return {64.0f, 64.0f};
    }
};

struct ProgressIcon {
    FLUX_VIEW_PROPERTIES;

    Property<int> progress = 0; // 0-100
    Property<Color> progressColor = Color::hex(0x1F6ACD); // Bright blue progress color
    Property<Color> trackColor = Color::hex(0xE0E1E2); // Light gray track color
    Property<Color> iconBackgroundColor = Color::hex(0x353466); // Dark blue background
    Property<Color> textColor = Color(1, 1, 1, 1); // White text color
    Property<float> size = 64.0f; // Icon size
    Property<float> strokeWidth = 1.0f; // Progress ring thickness

    // render() - Draw circular progress indicator
    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        float iconSize = size;
        float centerX = bounds.x + bounds.width / 2;
        float centerY = bounds.y + bounds.height / 2;
        float radius = (iconSize - strokeWidth) / 2;

        // Draw shadow
        Shadow shadow{0, 2, 4};
        Rect shadowRect = {centerX - iconSize/2, centerY - iconSize/2, iconSize, iconSize};
        ctx.drawShadow(shadowRect, iconSize/2, shadow);

        // Draw background circle
        Color bgColor = iconBackgroundColor;
        ctx.drawCircle({centerX, centerY}, iconSize/2, bgColor);

        // Draw progress track (background ring)
        float trackRadius = radius;
        Color trackCol = trackColor;
        StrokeStyle trackStyle = {trackCol, strokeWidth};
        ctx.drawArc({centerX, centerY}, trackRadius, 0, 2 * M_PI, trackStyle);

        // Draw progress arc
        int progressVal = progress.get() >= 0 ? progress.get() : 100;
        std::string textVal = progress.get() >= 0 ? std::format("{}%", progressVal) : " 📅";

        float progressAngle = (progressVal / 100.0f) * 2 * M_PI; // Convert to radians
        float progressRadius = radius + strokeWidth;
        Color progressCol = progressColor;

        // Draw progress arc starting from top (-π/2) and going clockwise
        float startAngle = -M_PI / 2; // Start at top
        float endAngle = startAngle + progressAngle;
        ctx.drawArc({centerX, centerY}, progressRadius, startAngle, endAngle, {progressCol, strokeWidth});

        Size textSize = ctx.measureText(textVal, 18, FontWeight::medium);
        Point textPos = {centerX, centerY};
        Color txtColor = textColor;
        ctx.drawText(textVal, textPos, 18, txtColor, FontWeight::medium, HorizontalAlignment::center, VerticalAlignment::center);
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        float iconSize = size;
        EdgeInsets paddingVal = padding;
        return {iconSize + paddingVal.horizontal(), iconSize + paddingVal.vertical()};
    }
};

// Separator - Horizontal line component (like HTML <hr>)
struct Separator {
    FLUX_VIEW_PROPERTIES;

    Property<Color> color = Color::hex(0xE0E1E2); // Light gray color
    Property<float> thickness = 1.0f; // Line thickness
    Property<float> lineMargin = 0.0f; // Margin on left and right sides of the line

    // render() - Draw horizontal line
    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        float marginVal = lineMargin;
        float thicknessVal = thickness;
        Color lineColor = color;

        // Calculate line position (centered vertically)
        float lineY = bounds.y + bounds.height / 2;
        float lineStartX = bounds.x + marginVal;
        float lineEndX = bounds.x + bounds.width - marginVal;

        // Draw the horizontal line
        ctx.drawLine({lineStartX, lineY}, {lineEndX, lineY}, {lineColor, thicknessVal});
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        EdgeInsets paddingVal = padding;
        float thicknessVal = thickness;
        float marginVal = lineMargin;
        return {thicknessVal + paddingVal.horizontal() + marginVal * 2,
                thicknessVal + paddingVal.vertical()};
    }
};

enum class TaskStatus {
    todo,
    ongoing,
    review
};

struct TodoItem {
    int id;
    std::string title;
    std::string subtitle;
    std::string month;
    int day;
    int progress; // 0-100 for progress percentage, or -1 for calendar icon
    TaskStatus status;

    bool operator==(const TodoItem& other) const {
        return id == other.id;
    }
};

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    // Sample tasks matching the design
    State<std::vector<TodoItem>> todos = std::vector<TodoItem>{
        {1, "Creating Website", "Creating Website", "Jun", 14, 86, TaskStatus::ongoing},
        {2, "Meeting with mas client", "10.00 AM - 12.00 AM", "Jun", 19, -1, TaskStatus::todo},
        {3, "User Testing", "25 user in one day", "Jun", 22, 57, TaskStatus::review},
        {4, "Gambling Project", "11.00 PM till drop", "Jul", 12, -1, TaskStatus::todo},
        {5, "Meeting with Investor", "10.00 AM - 12.00 AM", "Jul", 17, -1, TaskStatus::ongoing}
    };

    Window window({
        .size = {600, 1200},
        .title = "Task Manager"
    });

    window.setRootView(
        VStack {
            .backgroundColor = Color::hex(0xF8F8F8),
            .padding = 32,
            .children = {
                // Status cards row
                HStack {
                    .spacing = 16,
                    .children = {
                        // To do card
                        VStack {
                            .padding = 32,
                            .spacing = 32,
                            .backgroundColor = Color::hex(0x313F4D),
                            .borderWidth = 2,
                            .borderColor = Color::hex(0xF8F8F8),
                            .cornerRadius = 16,
                            .expansionBias = 1.0f,
                            .children = {
                                Text {
                                    .value = [&]() {
                                        int count = 0;
                                        for (const auto& task : static_cast<std::vector<TodoItem>>(todos)) {
                                            if (task.status == TaskStatus::todo) count++;
                                        }
                                        return std::to_string(count);
                                    },
                                    .color = Colors::white,
                                    .fontSize = 56,
                                    .fontWeight = FontWeight::bold,
                                    .horizontalAlignment = HorizontalAlignment::leading
                                },
                                Text {
                                    .value = "To do",
                                    .color = Colors::lightGray,
                                    .fontSize = 21,
                                    .fontWeight = FontWeight::bold,
                                    .horizontalAlignment = HorizontalAlignment::leading
                                }
                            }
                        },
                        // On going card
                        VStack {
                            .padding = 32,
                            .spacing = 32,
                            .backgroundColor = Color::hex(0xF1F3F4),
                            .borderWidth = 2,
                            .borderColor = Color::hex(0xE0E1E2),
                            .cornerRadius = 16,
                            .expansionBias = 1.0f,
                            .children = {
                                Text {
                                    .value = [&]() {
                                        int count = 0;
                                        for (const auto& task : static_cast<std::vector<TodoItem>>(todos)) {
                                            if (task.status == TaskStatus::ongoing) count++;
                                        }
                                        return std::to_string(count);
                                    },
                                    .color = Color::hex(0x37393B),
                                    .fontSize = 56,
                                    .fontWeight = FontWeight::bold,
                                    .horizontalAlignment = HorizontalAlignment::leading
                                },
                                Text {
                                    .value = "On going",
                                    .color = Color::hex(0x555759),
                                    .fontSize = 21,
                                    .fontWeight = FontWeight::bold,
                                    .horizontalAlignment = HorizontalAlignment::leading
                                }
                            }
                        },
                        // On review card
                        VStack {
                            .padding = 32,
                            .spacing = 32,
                            .backgroundColor = Color::hex(0xF1F3F4),
                            .borderWidth = 2,
                            .borderColor = Color::hex(0xE0E1E2),
                            .cornerRadius = 16,
                            .expansionBias = 1.0f,
                            .children = {
                                Text {
                                    .value = [&]() {
                                        int count = 0;
                                        for (const auto& task : static_cast<std::vector<TodoItem>>(todos)) {
                                            if (task.status == TaskStatus::review) count++;
                                        }
                                        return std::to_string(count);
                                    },
                                    .color = Color::hex(0x37393B),
                                    .fontSize = 56,
                                    .fontWeight = FontWeight::bold,
                                    .horizontalAlignment = HorizontalAlignment::leading
                                },
                                Text {
                                    .value = "On review",
                                    .color = Color::hex(0x555759),
                                    .fontSize = 21,
                                    .fontWeight = FontWeight::bold,
                                    .horizontalAlignment = HorizontalAlignment::leading
                                }
                            }
                        }
                    }
                },

                // Priority Task section
                VStack {
                    .children = {
                        // Section header
                        HStack {
                            .spacing = 16,
                            .padding = EdgeInsets{64, 0},
                            .children = {
                                Text {
                                    .value = "Priority Task",
                                    .color = Color::hex(0x222222),
                                    .fontSize = 28,
                                    .fontWeight = FontWeight::medium,
                                    .horizontalAlignment = HorizontalAlignment::leading,
                                    .verticalAlignment = VerticalAlignment::center
                                },
                                Spacer {},
                                Text {
                                    .value = "See All",
                                    .color = Color::hex(0x1F6ACD),
                                    .fontSize = 28,
                                    .horizontalAlignment = HorizontalAlignment::trailing,
                                    .verticalAlignment = VerticalAlignment::center
                                }
                            }
                        },

                        // Task list
                        VStack {
                            .spacing = 32,
                            .children = [&]() {
                                std::vector<TodoItem> currentTodos = todos;
                                std::vector<View> children;

                                for (size_t i = 0; i < currentTodos.size(); i++) {
                                    const TodoItem& todo = currentTodos[i];

                                    // Add the task row
                                    children.push_back(View(HStack {
                                        .spacing = 32,
                                        .alignItems = AlignItems::center,
                                        .padding = EdgeInsets{16, 0}, // Add some vertical padding
                                        .children = {
                                            // Date section
                                            VStack {
                                                .spacing = 8,
                                                .children = {
                                                    Text {
                                                        .expansionBias = 1.0f,
                                                        .value = todo.month,
                                                        .color = Color::hex(0x888888),
                                                        .fontSize = 21,
                                                        .horizontalAlignment = HorizontalAlignment::center,
                                                    },
                                                    Text {
                                                        .expansionBias = 1.0f,
                                                        .value = std::to_string(todo.day),
                                                        .color = Color::hex(0x1C78FA),
                                                        .fontSize = 35,
                                                        .fontWeight = FontWeight::medium,
                                                        .horizontalAlignment = HorizontalAlignment::center,
                                                    }
                                                }
                                            },

                                            // Task info section
                                            VStack {
                                                .spacing = 8,
                                                .expansionBias = 1.0f,
                                                .children = {
                                                    Text {
                                                        .expansionBias = 1.0f,
                                                        .value = todo.title,
                                                        .color = Color::hex(0x2F3B45),
                                                        .fontSize = 28,
                                                        .fontWeight = FontWeight::medium,
                                                        .horizontalAlignment = HorizontalAlignment::leading
                                                    },
                                                    Text {
                                                        .expansionBias = 1.0f,
                                                        .value = todo.subtitle,
                                                        .color = Color::hex(0x888888),
                                                        .fontSize = 21,
                                                        .horizontalAlignment = HorizontalAlignment::leading
                                                    }
                                                }
                                            },
                                            CalendarIcon {},
                                            ProgressIcon {
                                                .progress = todo.progress,
                                                .size = 64.0f,
                                                .strokeWidth = 4.0f
                                            }
                                        }
                                    }));

                                    // Add separator after each task except the last one
                                    if (i < currentTodos.size() - 1) {
                                        children.push_back(View(Separator {
                                            .color = Color::hex(0xE0E1E2),
                                            .thickness = 1.0f
                                        }));
                                    }
                                }

                                return children;
                            }
                        },
                    }
                }
            }
        }
    );

    return app.exec();
}
