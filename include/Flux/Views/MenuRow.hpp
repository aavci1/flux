#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Views/StackLayout.hpp>
#include <Flux/Graphics/RenderContext.hpp>

namespace flux {

/**
 * Horizontal row for menu overlays: same layout as HStack, with themeable selection and hover fills.
 * Keep `backgroundColor` transparent; row fill is drawn in `render()`.
 */
struct MenuRow {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;

    Property<std::vector<View>> children = {};
    Property<float> spacing = 0;
    Property<JustifyContent> justifyContent = JustifyContent::start;
    Property<AlignItems> alignItems = AlignItems::stretch;
    Property<bool> selected = false;
    /** When false, hover highlight is suppressed (e.g. disabled items). */
    Property<bool> enabled = true;
    Property<Color> rowBackground = Colors::inherit;
    Property<Color> selectionAccent = Colors::inherit;
    Property<Color> hoverBackground = Colors::inherit;

    LayoutNode layout(RenderContext& ctx, const Rect& bounds) {
        std::vector<View> childrenVec = children;
        auto result = layoutStack<StackAxis::Horizontal>(
            childrenVec, spacing, justifyContent, alignItems, padding, bounds, ctx
        );
        return LayoutNode(View(*this), bounds, std::move(result.childLayouts));
    }

    Size preferredSize(TextMeasurement& textMeasurer) const {
        std::vector<View> childrenVec = children;
        return stackPreferredSize<StackAxis::Horizontal>(childrenVec, spacing, padding, textMeasurer);
    }

    float heightForWidth(float width, TextMeasurement& textMeasurer) const {
        std::vector<View> childrenVec = children;
        return stackHeightForWidth<StackAxis::Horizontal>(childrenVec, width, spacing, padding, textMeasurer);
    }

    void render(RenderContext& ctx, const Rect& bounds) const {
        const Theme& th = ctx.theme();
        Color base = resolveColor(rowBackground, th.controlBackground);
        Color accent = resolveColor(selectionAccent, th.accent);
        /** Same tint as the non-hover selected row; hover on other rows is derived a bit lighter for contrast. */
        Color selectedRef = lerpColor(base, accent, 0.15f);

        bool sel = static_cast<bool>(selected);
        bool rowEnabled = static_cast<bool>(enabled);
        bool hovered = rowEnabled && ctx.isCurrentViewHovered();

        Color fill = base;
        if (sel) {
            float t = hovered ? 0.18f : 0.15f;
            fill = lerpColor(base, accent, t);
        } else if (hovered) {
            Color hb = static_cast<Color>(hoverBackground);
            if (hb.isInherit()) {
                fill = selectedRef.lighten(0.08f);
            } else {
                fill = resolveColor(hoverBackground, th.menuItemHoverBackground);
            }
        }

        CornerRadius cr = static_cast<CornerRadius>(cornerRadius);
        ctx.setFillStyle(FillStyle::solid(fill));
        ctx.setStrokeStyle(StrokeStyle::none());
        ctx.drawRect(bounds, cr);
    }
};

} // namespace flux
