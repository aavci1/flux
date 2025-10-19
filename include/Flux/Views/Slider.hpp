#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>

namespace flux {

struct Slider {
    FLUX_VIEW_PROPERTIES;

    Property<float> value = 0.6;

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        // Draw progress line
        Path progressPath;
        progressPath.moveTo({ bounds.x, bounds.y + bounds.height / 2 });
        progressPath.lineTo({ bounds.x + bounds.width * this->value, bounds.y + bounds.height / 2 });
        ctx.setStrokeStyle(StrokeStyle {
            .color = Colors::blue,
            .width = 4.0f,
            .cap = LineCap::Round
        });
        ctx.drawPath(progressPath, false, true);

        // Draw remaining line
        Path remainingPath;
        remainingPath.moveTo({ bounds.x + bounds.width * this->value, bounds.y + bounds.height / 2 });
        remainingPath.lineTo({ bounds.x + bounds.width, bounds.y + bounds.height / 2 });
        ctx.setStrokeStyle(StrokeStyle {
            .color = Colors::gray,
            .width = 4.0f,
            .cap = LineCap::Round
        });
        ctx.drawPath(remainingPath, false, true);

        // Draw thumb
        Path thumbPath;
        thumbPath.circle({ bounds.x + bounds.width * this->value, bounds.y + bounds.height / 2 }, 6);
        ctx.setFillColor(Colors::blue);
        ctx.drawPath(thumbPath, true, false);
    }
};

} // namespace flux