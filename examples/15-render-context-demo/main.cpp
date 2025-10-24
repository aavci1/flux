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
        Path gradientPath;
        gradientPath.rect(bounds, 20);

        ctx.setStrokeStyle(StrokeStyle::none()); // Clear stroke to prevent unwanted stroke
        ctx.setFillStyle(FillStyle::linearGradient(
            Point(0, 0), 
            Point(bounds.width, bounds.height),
            Color::hex(0x667eea), 
            Color::hex(0x764ba2)
        ));
        ctx.drawPath(gradientPath);

        // Demo different fill styles using factory methods
        float fillY = 300;

        // Solid fill
        Path solidPath;
        solidPath.rect(Rect(50, fillY, 80, 60), 10);
        
        ctx.setStrokeStyle(StrokeStyle::none()); // Clear stroke to prevent unwanted stroke
        ctx.setFillStyle(FillStyle::solid(Color::hex(0xe74c3c)));
        ctx.drawPath(solidPath);

        // Linear gradient
        Path linearPath;
        linearPath.rect(Rect(150, fillY, 80, 60), 10);
        
        ctx.setStrokeStyle(StrokeStyle::none()); // Clear stroke to prevent unwanted stroke
        ctx.setFillStyle(FillStyle::linearGradient(
            Point(0, 0), Point(80, 60),
            Color::hex(0x3498db), Color::hex(0x2ecc71)
        ));
        ctx.drawPath(linearPath);

        // Radial gradient
        Path radialPath;
        radialPath.rect(Rect(250, fillY, 80, 60), 10);
        
        ctx.setStrokeStyle(StrokeStyle::none()); // Clear stroke to prevent unwanted stroke
        ctx.setFillStyle(FillStyle::radialGradient(
            Point(40, 30), 10, 40,
            Color::hex(0xf39c12), Color::hex(0xe67e22)
        ));
        ctx.drawPath(radialPath);

        // Box gradient
        Path boxPath;
        boxPath.rect(Rect(350, fillY, 80, 60), 15);
        
        ctx.setStrokeStyle(StrokeStyle::none()); // Clear stroke to prevent unwanted stroke
        ctx.setFillStyle(FillStyle::boxGradient(
            Rect(0, 0, 80, 60), 15, 10,
            Color::hex(0x9b59b6), Color::hex(0x8e44ad)
        ));
        ctx.drawPath(boxPath);

        // ============================================================================
        // DEMONSTRATE STROKE STYLE FACTORY METHODS
        // ============================================================================

        // Demo different stroke styles using factory methods
        float strokeY = 100;

        // Solid stroke
        Path solidPath2;
        solidPath2.moveTo(Point(50, strokeY));
        solidPath2.lineTo(Point(150, strokeY));
        ctx.setFillStyle(FillStyle::none()); // Clear fill to prevent unwanted fill
        ctx.setStrokeStyle(StrokeStyle::solid(Color::hex(0x2c3e50), 4.0f));
        ctx.drawPath(solidPath2);

        // Dashed stroke with animation
        Path dashedPath;
        dashedPath.moveTo(Point(50, strokeY + 30));
        dashedPath.lineTo(Point(150, strokeY + 30));
        ctx.setFillStyle(FillStyle::none()); // Clear fill to prevent unwanted fill
        ctx.setStrokeStyle(StrokeStyle::dashed(
            Color::hex(0xe74c3c), 
            3.0f, 
            {10.0f, 5.0f, 3.0f, 5.0f}, 
            animationTime * 20.0f
        ));
        ctx.drawPath(dashedPath);

        // Rounded stroke
        Path roundedPath;
        roundedPath.moveTo(Point(50, strokeY + 60));
        roundedPath.lineTo(Point(150, strokeY + 60));
        ctx.setFillStyle(FillStyle::none()); // Clear fill to prevent unwanted fill
        ctx.setStrokeStyle(StrokeStyle::rounded(Color::hex(0x27ae60), 5.0f));
        ctx.drawPath(roundedPath);

        // Square stroke
        Path squarePath;
        squarePath.moveTo(Point(50, strokeY + 90));
        squarePath.lineTo(Point(150, strokeY + 90));
        ctx.setFillStyle(FillStyle::none()); // Clear fill to prevent unwanted fill
        ctx.setStrokeStyle(StrokeStyle::square(Color::hex(0x8e44ad), 3.0f));
        ctx.drawPath(squarePath);

        // ============================================================================
        // DEMONSTRATE RADIAL GRADIENTS
        // ============================================================================

        Point center = Point(400, 150);
        Path radialPath2;
        radialPath2.circle(center, 80);
        ctx.setStrokeStyle(StrokeStyle::none()); // Clear stroke to prevent unwanted stroke
        ctx.setFillStyle(FillStyle::radialGradient(
            center, 20.0f, 80.0f,
            Color::hex(0xff6b6b), Color::hex(0x4ecdc4)
        ));
        ctx.drawPath(radialPath2);

        // ============================================================================
        // DEMONSTRATE ADVANCED PATH BUILDING
        // ============================================================================

        ctx.save();
        ctx.translate(100, 300);
        ctx.rotate(animationTime * 0.5f); // Rotate with animation

        // Create a complex shape using path building
        Path complexPath;
        complexPath.moveTo(Point(0, -50));
        complexPath.quadTo(Point(25, -25), Point(0, 0));
        complexPath.quadTo(Point(-25, 25), Point(0, 50));
        complexPath.quadTo(Point(25, 25), Point(50, 0));
        complexPath.quadTo(Point(25, -25), Point(0, -50));
        complexPath.close();
        ctx.setStrokeStyle(StrokeStyle::none()); // Clear stroke to prevent unwanted stroke
        ctx.setFillStyle(FillStyle::linearGradient(
            Point(-50, -50), Point(50, 50),
            Color::hex(0xffd700), Color::hex(0xff8c00)
        ));
        ctx.drawPath(complexPath);

        // Stroke the complex shape with a contrasting color
        ctx.setFillStyle(FillStyle::none()); // Clear fill to prevent unwanted fill
        ctx.setStrokeStyle(StrokeStyle::solid(Color::hex(0x8b4513), 3.0f));
        ctx.drawPath(complexPath);

        ctx.restore();

        // ============================================================================
        // DEMONSTRATE ELLIPSES AND ARCS
        // ============================================================================

        // Draw an ellipse
        Path ellipsePath;
        ellipsePath.ellipse(Point(350, 350), 60, 30);
        ctx.setStrokeStyle(StrokeStyle::none()); // Clear stroke to prevent unwanted stroke
        ctx.setFillStyle(FillStyle::solid(Color::hex(0x9b59b6).opacity(0.7)));
        ctx.drawPath(ellipsePath);

        // Draw an arc with advanced stroke style
        Path arcPath;
        arcPath.arc(Point(350, 350), 80, 0, animationTime * 2.0f);
        ctx.setFillStyle(FillStyle::none()); // Clear fill to prevent unwanted fill
        ctx.setStrokeStyle(StrokeStyle::rounded(Color::hex(0xe74c3c), 8.0f));
        ctx.drawPath(arcPath);

        // ============================================================================
        // DEMONSTRATE TEXT STYLE FACTORY METHODS
        // ============================================================================

        // Demo different text styles using factory methods
        float textY = 20;

        // Title with bold style
        auto titleStyle = TextStyle::bold("default", 24.0f);
        titleStyle.letterSpacing = 1.0f;
        ctx.setTextStyle(titleStyle);
        ctx.setStrokeStyle(StrokeStyle::none()); // Clear stroke to prevent unwanted stroke
        ctx.setFillStyle(FillStyle{.type = FillStyle::Type::Solid, .color = Color::hex(0x2c3e50)});
        ctx.drawText("Advanced RenderContext Demo", Point(400, textY), HorizontalAlignment::center, VerticalAlignment::center);

        // Regular text
        auto regularStyle = TextStyle::regular("default", 16.0f);
        ctx.setTextStyle(regularStyle);
        ctx.setStrokeStyle(StrokeStyle::none()); // Clear stroke to prevent unwanted stroke
        ctx.setFillStyle(FillStyle{.type = FillStyle::Type::Solid, .color = Color::hex(0x34495e)});
        ctx.drawText("Regular Text Style", Point(50, textY + 40), HorizontalAlignment::leading, VerticalAlignment::center);

        // Light text
        auto lightStyle = TextStyle::light("default", 14.0f);
        ctx.setTextStyle(lightStyle);
        ctx.setStrokeStyle(StrokeStyle::none()); // Clear stroke to prevent unwanted stroke
        ctx.setFillStyle(FillStyle{.type = FillStyle::Type::Solid, .color = Color::hex(0x7f8c8d)});
        ctx.drawText("Light Text Style", Point(50, textY + 70), HorizontalAlignment::leading, VerticalAlignment::center);

        // Centered text
        auto centeredStyle = TextStyle::regular("default", 18.0f);
        ctx.setTextStyle(centeredStyle);
        ctx.setStrokeStyle(StrokeStyle::none()); // Clear stroke to prevent unwanted stroke
        ctx.setFillStyle(FillStyle{.type = FillStyle::Type::Solid, .color = Color::hex(0xe74c3c)});
        ctx.drawText("Centered Text", Point(400, textY + 100), HorizontalAlignment::center, VerticalAlignment::center);

        // Gradient text
        FillStyle textGradient = FillStyle::linearGradient(
            Point(0, 0), Point(200, 0),
            Color::hex(0x667eea), Color::hex(0x764ba2)
        );
        auto gradientStyle = TextStyle::regular("default", 16.0f);
        ctx.setTextStyle(gradientStyle);
        ctx.setStrokeStyle(StrokeStyle::none()); // Clear stroke to prevent unwanted stroke
        ctx.setFillStyle(textGradient);
        ctx.drawText("Gradient Text Style", Point(50, textY + 130), HorizontalAlignment::leading, VerticalAlignment::center);

        // ============================================================================
        // DEMONSTRATE COMPOSITE OPERATIONS
        // ============================================================================

        ctx.save();
        ctx.translate(600, 300);

        // Draw base circle
        Path baseCircle;
        baseCircle.circle(Point(0, 0), 50);
        ctx.setStrokeStyle(StrokeStyle::none()); // Clear stroke to prevent unwanted stroke
        ctx.setFillStyle(FillStyle::solid(Color::hex(0x3498db)));
        ctx.drawPath(baseCircle);

        // Draw overlapping circle with different composite operation
        ctx.setCompositeOperation(CompositeOperation::Lighter);
        Path overlappingCircle;
        overlappingCircle.circle(Point(30, 0), 50);
        ctx.setStrokeStyle(StrokeStyle::none()); // Clear stroke to prevent unwanted stroke
        ctx.setFillStyle(FillStyle::solid(Color::hex(0xe74c3c)));
        ctx.drawPath(overlappingCircle);

        ctx.restore();

        // ============================================================================
        // DEMONSTRATE CLIPPING
        // ============================================================================

        ctx.save();
        Path clippingPath;
        clippingPath.rect(Rect(50, 450, 300, 100), 20);
        ctx.clipPath(clippingPath);

        // Draw a pattern that will be clipped
        Path patternPath;
        patternPath.rect(Rect(50, 450, 300, 100), 20);
        ctx.setStrokeStyle(StrokeStyle::none()); // Clear stroke to prevent unwanted stroke
        ctx.setFillStyle(FillStyle::solid(Color::hex(0x3498db).opacity(0.3)));
        ctx.drawPath(patternPath);

        ctx.restore();

        // ============================================================================
        // DEMONSTRATE TRANSFORMATIONS
        // ============================================================================

        ctx.save();
        ctx.translate(450, 500);
        ctx.rotate(animationTime);
        ctx.scale(1.0f + 0.3f * std::sin(animationTime * 2.0f), 1.0f + 0.3f * std::cos(animationTime * 2.0f));

        Path transformationPath;
        transformationPath.rect(Rect(-25, -25, 50, 50));
        ctx.setStrokeStyle(StrokeStyle::none()); // Clear stroke to prevent unwanted stroke
        ctx.setFillStyle(FillStyle::solid(Color::hex(0xf39c12)));
        ctx.drawPath(transformationPath);

        ctx.restore();

        // Restore the initial state
        ctx.restore();
    }
};

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    Property animationTime = 0.0f;

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
