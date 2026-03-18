#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Views/VStack.hpp>
#include <Flux/Views/Text.hpp>
#include <Flux/Views/Divider.hpp>
#include <string>
#include <algorithm>

namespace flux {

struct SectionHeader {
    FLUX_VIEW_PROPERTIES;

    Property<std::string> title = std::string("");
    Property<float> fontSize = 11.0f;
    Property<Color> color = Color(0.48f, 0.48f, 0.48f);
    Property<Color> dividerColor = Color(0.22f, 0.22f, 0.22f);
    Property<float> topPad = 20.0f;
    Property<float> bottomPad = 8.0f;
    Property<bool> showRule = true;

    View body() const {
        std::string titleStr = title;
        std::string upper;
        upper.reserve(titleStr.size());
        for (char c : titleStr) upper.push_back(static_cast<char>(std::toupper(c)));

        std::vector<View> children;
        children.push_back(Text{
            .value = upper,
            .fontSize = fontSize,
            .fontWeight = FontWeight::semibold,
            .color = color,
            .horizontalAlignment = HorizontalAlignment::leading
        });

        if (static_cast<bool>(showRule)) {
            children.push_back(Divider{
                .borderColor = dividerColor
            });
        }

        return VStack{
            .spacing = 4.0f,
            .padding = EdgeInsets(static_cast<float>(topPad), 0,
                                  static_cast<float>(bottomPad), 0),
            .children = std::move(children)
        };
    }
};

} // namespace flux
