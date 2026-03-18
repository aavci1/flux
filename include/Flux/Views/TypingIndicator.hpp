#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <cmath>
#include <chrono>
#include <thread>
#include <atomic>
#include <memory>

namespace flux {

struct TypingIndicator {
    FLUX_VIEW_PROPERTIES;

    Property<bool> isVisible = true;
    Property<Color> dotColor = Colors::gray;
    Property<float> dotRadius = 4.0f;
    Property<float> dotSpacing = 6.0f;
    Property<float> animationSpeed = 1.2f;

    Property<float> animPhase = 0.0f;
    mutable std::shared_ptr<std::atomic<bool>> animRunning =
        std::make_shared<std::atomic<bool>>(false);

    void init() {
        if (!animRunning->exchange(true)) {
            std::thread([this]() {
                while (*animRunning) {
                    float t = std::chrono::duration<float>(
                        std::chrono::steady_clock::now().time_since_epoch()).count();
                    animPhase = t;
                    std::this_thread::sleep_for(std::chrono::milliseconds(33));
                }
            }).detach();
        }
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        if (!static_cast<bool>(isVisible)) return;

        float r = dotRadius;
        float sp = dotSpacing;
        float speed = animationSpeed;
        float time = animPhase;
        float pi = 3.14159265f;

        float totalW = r * 2 * 3 + sp * 2;
        float cx = bounds.x + (bounds.width - totalW) / 2 + r;
        float cy = bounds.y + bounds.height / 2;

        for (int i = 0; i < 3; i++) {
            float offset = std::sin(time * speed * 2 * pi + i * pi / 1.5f) * 4.0f;
            float alpha = 0.4f + 0.6f * (0.5f + 0.5f * std::sin(time * speed * 2 * pi + i * pi / 1.5f));

            ctx.setFillStyle(FillStyle::solid(static_cast<Color>(dotColor).opacity(alpha)));
            ctx.setStrokeStyle(StrokeStyle::none());
            ctx.drawCircle({cx + i * (r * 2 + sp), cy + offset}, r);
        }
    }

    Size preferredSize(TextMeasurement&) const {
        float r = dotRadius;
        float sp = dotSpacing;
        return {r * 2 * 3 + sp * 2 + 16, r * 2 + 16};
    }
};

} // namespace flux
