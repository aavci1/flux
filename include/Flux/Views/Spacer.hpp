#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>

namespace flux {

struct Spacer {
    Property<EdgeInsets> margin = {};
    Property<EdgeInsets> padding = {};
    Property<Color> backgroundColor = Colors::transparent;
    Property<BackgroundImage> backgroundImage = BackgroundImage{};
    Property<Color> borderColor = Colors::transparent;
    Property<float> borderWidth = 0;
    Property<float> cornerRadius = 0;
    Property<float> opacity = 1.0;
    Property<bool> visible = true;
    Property<bool> clip = false;
    Property<float> rotation = 0;
    Property<float> scaleX = 1.0;
    Property<float> scaleY = 1.0;
    Property<Point> offset = Point{0, 0};
    Property<float> expansionBias = 1.0f;
    Property<float> compressionBias = 1.0f;

    Size preferredSize(TextMeasurement& textMeasurer) const {
        return {0, 0};
    }
};

} // namespace flux
