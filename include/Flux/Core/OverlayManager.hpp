#pragma once

#include <Flux/Core/View.hpp>
#include <Flux/Core/Element.hpp>
#include <Flux/Core/Types.hpp>
#include <functional>
#include <string>
#include <vector>
#include <algorithm>

namespace flux {

enum class OverlayPosition {
    Below,
    Above,
    Center
};

struct OverlayConfig {
    OverlayPosition position = OverlayPosition::Below;
    bool dismissOnClickOutside = true;
    bool modal = false;
    std::function<void()> onDismiss;
    Color backdrop = Colors::transparent;
};

struct OverlayEntry {
    std::string id;
    View content;
    Rect anchor;
    OverlayConfig config;

    LayoutNode layoutTree;
    std::unique_ptr<Element> element;
};

class OverlayManager {
public:
    void show(const std::string& id, View content, Rect anchor, OverlayConfig config = {}) {
        hideImmediate(id);
        entries_.push_back({id, std::move(content), anchor, std::move(config), {}, nullptr});
    }

    void hide(const std::string& id) {
        if (deferHideDepth_ > 0) {
            pendingHides_.push_back(id);
        } else {
            hideImmediate(id);
        }
    }

    void hideAll() {
        if (deferHideDepth_ > 0) {
            pendingHideAll_ = true;
        } else {
            entries_.clear();
        }
    }

    void beginDeferHide() { ++deferHideDepth_; }

    void endDeferHide() {
        if (deferHideDepth_ == 0) return;
        --deferHideDepth_;
        if (deferHideDepth_ == 0) {
            flushPendingHides();
        }
    }

    bool empty() const { return entries_.empty(); }
    size_t count() const { return entries_.size(); }

    void layoutOverlays(RenderContext& ctx, const Rect& viewport) {
        for (auto& entry : entries_) {
            Rect bounds = computeBounds(entry, ctx, viewport);
            suppressRedrawRequests();
            entry.layoutTree = entry.content.layout(ctx, bounds);
            if (!entry.element) {
                entry.element = Element::buildTree(entry.layoutTree);
            } else {
                entry.element->reconcile(entry.layoutTree);
            }
            resumeRedrawRequests();
        }
    }

    std::vector<OverlayEntry>& entries() { return entries_; }
    const std::vector<OverlayEntry>& entries() const { return entries_; }

private:
    void hideImmediate(const std::string& id) {
        entries_.erase(
            std::remove_if(entries_.begin(), entries_.end(),
                [&](const OverlayEntry& e) { return e.id == id; }),
            entries_.end()
        );
    }

    void flushPendingHides() {
        if (pendingHideAll_) {
            pendingHideAll_ = false;
            pendingHides_.clear();
            entries_.clear();
            return;
        }
        for (const std::string& id : pendingHides_) {
            hideImmediate(id);
        }
        pendingHides_.clear();
    }

    Rect computeBounds(OverlayEntry& entry, RenderContext& ctx, const Rect& viewport) {
        if (entry.config.position == OverlayPosition::Center) {
            return viewport;
        }

        Size pref = entry.content.preferredSize(ctx);
        float anchorW = entry.anchor.width;
        // Prefer the anchor width when known so popups (dropdowns, menus) match their trigger control.
        // Content preferred width alone can be smaller than the laid-out control (e.g. truncated labels).
        float w = (anchorW > 0.5f) ? anchorW : (pref.width > 0.0f ? pref.width : 200.0f);
        float h = pref.height > 0 ? pref.height : 200.0f;

        float x = entry.anchor.x;
        float y;

        constexpr float kMenuAnchorGap = 6.0f; // space between trigger and menu (e.g. below dropdown)

        if (entry.config.position == OverlayPosition::Below) {
            y = entry.anchor.y + entry.anchor.height + kMenuAnchorGap;
            if (y + h > viewport.y + viewport.height) {
                y = entry.anchor.y - h - kMenuAnchorGap;
            }
        } else {
            y = entry.anchor.y - h - kMenuAnchorGap;
            if (y < viewport.y) {
                y = entry.anchor.y + entry.anchor.height + kMenuAnchorGap;
            }
        }

        if (x + w > viewport.x + viewport.width) {
            x = viewport.x + viewport.width - w;
        }
        if (x < viewport.x) x = viewport.x;

        return {x, y, w, h};
    }

    std::vector<OverlayEntry> entries_;

    int deferHideDepth_ = 0;
    std::vector<std::string> pendingHides_;
    bool pendingHideAll_ = false;
};

void showOverlay(const std::string& id, View content, Rect anchor, OverlayConfig config = {});
/** Menu-style overlay: dismiss on outside click; wraps showOverlay. */
void showMenuOverlay(
    const std::string& id,
    View content,
    Rect anchor,
    OverlayPosition position = OverlayPosition::Below,
    std::function<void()> onDismiss = nullptr
);
void hideOverlay(const std::string& id);
void hideAllOverlays();

} // namespace flux
