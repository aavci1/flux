#include <Flux.hpp>
#include <iostream>
#include <cmath>

using namespace flux;

struct Clock {
    FLUX_VIEW_PROPERTIES;

    Property<int> hours = 0;
    Property<int> minutes = 0;
    Property<int> seconds = 0;

    void drawHand(RenderContext& ctx, Point center, float length, float width, float angle, Color color) const {
        auto x = center.x + (length * static_cast<float>(sin((180 - angle) * M_PI / 180.0f)));
        auto y = center.y + (length * static_cast<float>(cos((180 - angle) * M_PI / 180.0f)));

        Path path;
        path.moveTo(center);
        path.lineTo({ x, y });
        
        ctx.setStrokeStyle(StrokeStyle::rounded(color, width));
        ctx.drawPath(path, false, true);
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        auto radius = fmin(bounds.width, bounds.height) / 2 - 20;

        Path frame;
        frame.circle(bounds.center(), radius);
        ctx.setFillStyle(FillStyle::solid(Colors::white));
        ctx.setStrokeStyle(StrokeStyle::solid(Colors::black, 20));
        ctx.drawPath(frame, false, true);

        // Hour marks
        auto hourTickLength = radius / 10.0f;
        auto hourTickWidth = 6.0f;

        Path hourMarks;
        for (int i = 0; i < 12; i++) {
            auto angle = i * 2 * M_PI / 12;
            float x1 = bounds.center().x + (radius - 20 - hourTickLength) * static_cast<float>(cos(angle));
            float y1 = bounds.center().y + (radius - 20 - hourTickLength) * static_cast<float>(sin(angle));
            float x2 = bounds.center().x + (radius - 20) * static_cast<float>(cos(angle));
            float y2 = bounds.center().y + (radius - 20) * static_cast<float>(sin(angle));

            hourMarks.moveTo({ x1, y1 });
            hourMarks.lineTo({ x2, y2 });
        }

        ctx.setStrokeStyle(StrokeStyle::solid(Colors::black, hourTickWidth));
        ctx.drawPath(hourMarks, false, true);

        // Minute marks
        auto minuteTickLength = radius / 10.0f;
        auto minuteTickWidth = 2.0f;

        Path minuteMarks;
        for (int i = 0; i < 60; i++) {
            auto angle = i * 2 * M_PI / 60;
            float x1 = bounds.center().x + (radius - 20 - minuteTickLength) * static_cast<float>(cos(angle));
            float y1 = bounds.center().y + (radius - 20 - minuteTickLength) * static_cast<float>(sin(angle));
            float x2 = bounds.center().x + (radius - 20) * static_cast<float>(cos(angle));
            float y2 = bounds.center().y + (radius - 20) * static_cast<float>(sin(angle));

            minuteMarks.moveTo({ x1, y1 });
            minuteMarks.lineTo({ x2, y2 });
        }

        ctx.setStrokeStyle(StrokeStyle::solid(Colors::black, minuteTickWidth));
        ctx.drawPath(minuteMarks, false, true);

        // numbers
        for (int i = 1; i <= 12; i++) {
            auto angle = i * 30 * M_PI / 180 - M_PI / 2;
            float x = bounds.center().x + (radius * 0.7f) * static_cast<float>(cos(angle));
            float y = bounds.center().y + (radius * 0.7f) * static_cast<float>(sin(angle));
            ctx.drawText(std::to_string(i), { x, y }, TextStyle::centered("sans", 72.0f, Colors::black));
        }

        drawHand(ctx, bounds.center(), radius * 0.4f, 12.0f, (hours * 30) + (minutes * 0.5f), Colors::black);
        drawHand(ctx, bounds.center(), radius * 0.55f, 8.0f, (minutes * 6) + (seconds * 0.1f), Colors::black);
        drawHand(ctx, bounds.center(), radius * 0.7f, 4.0f, seconds * 6, Colors::red);

        Path centerCircle;
        centerCircle.circle(bounds.center(), 16);
        ctx.setFillStyle(FillStyle::solid(Colors::white));
        ctx.setStrokeStyle(StrokeStyle::solid(Colors::red, 6));
        ctx.drawPath(centerCircle, true, true);
    }
};