#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>

namespace flux {

struct Spacer {
    FLUX_VIEW_PROPERTIES;

    void init() {
        expansionBias = 1.0f;
        compressionBias = 1.0f;
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        return {0, 0};
    }
};

} // namespace flux
