#include <Flux.hpp>
#include <cmath>

using namespace flux;

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

// Example component demonstrating the new RenderContext capabilities
struct RenderContextDemo {
    FLUX_VIEW_PROPERTIES;

    Property<Size> size = Size{800, 600};
    Property<float> animationTime = 0.0f;

    LayoutNode layout(const Rect& bounds) const {
        return LayoutNode(View(*this), bounds);
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        // Save the current state
        ctx.save();

        // ============================================================================
        // DEMONSTRATE FILL STYLE FACTORY METHODS
        // ============================================================================

        // Create a linear gradient background using factory method
        FillStyle linearGradient = FillStyle::linearGradient(
            Point(0, 0), Point(bounds.width, bounds.height),
            Color::hex(0x667eea), Color::hex(0x764ba2)
        );

        ctx.setFillStyle(linearGradient);
        ctx.drawRoundedRect(bounds, 20, Colors::white); // This will use the gradient

        // Demo different fill styles using factory methods
        float fillY = 300;

        // Solid fill
        ctx.setFillStyle(FillStyle::solid(Color::hex(0xe74c3c)));
        ctx.drawRoundedRect(Rect(50, fillY, 80, 60), 10, Colors::white);

        // Linear gradient
        FillStyle linearFill = FillStyle::linearGradient(
            Point(0, 0), Point(80, 60),
            Color::hex(0x3498db), Color::hex(0x2ecc71)
        );
        ctx.setFillStyle(linearFill);
        ctx.drawRoundedRect(Rect(150, fillY, 80, 60), 10, Colors::white);

        // Radial gradient
        FillStyle radialFill = FillStyle::radialGradient(
            Point(40, 30), 10, 40,
            Color::hex(0xf39c12), Color::hex(0xe67e22)
        );
        ctx.setFillStyle(radialFill);
        ctx.drawRoundedRect(Rect(250, fillY, 80, 60), 10, Colors::white);

        // Box gradient
        FillStyle boxFill = FillStyle::boxGradient(
            Rect(0, 0, 80, 60), 15, 10,
            Color::hex(0x9b59b6), Color::hex(0x8e44ad)
        );
        ctx.setFillStyle(boxFill);
        ctx.drawRoundedRect(Rect(350, fillY, 80, 60), 15, Colors::white);

        // ============================================================================
        // DEMONSTRATE STROKE STYLE FACTORY METHODS
        // ============================================================================

        // Demo different stroke styles using factory methods
        float strokeY = 100;

        // Solid stroke
        ctx.setStrokeStyle(StrokeStyle::solid(Color::hex(0x2c3e50), 4.0f));
        ctx.beginPath();
        ctx.moveTo(Point(50, strokeY));
        ctx.lineTo(Point(150, strokeY));
        ctx.stroke();

        // Dashed stroke with animation
        auto dashedStroke = StrokeStyle::dashed(Color::hex(0xe74c3c), 3.0f, {10.0f, 5.0f, 3.0f, 5.0f});
        dashedStroke.dashOffset = animationTime * 20.0f; // Animated dash offset
        ctx.setStrokeStyle(dashedStroke);
        ctx.beginPath();
        ctx.moveTo(Point(50, strokeY + 30));
        ctx.lineTo(Point(150, strokeY + 30));
        ctx.stroke();

        // Rounded stroke
        ctx.setStrokeStyle(StrokeStyle::rounded(Color::hex(0x27ae60), 5.0f));
        ctx.beginPath();
        ctx.moveTo(Point(50, strokeY + 60));
        ctx.lineTo(Point(150, strokeY + 60));
        ctx.stroke();

        // Square stroke
        ctx.setStrokeStyle(StrokeStyle::square(Color::hex(0x8e44ad), 3.0f));
        ctx.beginPath();
        ctx.moveTo(Point(50, strokeY + 90));
        ctx.lineTo(Point(150, strokeY + 90));
        ctx.stroke();

        // ============================================================================
        // DEMONSTRATE RADIAL GRADIENTS
        // ============================================================================

        Point center = Point(400, 150);
        FillStyle radialGradient = FillStyle::radialGradient(
            center, 20.0f, 80.0f,
            Color::hex(0xff6b6b), Color::hex(0x4ecdc4)
        );

        ctx.setFillStyle(radialGradient);
        ctx.drawCircle(center, 80, Colors::white);

        // ============================================================================
        // DEMONSTRATE SHADOW FACTORY METHODS
        // ============================================================================

        // Demo different shadow types using factory methods
        float shadowY = 200;

        // Drop shadow
        auto dropShadow = Shadow::drop(3, 3, 5, Color::hex(0x000000).opacity(0.3));
        ctx.drawShadow(Rect(50, shadowY, 100, 50), 10, dropShadow);
        ctx.setFillStyle(FillStyle::solid(Color::hex(0x3498db)));
        ctx.drawRoundedRect(Rect(50, shadowY, 100, 50), 10, Colors::white);

        // Glow effect
        auto glowShadow = Shadow::glow(10, Color::hex(0xf39c12));
        ctx.drawShadow(Rect(200, shadowY, 100, 50), 10, glowShadow);
        ctx.setFillStyle(FillStyle::solid(Color::hex(0xe74c3c)));
        ctx.drawRoundedRect(Rect(200, shadowY, 100, 50), 10, Colors::white);

        // Subtle shadow
        auto subtleShadow = Shadow::subtle(2, 2, 3, Color::hex(0x7f8c8d));
        ctx.drawShadow(Rect(350, shadowY, 100, 50), 10, subtleShadow);
        ctx.setFillStyle(FillStyle::solid(Color::hex(0x27ae60)));
        ctx.drawRoundedRect(Rect(350, shadowY, 100, 50), 10, Colors::white);

        // Inner shadow
        auto innerShadow = Shadow::inner(1, 1, 3, Color::hex(0x2c3e50));
        ctx.drawShadow(Rect(500, shadowY, 100, 50), 10, innerShadow);
        ctx.setFillStyle(FillStyle::solid(Color::hex(0x9b59b6)));
        ctx.drawRoundedRect(Rect(500, shadowY, 100, 50), 10, Colors::white);

        // ============================================================================
        // DEMONSTRATE ADVANCED PATH BUILDING
        // ============================================================================

        ctx.save();
        ctx.translate(100, 300);
        ctx.rotate(animationTime * 0.5f); // Rotate with animation

        // Create a complex shape using path building
        ctx.beginPath();
        ctx.moveTo(Point(0, -50));
        ctx.quadTo(Point(25, -25), Point(0, 0));
        ctx.quadTo(Point(-25, 25), Point(0, 50));
        ctx.quadTo(Point(25, 25), Point(50, 0));
        ctx.quadTo(Point(25, -25), Point(0, -50));
        ctx.closePath();

        // Fill with a gradient
        FillStyle starGradient = FillStyle::linearGradient(
            Point(-50, -50), Point(50, 50),
            Color::hex(0xffd700), Color::hex(0xff8c00)
        );
        ctx.setFillStyle(starGradient);
        ctx.fill();

        // Stroke with a contrasting color
        ctx.setStrokeColor(Color::hex(0x8b4513));
        ctx.setStrokeWidth(3.0f);
        ctx.setLineCap(LineCap::Round);
        ctx.stroke();

        ctx.restore();

        // ============================================================================
        // DEMONSTRATE ELLIPSES AND ARCS
        // ============================================================================

        // Draw an ellipse
        ctx.setFillColor(Color::hex(0x9b59b6).opacity(0.7));
        ctx.drawEllipse(Point(350, 350), 60, 30, Colors::white);

        // Draw an arc with advanced stroke style
        StrokeStyle arcStyle;
        arcStyle.color = Color::hex(0xe74c3c);
        arcStyle.width = 8.0f;
        arcStyle.cap = LineCap::Round;
        ctx.drawArc(Point(350, 350), 80, 0, animationTime * 2.0f, arcStyle);

        // ============================================================================
        // DEMONSTRATE TEXT STYLE FACTORY METHODS
        // ============================================================================

        // Demo different text styles using factory methods
        float textY = 20;

        // Title with bold style
        auto titleStyle = TextStyle::bold("default", 24.0f, Color::hex(0x2c3e50));
        titleStyle.hAlign = HorizontalAlignment::center;
        titleStyle.vAlign = VerticalAlignment::top;
        titleStyle.letterSpacing = 1.0f;
        ctx.drawText("Advanced RenderContext Demo", Point(400, textY), titleStyle);

        // Regular text
        auto regularStyle = TextStyle::regular("default", 16.0f, Color::hex(0x34495e));
        ctx.drawText("Regular Text Style", Point(50, textY + 40), regularStyle);

        // Light text
        auto lightStyle = TextStyle::light("default", 14.0f, Color::hex(0x7f8c8d));
        ctx.drawText("Light Text Style", Point(50, textY + 70), lightStyle);

        // Centered text
        auto centeredStyle = TextStyle::centered("default", 18.0f, Color::hex(0xe74c3c));
        ctx.drawText("Centered Text", Point(400, textY + 100), centeredStyle);

        // Gradient text
        FillStyle textGradient = FillStyle::linearGradient(
            Point(0, 0), Point(200, 0),
            Color::hex(0x667eea), Color::hex(0x764ba2)
        );
        auto gradientStyle = TextStyle::gradient("default", 16.0f, textGradient);
        ctx.drawText("Gradient Text Style", Point(50, textY + 130), gradientStyle);

        // ============================================================================
        // DEMONSTRATE COMPOSITE OPERATIONS
        // ============================================================================

        ctx.save();
        ctx.translate(600, 300);

        // Draw base circle
        ctx.setFillColor(Color::hex(0x3498db));
        ctx.drawCircle(Point(0, 0), 50, Colors::white);

        // Draw overlapping circle with different composite operation
        ctx.setCompositeOperation(CompositeOperation::Lighter);
        ctx.setFillColor(Color::hex(0xe74c3c));
        ctx.drawCircle(Point(30, 0), 50, Colors::white);

        ctx.restore();

        // ============================================================================
        // DEMONSTRATE CLIPPING
        // ============================================================================

        ctx.save();
        ctx.clipRoundedRect(Rect(50, 450, 300, 100), 20);

        // Draw a pattern that will be clipped
        for (int i = 0; i < 10; i++) {
            ctx.setFillColor(Color::hex(0x3498db).opacity(0.3));
            ctx.drawRect(Rect(i * 30, 450 + i * 5, 25, 25), Colors::white);
        }

        ctx.restore();

        // ============================================================================
        // DEMONSTRATE TRANSFORMATIONS
        // ============================================================================

        ctx.save();
        ctx.translate(450, 500);
        ctx.rotate(animationTime);
        ctx.scale(1.0f + 0.3f * std::sin(animationTime * 2.0f), 1.0f + 0.3f * std::cos(animationTime * 2.0f));

        ctx.setFillColor(Color::hex(0xf39c12));
        ctx.drawRect(Rect(-25, -25, 50, 50), Colors::white);

        ctx.restore();

        // Restore the initial state
        ctx.restore();
    }
};

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    State animationTime = 0.0f;

    Window window({
        .size = {800, 600},
        .title = "RenderContext Demo"
    });

    window.setRootView(RenderContextDemo {
        .animationTime = animationTime
    });

    timeout([&]() {
        animationTime += 0.016f;
    }, 16);

    return app.exec();
}
