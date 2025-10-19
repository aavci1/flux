#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <functional>
#include <string>

namespace flux {

struct Button {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> text;
    std::function<void()> onClick;

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        ctx.setFillStyle(FillStyle::solid(Colors::blue));
        ctx.drawRect(bounds);

        ctx.setFillStyle(FillStyle::solid(Colors::white));
        ctx.drawText(static_cast<std::string>(text), bounds.center(), HorizontalAlignment::center, VerticalAlignment::center);
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        EdgeInsets paddingVal = padding;
        if (paddingVal.horizontal() == 0) {
            paddingVal = EdgeInsets(12, 24); // Default button padding
        }

        std::string buttonText = text;

        // Use accurate measurement from renderer
        Size textSize = textMeasurer.measureText(buttonText, TextStyle::regular("default", 16));
        return {textSize.width + paddingVal.horizontal(),
                textSize.height + paddingVal.vertical()};
    }
};

} // namespace flux
