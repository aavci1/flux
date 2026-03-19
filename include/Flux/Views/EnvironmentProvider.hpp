#pragma once

#include <Flux/Core/Environment.hpp>
#include <Flux/Core/View.hpp>

namespace flux {

/**
 * Overrides an environment value for all descendants (layout + render via RenderContext).
 */
template<typename Key>
struct EnvironmentProvider {
    FLUX_VIEW_PROPERTIES;

    typename Key::Value value = Key::defaultValue();
    View child;

    LayoutNode layout(RenderContext& ctx, const Rect& bounds) const {
        Environment merged = ctx.environment().template with<Key>(value);
        ctx.pushEnvironment(merged);

        EdgeInsets pad = padding;
        Rect contentBounds = {bounds.x + pad.left,
                              bounds.y + pad.top,
                              bounds.width - pad.horizontal(),
                              bounds.height - pad.vertical()};

        LayoutNode inner = child.layout(ctx, contentBounds);
        ctx.popEnvironment();

        std::vector<LayoutNode> kids;
        kids.push_back(std::move(inner));
        LayoutNode node(View(*this), bounds, std::move(kids));
        node.environment = merged;
        return node;
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        EdgeInsets pad = padding;
        Size inner = child.preferredSize(textMeasurer);
        return {inner.width + pad.horizontal(), inner.height + pad.vertical()};
    }

    float heightForWidth(float width, TextMeasurement& textMeasurer) const {
        EdgeInsets pad = padding;
        float innerW = width - pad.horizontal();
        return child.heightForWidth(innerW, textMeasurer) + pad.vertical();
    }
};

} // namespace flux
