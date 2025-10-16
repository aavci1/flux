#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Graphics/RenderContext.hpp>
#include <iostream>

namespace flux {

// ViewHelpers namespace for view rendering utilities
namespace ViewHelpers {

// Unified render function that handles decorations only
// Framework handles children rendering through layout tree
template<typename ViewType>
inline void renderView(const ViewType& view, RenderContext& ctx, const Rect& bounds) {
    ctx.save();

    // Apply transforms
    Point offsetPt = view.offset;
    ctx.translate(offsetPt.x, offsetPt.y);

    float rot = view.rotation;
    if (rot != 0) {
        ctx.rotate(rot);
    }

    float sx = view.scaleX, sy = view.scaleY;
    if (sx != 1.0f || sy != 1.0f) {
        ctx.scale(sx, sy);
    }

    float opacityVal = view.opacity;
    if (opacityVal < 1.0f) {
        ctx.setOpacity(opacityVal);
    }

    // Draw shadow if needed
    Shadow shadowVal = view.shadow;
    if (shadowVal.blurRadius > 0 || shadowVal.spreadRadius > 0) {
        ctx.drawShadow(bounds, static_cast<float>(view.cornerRadius), shadowVal);
    }

    // Draw background
    Color bgColor = view.backgroundColor;
    if (bgColor.a > 0) {
        ctx.drawRoundedRect(bounds, static_cast<float>(view.cornerRadius), bgColor);
    }

    // Draw border
    float borderWidth = view.borderWidth;
    Color borderColor = view.borderColor;
    if (borderWidth > 0 && borderColor.a > 0) {
        ctx.drawRoundedRectBorder(bounds, static_cast<float>(view.cornerRadius), borderColor, borderWidth);
    }

    ctx.restore();
}

} // namespace ViewHelpers

} // namespace flux

