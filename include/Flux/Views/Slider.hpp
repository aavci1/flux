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
        ctx.beginPath();
        ctx.moveTo({ bounds.x, bounds.y + bounds.height / 2 });
        ctx.lineTo({ bounds.x + bounds.width * this->value, bounds.y + bounds.height / 2 });
        ctx.setStrokeStyle(StrokeStyle {
            .color = Colors::blue,
            .width = 4.0f,
            .cap = LineCap::Round
        });
        ctx.stroke();

        // Draw remaining line
        ctx.beginPath();
        ctx.moveTo({ bounds.x + bounds.width * this->value, bounds.y + bounds.height / 2 });
        ctx.lineTo({ bounds.x + bounds.width, bounds.y + bounds.height / 2 });
        ctx.setStrokeStyle(StrokeStyle {
            .color = Colors::gray,
            .width = 4.0f,
            .cap = LineCap::Round
        });
        ctx.stroke();

        // Draw thumb
        ctx.drawCircle({ bounds.x + bounds.width * this->value, bounds.y + bounds.height / 2 }, 6, Colors::blue);
    }
};

} // namespace flux