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
        hide(id);
        entries_.push_back({id, std::move(content), anchor, std::move(config), {}, nullptr});
    }

    void hide(const std::string& id) {
        entries_.erase(
            std::remove_if(entries_.begin(), entries_.end(),
                [&](const OverlayEntry& e) { return e.id == id; }),
            entries_.end()
        );
    }

    void hideAll() {
        entries_.clear();
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
    Rect computeBounds(OverlayEntry& entry, RenderContext& ctx, const Rect& viewport) {
        if (entry.config.position == OverlayPosition::Center) {
            return viewport;
        }

        Size pref = entry.content.preferredSize(ctx);
        float w = pref.width > 0 ? pref.width : entry.anchor.width;
        float h = pref.height > 0 ? pref.height : 200.0f;

        float x = entry.anchor.x;
        float y;

        if (entry.config.position == OverlayPosition::Below) {
            y = entry.anchor.y + entry.anchor.height + 2.0f;
            if (y + h > viewport.y + viewport.height) {
                y = entry.anchor.y - h - 2.0f;
            }
        } else {
            y = entry.anchor.y - h - 2.0f;
            if (y < viewport.y) {
                y = entry.anchor.y + entry.anchor.height + 2.0f;
            }
        }

        if (x + w > viewport.x + viewport.width) {
            x = viewport.x + viewport.width - w;
        }
        if (x < viewport.x) x = viewport.x;

        return {x, y, w, h};
    }

    std::vector<OverlayEntry> entries_;
};

void showOverlay(const std::string& id, View content, Rect anchor, OverlayConfig config = {});
void hideOverlay(const std::string& id);
void hideAllOverlays();

} // namespace flux
