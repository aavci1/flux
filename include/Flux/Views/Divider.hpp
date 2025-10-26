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

    void init() {
        borderWidth = 1.0f;
        borderColor = Colors::gray.opacity(0.2f);
        compressionBias = 0.0f;
        expansionBias = 0.0f;
    }

    Size preferredSize(TextMeasurement& /* textMeasurer */) const {
        EdgeInsets paddingVal = padding;
        DividerOrientation orient = orientation;

        if (orient == DividerOrientation::Horizontal) {
            return {paddingVal.horizontal(), 0.5f + paddingVal.vertical()};
        } else {
            return {0.5f + paddingVal.horizontal(), paddingVal.vertical()};
        }
    }
};

} // namespace flux

