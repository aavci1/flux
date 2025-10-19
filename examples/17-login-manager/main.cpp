#include <Flux.hpp>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cmath>

using namespace flux;

// Custom circular avatar component
struct UserAvatar {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> username = "User";
    Property<Size> size = Size{120, 120};
    Property<Color> avatarBorderColor = Colors::white;
    Property<float> avatarBorderWidth = 2.0f;

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        auto radius = fmin(bounds.width, bounds.height) / 2;

        // Draw circular background with gradient-like effect
        Color bgColor = Color::hex(0x4A90E2); // Blue gradient start
        Path path;
        path.circle(bounds.center(), radius);
        ctx.setFillStyle(FillStyle::solid(bgColor));
        ctx.drawPath(path);

        // Draw border
        Path borderPath;
        borderPath.arc(bounds.center(), radius, 0, 2 * M_PI);
        ctx.setStrokeStyle(StrokeStyle::solid(static_cast<Color>(avatarBorderColor), static_cast<float>(avatarBorderWidth)));
        ctx.drawPath(borderPath);

        // Draw user initial
        std::string initial = static_cast<std::string>(username).substr(0, 1);
        TextStyle textStyle = TextStyle::bold("Arial", 48);
        ctx.setTextStyle(textStyle);
        ctx.setFillStyle(FillStyle{.type = FillStyle::Type::Solid, .color = Colors::white});
        ctx.drawText(initial, bounds.center(), HorizontalAlignment::center, VerticalAlignment::center);
    }

    Size preferredSize(TextMeasurement& /* textMeasurer */) const {
        return static_cast<Size>(size);
    }
};

// Custom password input field with enhanced glass effect
struct PasswordField {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> placeholder = "Password";
    Property<std::string> value = "";
    Property<Size> size = Size{300, 50};
    Property<Color> fieldBackgroundColor = Color(1.0f, 1.0f, 1.0f, 0.15f);
    Property<Color> fieldBorderColor = Colors::white;
    Property<float> fieldBorderWidth = 1.0f;
    Property<float> fieldCornerRadius = 25.0f;

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        // Draw glass background with multiple layers
        float radius = static_cast<float>(fieldCornerRadius);
        
        // Layer 1: Darker base
        Color darkBase = Color(0.0f, 0.0f, 0.0f, 0.1f);
        Path path;
        path.rect(bounds, radius);
        ctx.setFillStyle(FillStyle::solid(darkBase));
        ctx.drawPath(path);

        // Layer 2: Main glass background
        Color bgColor = static_cast<Color>(fieldBackgroundColor);
        Path path2;
        path2.rect(bounds, radius);
        ctx.setFillStyle(FillStyle::solid(bgColor));
        ctx.drawPath(path2);

        // Layer 3: Subtle highlight at top
        Color highlight = Color(1.0f, 1.0f, 1.0f, 0.1f);
        Rect highlightRect = {bounds.x, bounds.y, bounds.width, bounds.height * 0.4f};
        Path path3;
        path3.rect(highlightRect, radius);
        ctx.setFillStyle(FillStyle::solid(highlight));
        ctx.drawPath(path3);

        // Draw border
        Path path4;
        path4.rect(bounds, radius);
        ctx.setStrokeStyle(StrokeStyle::solid(static_cast<Color>(fieldBorderColor), static_cast<float>(fieldBorderWidth)));
        ctx.drawPath(path4);

        // Draw placeholder or password dots
        std::string displayText = static_cast<std::string>(value).empty() ?
                                 static_cast<std::string>(placeholder) :
                                 std::string(static_cast<std::string>(value).length(), '*');

        TextStyle textStyle = TextStyle::regular("Arial", 16);
        ctx.setTextStyle(textStyle);
        ctx.setFillStyle(FillStyle{.type = FillStyle::Type::Solid, .color = Colors::white});
        ctx.drawText(displayText, {bounds.x + 20, bounds.center().y}, HorizontalAlignment::leading, VerticalAlignment::center);
    }

    Size preferredSize(TextMeasurement& /* textMeasurer */) const {
        return static_cast<Size>(size);
    }
};

// Custom submit button
struct SubmitButton {
    FLUX_VIEW_PROPERTIES;

    Property<Size> size = Size{50, 50};
    Property<Color> buttonBackgroundColor = Colors::white;
    Property<float> buttonCornerRadius = 25.0f;

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        // Draw circular button
        Color bgColor = static_cast<Color>(buttonBackgroundColor);
        Path path;
        path.rect(bounds, static_cast<float>(buttonCornerRadius));
        ctx.setFillStyle(FillStyle::solid(bgColor));
        ctx.drawPath(path);

        // Draw arrow icon (simple triangle)
        Point center = bounds.center();
        float arrowSize = 12.0f;

        // Draw right-pointing arrow
        Path path2;
        path2.moveTo({center.x - arrowSize/2, center.y - arrowSize/2});
        path2.lineTo({center.x + arrowSize/2, center.y});
        path2.lineTo({center.x - arrowSize/2, center.y + arrowSize/2});
        ctx.setStrokeStyle(StrokeStyle::solid(Color::hex(0x333333), 3.0f));
        ctx.drawPath(path2);
    }

    Size preferredSize(TextMeasurement& /* textMeasurer */) const {
        return static_cast<Size>(size);
    }
};

// Custom action button for system actions
struct ActionButton {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> iconPath = "";
    Property<Size> size = Size{60, 60};
    Property<Color> buttonBorderColor = Colors::white;
    Property<float> buttonBorderWidth = 1.0f;
    Property<std::string> action = "";

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        // Draw circular border
        Path path;
        path.arc(bounds.center(), bounds.width / 2, 0, 2 * M_PI);
        ctx.setStrokeStyle(StrokeStyle::solid(static_cast<Color>(buttonBorderColor), static_cast<float>(buttonBorderWidth)));
        ctx.drawPath(path);

        // Draw icon based on action type
        std::string actionType = static_cast<std::string>(action);
        Point center = bounds.center();

        if (actionType == "emergency") {
            // Draw "E" symbol
            TextStyle textStyle = TextStyle::bold("Arial", 20);
            ctx.setTextStyle(textStyle);
            ctx.setFillStyle(FillStyle{.type = FillStyle::Type::Solid, .color = Colors::white});
            ctx.drawText("E", center, HorizontalAlignment::center, VerticalAlignment::center);
        } else if (actionType == "restart") {
            // Draw circular arrow
            float radius = 15.0f;
            Path path;
            path.arc(center, radius, 0, 2 * M_PI);
            ctx.setStrokeStyle(StrokeStyle::solid(Colors::white, 2.0f));
            ctx.drawPath(path);
            // Draw arrow head
            Path path2;
            path2.moveTo({center.x + radius - 5, center.y});
            path2.lineTo({center.x + radius + 5, center.y});
            ctx.setStrokeStyle(StrokeStyle::solid(Colors::white, 3.0f));
            ctx.drawPath(path2);
        } else if (actionType == "shutdown") {
            // Draw power symbol
            float radius = 15.0f;
            Path path;
            path.arc(center, radius, 0, 2 * M_PI);
            ctx.setStrokeStyle(StrokeStyle::solid(Colors::white, 2.0f));
            ctx.drawPath(path);
            // Draw vertical line
            Path path2;
            path2.moveTo({center.x, center.y - radius + 5});
            path2.lineTo({center.x, center.y + radius - 5});
            ctx.setStrokeStyle(StrokeStyle::solid(Colors::white, 3.0f));
            ctx.drawPath(path2);
        } else if (actionType == "switch") {
            // Draw double arrow
            float arrowSize = 8.0f;
            Path path;
            path.moveTo({center.x - arrowSize, center.y});
            path.lineTo({center.x + arrowSize, center.y});
            ctx.setStrokeStyle(StrokeStyle::solid(Colors::white, 3.0f));
            ctx.drawPath(path);
            Path path2;
            path2.moveTo({center.x - arrowSize, center.y - 5});
            path2.lineTo({center.x + arrowSize, center.y - 5});
            ctx.setStrokeStyle(StrokeStyle::solid(Colors::white, 3.0f));
            ctx.drawPath(path2);
        }
    }

    Size preferredSize(TextMeasurement& /* textMeasurer */) const {
        return static_cast<Size>(size);
    }
};

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    Window window({
        .size = {1920, 1080},
        .title = "WhiteSur Login Manager"
    });

    // State for time display
    State currentTime = std::string("18:24");
    State currentDate = std::string("Monday, December 16");
    State username = std::string("User");
    State password = std::string("");

    window.setRootView(
        VStack {
            .backgroundImage = BackgroundImage {
                .imagePath = "./background.jpg",
                .size = BackgroundSize::Cover,
                .position = BackgroundPosition::Center,
            },
            .padding = 120,
            .justifyContent = JustifyContent::spaceBetween,
            .alignItems = AlignItems::center,
            .children = {
                Text {
                    .value = currentTime,
                    .fontSize = 96,
                    .fontWeight = FontWeight::bold,
                    .color = Colors::white,
                    .horizontalAlignment = HorizontalAlignment::center
                },
                Text {
                    .value = currentDate,
                    .fontSize = 18,
                    .color = Colors::white,
                    .horizontalAlignment = HorizontalAlignment::center
                },
                Spacer {},
                UserAvatar {
                    .username = username,
                    .size = Size{120, 120}
                },
                Spacer {},
                HStack {
                    .spacing = 20,
                    .alignItems = AlignItems::center,
                    .justifyContent = JustifyContent::center,
                    .children = {
                        PasswordField {
                            .placeholder = "Password",
                            .value = password,
                            .size = Size{300, 50}
                        },
                        SubmitButton {
                            .size = Size{50, 50}
                        }
                    }
                },
                Spacer {},
                HStack {
                    .compressionBias = 0,
                    .expansionBias = 0,
                    .spacing = 40,
                    .justifyContent = JustifyContent::center,
                    .padding = EdgeInsets{0, 0, 60, 0},
                    .children = {
                        ActionButton {
                            .action = "emergency",
                            .size = Size{60, 60}
                        },
                        ActionButton {
                            .action = "restart",
                            .size = Size{60, 60}
                        },
                        ActionButton {
                            .action = "shutdown",
                            .size = Size{60, 60}
                        },
                        ActionButton {
                            .action = "switch",
                            .size = Size{60, 60}
                        }
                    }
                }
            }
        }
    );

    // Update time every second
    std::thread([&currentTime, &currentDate]() {
        while (true) {
            auto now = std::chrono::system_clock::now();
            auto in_time_t = std::chrono::system_clock::to_time_t(now);

            std::stringstream ts;
            ts << std::put_time(std::localtime(&in_time_t), "%H:%M");
            currentTime = ts.str();

            std::stringstream ds;
            ds << std::put_time(std::localtime(&in_time_t), "%A, %B %d");
            currentDate = ds.str();

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }).detach();

    std::cout << "WhiteSur Login Manager started. Press Ctrl+C to exit." << std::endl;

    return app.exec();
}
