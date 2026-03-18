#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Core/Types.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Views/VStack.hpp>
#include <algorithm>
#include <cmath>

namespace flux {

// Helper to create an invisible clipping container
struct ClipContainer {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;
    
    void init() {
        clip = true;
    }
    
    Size preferredSize(TextMeasurement&) const {
        return {0, 0};
    }
};

struct ScrollArea {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;

    // Content to scroll
    Property<std::vector<View>> children = {};
    
    // Scroll position
    Property<float> scrollX = 0.0f;
    Property<float> scrollY = 0.0f;
    
    // Content size - if not set, will be calculated from children
    Property<std::optional<Size>> contentSize = std::nullopt;
    
    // Internal state (mutable for use in const methods)
    mutable Size cachedContentSize;
    mutable Rect cachedViewportRect;

    void init() {
        // Handle scroll events
        onScroll = [this](float x, float y, float deltaX, float deltaY) {
            (void)x; (void)y;
            handleScroll(deltaX, deltaY);
        };
    }

    LayoutNode layout(RenderContext& ctx, const Rect& bounds) {
        std::vector<View> childrenVec = children;
        EdgeInsets paddingVal = padding;

        cachedViewportRect = bounds;

        float contentWidth = bounds.width - paddingVal.horizontal();

        Size contentSz;
        if (contentSize.get().has_value()) {
            contentSz = contentSize.get().value();
        } else {
            float totalHeight = paddingVal.vertical();
            for (const auto& child : childrenVec) {
                if (!child->isVisible()) continue;
                float childH = child.heightForWidth(contentWidth, static_cast<TextMeasurement&>(ctx));
                totalHeight += childH;
            }
            contentSz = Size(bounds.width, totalHeight);
        }

        cachedContentSize = contentSz;

        std::vector<LayoutNode> contentChildLayouts;

        float currentY = bounds.y + paddingVal.top - static_cast<float>(scrollY);
        float currentX = bounds.x + paddingVal.left - static_cast<float>(scrollX);

        for (auto& child : childrenVec) {
            if (!child->isVisible()) continue;

            float childH = child.heightForWidth(contentWidth, static_cast<TextMeasurement&>(ctx));
            Rect childBounds(currentX, currentY, contentWidth, childH);

            LayoutNode childLayout = child.layout(ctx, childBounds);
            contentChildLayouts.push_back(std::move(childLayout));

            currentY += childH;
        }

        float borderW = borderWidth;
        Rect clipRect = {
            bounds.x + borderW / 2,
            bounds.y + borderW / 2,
            bounds.width - borderW,
            bounds.height - borderW
        };

        ClipContainer clipper;
        clipper.clip = true;
        View clipperView(clipper);

        std::vector<LayoutNode> childLayouts;
        LayoutNode contentWrapper(clipperView, clipRect, std::move(contentChildLayouts));
        childLayouts.push_back(std::move(contentWrapper));

        return LayoutNode(View(*this), bounds, std::move(childLayouts));
    }

    Size preferredSize(TextMeasurement& /*textMeasurer*/) const {
        EdgeInsets paddingVal = padding;
        return Size(paddingVal.horizontal(), paddingVal.vertical());
    }

private:
    void handleScroll(float deltaX, float deltaY) {        
        // Update scroll position
        float newScrollY = static_cast<float>(scrollY) + deltaY;
        float newScrollX = static_cast<float>(scrollX) + deltaX;
        
        // Clamp to valid range
        EdgeInsets paddingVal = padding;
        Size viewportSize = Size(
            cachedViewportRect.width - paddingVal.horizontal(),
            cachedViewportRect.height - paddingVal.vertical()
        );
        
        float maxScrollY = std::max(0.0f, cachedContentSize.height - viewportSize.height);
        float maxScrollX = std::max(0.0f, cachedContentSize.width - viewportSize.width);
        
        scrollY = std::clamp(newScrollY, 0.0f, maxScrollY);
        scrollX = std::clamp(newScrollX, 0.0f, maxScrollX);
        
        if (onChange) {
            onChange();
        }
    }
};

} // namespace flux

