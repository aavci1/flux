#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>

namespace flux {

enum class DividerOrientation {
    Horizontal,
    Vertical
};

struct Divider {
    FLUX_VIEW_PROPERTIES;

    Property<DividerOrientation> orientation = DividerOrientation::Horizontal;
    Property<float> thickness = 1.0f;
    Property<Color> color = Colors::lightGray;

    void render(RenderContext& ctx, const Rect& bounds) const {
        ViewHelpers::renderView(*this, ctx, bounds);

        EdgeInsets paddingVal = padding;
        DividerOrientation orient = orientation;
        float thick = thickness;

        if (orient == DividerOrientation::Horizontal) {
            // Horizontal line across the width
            float lineY = bounds.y + paddingVal.top + (bounds.height - paddingVal.vertical() - thick) / 2;
            Rect lineRect = {
                bounds.x + paddingVal.left,
                lineY,
                bounds.width - paddingVal.horizontal(),
                thick
            };
            ctx.setFillStyle(FillStyle::solid(color));
            ctx.setStrokeStyle(StrokeStyle::none());
            ctx.drawRect(lineRect);
        } else {
            // Vertical line across the height
            float lineX = bounds.x + paddingVal.left + (bounds.width - paddingVal.horizontal() - thick) / 2;
            Rect lineRect = {
                lineX,
                bounds.y + paddingVal.top,
                thick,
                bounds.height - paddingVal.vertical()
            };
            ctx.setFillStyle(FillStyle::solid(color));
            ctx.setStrokeStyle(StrokeStyle::none());
            ctx.drawRect(lineRect);
        }
    }

    Size preferredSize(TextMeasurement& /* textMeasurer */) const {
        EdgeInsets paddingVal = padding;
        DividerOrientation orient = orientation;
        float thick = thickness;

        if (orient == DividerOrientation::Horizontal) {
            // Horizontal divider: full width, minimal height
            return {100.0f + paddingVal.horizontal(), thick + paddingVal.vertical()};
        } else {
            // Vertical divider: minimal width, full height
            return {thick + paddingVal.horizontal(), 100.0f + paddingVal.vertical()};
        }
    }
};

} // namespace flux

