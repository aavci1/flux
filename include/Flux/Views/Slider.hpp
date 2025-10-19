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
        ctx.setFillStyle(FillStyle::none());
        ctx.setStrokeStyle(StrokeStyle {
            .color = Colors::blue,
            .width = 4.0f,
            .cap = LineCap::Round
        });
        ctx.drawLine({ bounds.x, bounds.y + bounds.height / 2 }, { bounds.x + bounds.width * this->value, bounds.y + bounds.height / 2 });

        // Draw remaining line
        ctx.setFillStyle(FillStyle::none());
        ctx.setStrokeStyle(StrokeStyle {
            .color = Colors::gray,
            .width = 4.0f,
            .cap = LineCap::Round
        });
        ctx.drawLine({ bounds.x + bounds.width * this->value, bounds.y + bounds.height / 2 }, { bounds.x + bounds.width, bounds.y + bounds.height / 2 });

        // Draw thumb
        ctx.setFillStyle(FillStyle::solid(Colors::blue));
        ctx.setStrokeStyle(StrokeStyle::none());
        ctx.drawCircle({ bounds.x + bounds.width * this->value, bounds.y + bounds.height / 2 }, 6);
    }
};

} // namespace flux