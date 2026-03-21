#pragma once

#include "TerminalKeyBindings.hpp"
#include "TerminalSession.hpp"

#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Core/Typography.hpp>
#include <Flux/Core/View.hpp>
#include <Flux/Graphics/RenderContext.hpp>

#include <memory>
#include <string>

namespace flux::term {

struct TerminalView {
    FLUX_VIEW_PROPERTIES;
    FLUX_INTERACTIVE_PROPERTIES;

    std::shared_ptr<TerminalSession> session;
    /// Key bindings (zoom, scroll, PTY bytes). If null, session uses built-in defaults.
    std::shared_ptr<TerminalKeyBindings> keyBindings;

    Property<float> fontSize = Property<float>::shared(14.0f);
    /// Prefer "Monaco" on macOS (CoreText); Linux FontProvider still resolves a system sans path.
    Property<std::string> fontFamily = Property<std::string>::shared(std::string("Monaco"));

    /// Vertical scroll offset in px (content moves up as this increases).
    mutable float scrollY = 0.0f;
    mutable bool stickToBottom = true;
    mutable Rect lastViewport{};
    /// Last line height from layout (used for keyboard page scroll).
    mutable float cachedLineHeight = 18.0f;

    void init();

    LayoutNode layout(RenderContext& ctx, const Rect& bounds);

    void render(RenderContext& ctx, const Rect& bounds) const;

    Size preferredSize(TextMeasurement& /*tm*/) const {
        return Size{800.0f, 520.0f};
    }

    float heightForWidth(float width, TextMeasurement& /*tm*/) const {
        (void)width;
        return 480.0f;
    }

    bool handleKeyDown(const KeyEvent& event);
    bool handleTextInput(const TextInputEvent& event);
};

} // namespace flux::term
