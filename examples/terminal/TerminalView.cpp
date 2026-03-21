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

const Cell defaultCell_{};

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

    const std::size_t totalLines = session->totalLines();
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

    const float viewTop = content.y;
    const float viewBottom = content.y + content.height;
    const float originY = viewTop - scrollY;

    // Only fetch lines that are actually visible — avoids copying the full scrollback.
    std::size_t firstVisible = 0;
    if (scrollY > 0.0f) {
        firstVisible = static_cast<std::size_t>(scrollY / lineH);
        if (firstVisible > 0) --firstVisible;
    }
    const std::size_t visibleCount =
        static_cast<std::size_t>(content.height / lineH) + 3;
    auto snap = session->snapshotRange(firstVisible,
                                       std::min(visibleCount, totalLines - firstVisible));

    ctx.save();
    Path clipPath;
    clipPath.rect(content);
    ctx.clipPath(clipPath);

    const Color defaultBg = globalPalette()[ColorPalette::kDefaultBg];

    for (std::size_t si = 0; si < snap.lines.size(); ++si) {
        const std::size_t li = snap.lineOffset + si;
        const float lineTop = originY + static_cast<float>(li) * lineH;
        const float lineBottom = lineTop + lineH;
        if (!verticalRangesOverlap(lineTop, lineBottom, viewTop, viewBottom)) {
            continue;
        }
        const std::vector<Cell>& line = snap.lines[si];
        const int lineSz = static_cast<int>(line.size());

        // --- Pass 1: batched background rects ---
        int bgStart = -1;
        Color bgColor{};
        for (int col = 0; col <= snap.cols; ++col) {
            Color thisBg = defaultBg;
            if (col < snap.cols) {
                const Cell& cell = col < lineSz ? line[static_cast<std::size_t>(col)] : defaultCell_;
                thisBg = cell.bg();
            }

            if (col < snap.cols && thisBg != defaultBg) {
                if (bgStart < 0 || thisBg != bgColor) {
                    if (bgStart >= 0) {
                        float bx = content.x + static_cast<float>(bgStart) * cellW;
                        float bw = static_cast<float>(col - bgStart) * cellW;
                        ctx.setFillStyle(FillStyle::solid(bgColor));
                        ctx.setStrokeStyle(StrokeStyle::none());
                        ctx.drawRect(Rect(bx, lineTop, bw, lineH));
                    }
                    bgStart = col;
                    bgColor = thisBg;
                }
            } else {
                if (bgStart >= 0) {
                    float bx = content.x + static_cast<float>(bgStart) * cellW;
                    float bw = static_cast<float>(col - bgStart) * cellW;
                    ctx.setFillStyle(FillStyle::solid(bgColor));
                    ctx.setStrokeStyle(StrokeStyle::none());
                    ctx.drawRect(Rect(bx, lineTop, bw, lineH));
                    bgStart = -1;
                }
            }
        }

        // --- Pass 2: text with style-change batching ---
        // Each character is positioned individually on the cell grid, but style
        // commands (setTextStyle/setFillStyle) are only emitted when they change.
        Color prevFg{};
        bool prevBold = false;
        bool styleSet = false;

        for (int col = 0; col < snap.cols; ++col) {
            const Cell& cell = col < lineSz ? line[static_cast<std::size_t>(col)] : defaultCell_;
            if (cell.empty()) continue;

            Color fg = cell.fg();
            bool bold = cell.bold();

            if (!styleSet || bold != prevBold) {
                TextStyle st = baseStyle;
                st.weight = bold ? FontWeight::bold : FontWeight::semibold;
                ctx.setTextStyle(st);
                prevBold = bold;
            }
            if (!styleSet || fg != prevFg) {
                ctx.setFillStyle(FillStyle::solid(fg));
                prevFg = fg;
            }
            styleSet = true;

            const float cx = content.x + static_cast<float>(col) * cellW;
            ctx.drawText(cell.grapheme(), {cx + 1.0f, lineBottom - 2.0f},
                         HorizontalAlignment::leading, VerticalAlignment::bottom);
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
