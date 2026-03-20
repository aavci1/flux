#include "TerminalView.hpp"

#include <Flux/Core/EventTypes.hpp>
#include <Flux/Core/Property.hpp>
#include <Flux/Core/ViewHelpers.hpp>
#include <Flux/Graphics/Path.hpp>

#include <algorithm>

namespace flux::term {

namespace {

constexpr bool verticalRangesOverlap(float a0, float a1, float b0, float b1) {
    return a1 >= b0 && a0 <= b1;
}

} // namespace

void TerminalView::init() {
    clip = true;
    focusable = true;
    focusKey = "terminal";
    cursor = CursorType::Text;
    expansionBias = 1.0f;
    minHeight = 200.0f;
    padding = EdgeInsets(8.0f);
    backgroundColor = Color::rgb(0, 0, 0);

    onScroll = [this](float /*x*/, float /*y*/, float /*dx*/, float deltaY) {
        scrollY += deltaY;
        stickToBottom = false;
        requestApplicationRedraw();
    };
}

LayoutNode TerminalView::layout(RenderContext& ctx, const Rect& bounds) {
    // Use the full allocated rect for hit-testing, focus, and background (see render).
    // Padding is applied inside render/fitToSize so the gutter matches the terminal color.
    EdgeInsets pad = padding;
    if (session) {
        float fs = fontSize;
        std::string face = fontFamily;
        float cw = std::max(0.0f, bounds.width - pad.horizontal());
        float ch = std::max(0.0f, bounds.height - pad.vertical());
        session->fitToSize(cw, ch, ctx, fs, face);
        float lineH = fs * Typography::lineHeightTight;
        if (lineH < fs * 1.1f) {
            lineH = fs * 1.2f;
        }
        cachedLineHeight = lineH;
    }
    LayoutNode node(View(*this), bounds);
    node.environment = ctx.environment();
    return node;
}

void TerminalView::render(RenderContext& ctx, const Rect& bounds) const {
    ViewHelpers::renderView(*this, ctx, bounds);

    if (!session) {
        return;
    }

    const EdgeInsets pad = padding;
    const Rect content = {
        bounds.x + pad.left,
        bounds.y + pad.top,
        bounds.width - pad.horizontal(),
        bounds.height - pad.vertical(),
    };
    lastViewport = content;

    const float fs = fontSize;
    std::string face = fontFamily;
    if (face.empty()) {
        face = "Monaco";
    }
    const TextStyle baseStyle = makeTextStyle(face, FontWeight::semibold, fs, Typography::lineHeightTight, 0.0f);
    float cellW = ctx.measureText("M", baseStyle).width;
    cellW = cellW < 4.0f ? 8.0f : cellW;
    const float lineH = cachedLineHeight > 0.5f
        ? cachedLineHeight
        : (fs * Typography::lineHeightTight < fs * 1.1f ? fs * 1.2f : fs * Typography::lineHeightTight);

    // Full buffer height: scrollback lines + screen rows (snapshot pads virtual empty screen rows).
    auto snap = session->snapshot();
    const std::size_t totalLines = snap.lines.size();
    const float contentH = static_cast<float>(totalLines) * lineH;
    const float maxScroll = std::max(0.0f, contentH - content.height);
    if (!stickToBottom) {
        scrollY = std::clamp(scrollY, 0.0f, maxScroll);
        if (maxScroll > 0.0f && scrollY >= maxScroll - 1.0f) {
            stickToBottom = true;
        }
    }
    if (stickToBottom) {
        scrollY = maxScroll;
    }

    ctx.save();
    Path clipPath;
    clipPath.rect(content);
    ctx.clipPath(clipPath);

    const Color defaultBg = Color::rgb(30, 30, 30);
    const float viewTop = content.y;
    const float viewBottom = content.y + content.height;
    const float originY = viewTop - scrollY;

    for (std::size_t li = 0; li < totalLines; ++li) {
        const float lineTop = originY + static_cast<float>(li) * lineH;
        const float lineBottom = lineTop + lineH;
        if (!verticalRangesOverlap(lineTop, lineBottom, viewTop, viewBottom)) {
            continue;
        }
        if (li >= snap.lines.size()) {
            continue;
        }
        const std::vector<Cell>& line = snap.lines[li];

        // Sparse lines: columns past line.size() are implicit default cells (no storage).
        for (int col = 0; col < snap.cols; ++col) {
            const Cell cell =
                col < static_cast<int>(line.size()) ? line[static_cast<std::size_t>(col)] : Cell{};
            const float cx = content.x + static_cast<float>(col) * cellW;

            if (cell.bg != defaultBg) {
                ctx.setFillStyle(FillStyle::solid(cell.bg));
                ctx.setStrokeStyle(StrokeStyle::none());
                ctx.drawRect(Rect(cx, lineTop, cellW, lineH));
            }

            if (!cell.g.empty()) {
                TextStyle st = baseStyle;
                st.weight = cell.bold ? FontWeight::bold : FontWeight::semibold;
                ctx.setTextStyle(st);
                ctx.setFillStyle(FillStyle::solid(cell.fg));
                ctx.drawText(cell.g, {cx + 1.0f, lineBottom - 2.0f},
                    HorizontalAlignment::leading, VerticalAlignment::bottom);
            }
        }
    }

    if (ctx.isCurrentViewFocused()) {
        const std::size_t absLine = snap.screenStart + static_cast<std::size_t>(snap.cursorRow);
        const float caretTop = originY + static_cast<float>(absLine) * lineH;
        const float caretX = content.x + static_cast<float>(snap.cursorCol) * cellW;
        if (verticalRangesOverlap(caretTop, caretTop + lineH, viewTop, viewBottom)) {
            ctx.setFillStyle(FillStyle::solid(Color::rgb(200, 200, 200).opacity(0.85f)));
            ctx.setStrokeStyle(StrokeStyle::none());
            ctx.drawRect(Rect(caretX + 1.0f, caretTop + 1.0f, std::max(1.0f, cellW * 0.35f), lineH - 2.0f));
        }
    }

    ctx.restore();
}

bool TerminalView::handleKeyDown(const KeyEvent& event) {
    if (!session) {
        return false;
    }

    std::optional<TerminalViewAction> viewAction;
    const bool handled = session->handleKey(event, keyBindings ? keyBindings.get() : nullptr, &viewAction);

    if (handled && viewAction) {
        // Apply view action from binding (zoom, scroll)
        constexpr float kMinFont = 9.0f;
        constexpr float kMaxFont = 28.0f;
        constexpr float kDefaultFont = 14.0f;
        switch (*viewAction) {
        case TerminalViewAction::ZoomIn:
            fontSize = std::clamp(static_cast<float>(fontSize) + 1.0f, kMinFont, kMaxFont);
            requestApplicationRedraw();
            return true;
        case TerminalViewAction::ZoomOut:
            fontSize = std::clamp(static_cast<float>(fontSize) - 1.0f, kMinFont, kMaxFont);
            requestApplicationRedraw();
            return true;
        case TerminalViewAction::ZoomReset:
            fontSize = kDefaultFont;
            requestApplicationRedraw();
            return true;
        case TerminalViewAction::ScrollPageUp: {
            float viewH = lastViewport.height < 1.0f ? 400.0f : lastViewport.height;
            float lineH = cachedLineHeight > 0.5f ? cachedLineHeight : 16.0f;
            float pagePx = std::max(lineH, viewH - lineH);
            scrollY = std::max(0.0f, scrollY - pagePx);
            stickToBottom = false;
            requestApplicationRedraw();
            return true;
        }
        case TerminalViewAction::ScrollPageDown: {
            float viewH = lastViewport.height < 1.0f ? 400.0f : lastViewport.height;
            float lineH = cachedLineHeight > 0.5f ? cachedLineHeight : 16.0f;
            float pagePx = std::max(lineH, viewH - lineH);
            scrollY += pagePx;
            requestApplicationRedraw();
            return true;
        }
        case TerminalViewAction::ScrollHome:
            scrollY = 0.0f;
            stickToBottom = false;
            requestApplicationRedraw();
            return true;
        case TerminalViewAction::ScrollEnd:
            stickToBottom = true;
            requestApplicationRedraw();
            return true;
        default:
            break;
        }
    }
    return handled;
}

bool TerminalView::handleTextInput(const TextInputEvent& event) {
    if (!session || event.text.empty()) {
        return false;
    }
    // Cocoa sends Return as both keyDown(Enter) → \r and insertText("\r"). Consume the duplicate.
    if (event.text == "\r" || event.text == "\n") {
        return true;
    }
    return session->pasteText(event.text);
}

} // namespace flux::term
