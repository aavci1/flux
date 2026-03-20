#include "TerminalEmulator.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <string_view>

namespace flux::term {

namespace {

std::vector<int> parseSemicolonParams(std::string_view s) {
    std::vector<int> out;
    if (s.empty()) {
        return {0};
    }
    int cur = -1;
    for (char ch : s) {
        if (ch == ';') {
            out.push_back(cur < 0 ? 0 : cur);
            cur = -1;
        } else if (ch >= '0' && ch <= '9') {
            if (cur < 0) {
                cur = 0;
            }
            cur = cur * 10 + (ch - '0');
        }
    }
    out.push_back(cur < 0 ? 0 : cur);
    return out;
}

bool lineIsEmpty(const std::vector<Cell>& line) {
    for (const Cell& c : line) {
        if (!c.g.empty()) {
            return false;
        }
    }
    return true;
}

} // namespace

Cell TerminalEmulator::defaultCell() const {
    Cell c{};
    c.fg = defaultFg_;
    c.bg = defaultBg_;
    c.bold = false;
    return c;
}

void TerminalEmulator::growLineTo(std::vector<Cell>& line, int minCols) const {
    if (minCols <= 0) {
        return;
    }
    if (static_cast<int>(line.size()) < minCols) {
        line.resize(static_cast<std::size_t>(minCols), defaultCell());
    }
}

void TerminalEmulator::trimTrailingEmpty(std::vector<Cell>& line) const {
    // Only drop cells that are implicit defaults (no grapheme + default attrs). Keeps insert-blank
    // (CSI @) space characters and styled empty cells from being stripped.
    while (!line.empty() && line.back().g.empty() && line.back().fg == defaultFg_ && line.back().bg == defaultBg_
           && !line.back().bold) {
        line.pop_back();
    }
}

void TerminalEmulator::setCellDefault(std::vector<Cell>& line, int c) const {
    if (c >= 0 && c < static_cast<int>(line.size())) {
        line[static_cast<std::size_t>(c)] = defaultCell();
    }
}

TerminalEmulator::TerminalEmulator(int cols, int rows) {
    resize(cols, rows);
}

void TerminalEmulator::resize(int cols, int rows) {
    std::lock_guard lock(mutex_);
    if (cols < 1) {
        cols = 80;
    }
    if (rows < 1) {
        rows = 24;
    }
    if (cols == cols_ && rows == rows_ && !lines_.empty()) {
        return;
    }

    const std::size_t absLine = screenStart_ + static_cast<std::size_t>(cursorRow_);

    cols_ = cols;
    rows_ = rows;

    // Do not pad lines to cols_ here: cells are allocated only when written (sparse lines).

    if (lines_.empty()) {
        lines_.emplace_back();
    }

    // Do not pad lines_.size() to rows_: that creates trailing empty rows that become "the screen"
    // when we shrink (screenStart = size - rows), hiding the prompt and confusing the shell into
    // redrawing prompts. Anchor the viewport to the cursor line instead of the buffer tail.
    std::size_t maxStart = 0;
    if (lines_.size() >= static_cast<std::size_t>(rows_)) {
        maxStart = lines_.size() - static_cast<std::size_t>(rows_);
    }
    const std::size_t lo = (absLine + 1 > static_cast<std::size_t>(rows_))
        ? absLine - static_cast<std::size_t>(rows_) + 1
        : 0;
    screenStart_ = std::min(maxStart, lo);

    if (screenStart_ > kMaxScrollback) {
        std::size_t excess = screenStart_ - kMaxScrollback;
        lines_.erase(lines_.begin(), lines_.begin() + static_cast<std::ptrdiff_t>(excess));
        screenStart_ -= excess;
    }

    while (lines_.size() > screenStart_ + static_cast<std::size_t>(rows_) && lineIsEmpty(lines_.back())) {
        lines_.pop_back();
    }

    const std::ptrdiff_t dr = static_cast<std::ptrdiff_t>(absLine) - static_cast<std::ptrdiff_t>(screenStart_);
    cursorRow_ = std::clamp(static_cast<int>(dr), 0, rows_ - 1);
    cursorCol_ = std::clamp(cursorCol_, 0, cols_ - 1);
}

std::size_t TerminalEmulator::scrollbackLines() const {
    std::lock_guard lock(mutex_);
    return screenStart_;
}

void TerminalEmulator::ensureScreen() {
    std::size_t needed = screenStart_ + static_cast<std::size_t>(rows_);
    while (lines_.size() < needed) {
        lines_.emplace_back();
    }
}

void TerminalEmulator::ensureForCursor() {
    std::size_t needed = screenStart_ + static_cast<std::size_t>(cursorRow_) + 1;
    while (lines_.size() < needed) {
        lines_.emplace_back();
    }
}

void TerminalEmulator::clearScreen() {
    ensureScreen();
    for (int r = 0; r < rows_; ++r) {
        screenRow(r).clear();
    }
    curFg_ = defaultFg_;
    curBg_ = defaultBg_;
    bold_ = false;
    cursorRow_ = 0;
    cursorCol_ = 0;
}

void TerminalEmulator::scrollUp() {
    ensureScreen();
    ++screenStart_;
    lines_.emplace_back();
    if (screenStart_ > kMaxScrollback) {
        lines_.erase(lines_.begin());
        --screenStart_;
    }
}

void TerminalEmulator::putUtf8(std::string utf8) {
    if (utf8.empty()) {
        return;
    }
    ensureForCursor();
    if (cursorCol_ >= cols_) {
        cursorCol_ = 0;
        ++cursorRow_;
        if (cursorRow_ >= rows_) {
            cursorRow_ = rows_ - 1;
            scrollUp();
        }
    }
    auto& line = screenRow(cursorRow_);
    growLineTo(line, cursorCol_ + 1);
    Cell cell{};
    cell.g = std::move(utf8);
    cell.fg = curFg_;
    cell.bg = curBg_;
    cell.bold = bold_;
    line[static_cast<std::size_t>(cursorCol_)] = std::move(cell);
    ++cursorCol_;
    if (cursorCol_ >= cols_) {
        cursorCol_ = 0;
        ++cursorRow_;
        if (cursorRow_ >= rows_) {
            cursorRow_ = rows_ - 1;
            scrollUp();
        }
    }
}

Color TerminalEmulator::ansi16(int code, bool fg) const {
    static const Color fgTable[8] = {
        Color::rgb(0, 0, 0),
        Color::rgb(205, 0, 0),
        Color::rgb(0, 205, 0),
        Color::rgb(205, 205, 0),
        Color::rgb(0, 0, 238),
        Color::rgb(205, 0, 205),
        Color::rgb(0, 205, 205),
        Color::rgb(229, 229, 229),
    };
    static const Color fgBright[8] = {
        Color::rgb(127, 127, 127),
        Color::rgb(255, 0, 0),
        Color::rgb(0, 255, 0),
        Color::rgb(255, 255, 0),
        Color::rgb(92, 92, 255),
        Color::rgb(255, 0, 255),
        Color::rgb(0, 255, 255),
        Color::rgb(255, 255, 255),
    };
    int c = code & 7;
    bool bright = (code >= 8);
    if (fg) {
        return bright ? fgBright[c] : fgTable[c];
    }
    // Background: codes 40-47, 100-107
    static const Color bgTable[8] = {
        Color::rgb(0, 0, 0),
        Color::rgb(205, 0, 0),
        Color::rgb(0, 205, 0),
        Color::rgb(205, 205, 0),
        Color::rgb(0, 0, 238),
        Color::rgb(205, 0, 205),
        Color::rgb(0, 205, 205),
        Color::rgb(229, 229, 229),
    };
    static const Color bgBright[8] = {
        Color::rgb(85, 85, 85),
        Color::rgb(255, 85, 85),
        Color::rgb(85, 255, 85),
        Color::rgb(255, 255, 85),
        Color::rgb(85, 85, 255),
        Color::rgb(255, 85, 255),
        Color::rgb(85, 255, 255),
        Color::rgb(255, 255, 255),
    };
    return bright ? bgBright[c] : bgTable[c];
}

Color TerminalEmulator::xterm256(int i) {
    if (i < 0) {
        i = 0;
    }
    if (i < 16) {
        static const Color base16[16] = {
            Color::rgb(0, 0, 0),
            Color::rgb(205, 0, 0),
            Color::rgb(0, 205, 0),
            Color::rgb(205, 205, 0),
            Color::rgb(0, 0, 238),
            Color::rgb(205, 0, 205),
            Color::rgb(0, 205, 205),
            Color::rgb(229, 229, 229),
            Color::rgb(127, 127, 127),
            Color::rgb(255, 0, 0),
            Color::rgb(0, 255, 0),
            Color::rgb(255, 255, 0),
            Color::rgb(92, 92, 255),
            Color::rgb(255, 0, 255),
            Color::rgb(0, 255, 255),
            Color::rgb(255, 255, 255),
        };
        return base16[static_cast<std::size_t>(i % 16)];
    }
    if (i < 232) {
        i -= 16;
        int r = i / 36;
        int g = (i % 36) / 6;
        int b = i % 6;
        auto ramp = [](int x) -> int {
            return x == 0 ? 0 : 55 + x * 40;
        };
        return Color::rgb(static_cast<uint8_t>(ramp(r)), static_cast<uint8_t>(ramp(g)), static_cast<uint8_t>(ramp(b)));
    }
    int g = i - 232;
    g = std::clamp(g, 0, 23);
    uint8_t v = static_cast<uint8_t>(8 + g * 10);
    return Color::rgb(v, v, v);
}

void TerminalEmulator::applySgr(const std::vector<int>& params, std::size_t& i) {
    if (i >= params.size()) {
        return;
    }
    int code = params[i++];
    switch (code) {
    case 0:
        curFg_ = defaultFg_;
        curBg_ = defaultBg_;
        bold_ = false;
        break;
    case 1:
        bold_ = true;
        break;
    case 22:
        bold_ = false;
        break;
    case 39:
        curFg_ = defaultFg_;
        break;
    case 49:
        curBg_ = defaultBg_;
        break;
    default:
        if (code >= 30 && code <= 37) {
            curFg_ = ansi16(code - 30, true);
        } else if (code >= 90 && code <= 97) {
            curFg_ = ansi16((code - 90) + 8, true);
        } else if (code >= 40 && code <= 47) {
            curBg_ = ansi16(code - 40, false);
        } else if (code >= 100 && code <= 107) {
            curBg_ = ansi16((code - 100) + 8, false);
        } else if (code == 38 && i + 1 < params.size() && params[i] == 5) {
            curFg_ = xterm256(params[i + 1]);
            i += 2;
        } else if (code == 48 && i + 1 < params.size() && params[i] == 5) {
            curBg_ = xterm256(params[i + 1]);
            i += 2;
        }
        break;
    }
}

void TerminalEmulator::handleCsi(const std::vector<int>& params, char finalCh) {
    ensureScreen();
    auto p0 = [&](std::size_t idx) -> int {
        if (params.empty()) {
            return 0;
        }
        int v = params[idx];
        return v == 0 ? 1 : v;
    };

    switch (finalCh) {
    case 'm': {
        if (params.empty() || (params.size() == 1 && params[0] == 0)) {
            curFg_ = defaultFg_;
            curBg_ = defaultBg_;
            bold_ = false;
            break;
        }
        std::size_t i = 0;
        while (i < params.size()) {
            applySgr(params, i);
        }
        break;
    }
    case 'H':
    case 'f': {
        int row = params.size() > 0 ? params[0] : 1;
        int col = params.size() > 1 ? params[1] : 1;
        if (row < 1) {
            row = 1;
        }
        if (col < 1) {
            col = 1;
        }
        cursorRow_ = std::min(rows_ - 1, row - 1);
        cursorCol_ = std::min(cols_ - 1, col - 1);
        break;
    }
    case 'J': {
        int n = params.empty() ? 0 : params[0];
        if (n == 0) {
            for (int r = cursorRow_; r < rows_; ++r) {
                auto& line = screenRow(r);
                int c0 = (r == cursorRow_) ? cursorCol_ : 0;
                for (int c = c0; c < cols_; ++c) {
                    setCellDefault(line, c);
                }
                trimTrailingEmpty(line);
            }
        } else if (n == 1) {
            for (int r = 0; r <= cursorRow_; ++r) {
                auto& line = screenRow(r);
                int c1 = (r == cursorRow_) ? cursorCol_ : cols_ - 1;
                for (int c = 0; c <= c1; ++c) {
                    setCellDefault(line, c);
                }
                trimTrailingEmpty(line);
            }
        } else if (n == 2) {
            clearScreen();
        }
        break;
    }
    case 'K': {
        int n = params.empty() ? 0 : params[0];
        auto& line = screenRow(cursorRow_);
        if (n == 0) {
            for (int c = cursorCol_; c < cols_; ++c) {
                setCellDefault(line, c);
            }
            trimTrailingEmpty(line);
        } else if (n == 1) {
            for (int c = 0; c <= cursorCol_; ++c) {
                setCellDefault(line, c);
            }
            trimTrailingEmpty(line);
        } else if (n == 2) {
            line.clear();
        }
        break;
    }
    case 'A':
        cursorRow_ = std::max(0, cursorRow_ - p0(0));
        break;
    case 'B':
        cursorRow_ = std::min(rows_ - 1, cursorRow_ + p0(0));
        break;
    case 'C':
        cursorCol_ = std::min(cols_ - 1, cursorCol_ + p0(0));
        break;
    case 'D':
        cursorCol_ = std::max(0, cursorCol_ - p0(0));
        break;
    case 'G': {
        int col = params.empty() ? 1 : params[0];
        if (col < 1) {
            col = 1;
        }
        cursorCol_ = std::min(cols_ - 1, col - 1);
        break;
    }
    case 'd': {
        int row = params.empty() ? 1 : params[0];
        if (row < 1) {
            row = 1;
        }
        cursorRow_ = std::min(rows_ - 1, row - 1);
        break;
    }
    case '@': {
        int n = p0(0);
        auto& line = screenRow(cursorRow_);
        growLineTo(line, cols_);
        for (int k = cols_ - 1; k >= cursorCol_ + n && k >= 0; --k) {
            line[static_cast<std::size_t>(k)] = line[static_cast<std::size_t>(k - n)];
        }
        for (int k = 0; k < n && cursorCol_ + k < cols_; ++k) {
            line[static_cast<std::size_t>(cursorCol_ + k)] = Cell{};
            line[static_cast<std::size_t>(cursorCol_ + k)].g = " ";
            line[static_cast<std::size_t>(cursorCol_ + k)].fg = curFg_;
            line[static_cast<std::size_t>(cursorCol_ + k)].bg = curBg_;
        }
        trimTrailingEmpty(line);
        break;
    }
    case 'P': {
        int n = p0(0);
        auto& line = screenRow(cursorRow_);
        growLineTo(line, cols_);
        for (int c = cursorCol_; c + n < cols_; ++c) {
            line[static_cast<std::size_t>(c)] = line[static_cast<std::size_t>(c + n)];
        }
        for (int c = cols_ - n; c < cols_; ++c) {
            if (c >= cursorCol_) {
                line[static_cast<std::size_t>(c)] = Cell{};
                line[static_cast<std::size_t>(c)].fg = defaultFg_;
                line[static_cast<std::size_t>(c)].bg = defaultBg_;
            }
        }
        trimTrailingEmpty(line);
        break;
    }
    case 'X': {
        int n = p0(0);
        auto& line = screenRow(cursorRow_);
        growLineTo(line, std::min(cols_, cursorCol_ + n));
        for (int k = 0; k < n && cursorCol_ + k < cols_; ++k) {
            line[static_cast<std::size_t>(cursorCol_ + k)] = Cell{};
            line[static_cast<std::size_t>(cursorCol_ + k)].fg = curFg_;
            line[static_cast<std::size_t>(cursorCol_ + k)].bg = curBg_;
        }
        trimTrailingEmpty(line);
        break;
    }
    case 's':
        savedRow_ = cursorRow_;
        savedCol_ = cursorCol_;
        break;
    case 'u':
        cursorRow_ = std::clamp(savedRow_, 0, rows_ - 1);
        cursorCol_ = std::clamp(savedCol_, 0, cols_ - 1);
        break;
    default:
        break;
    }
}

void TerminalEmulator::feed(const char* data, std::size_t len) {
    std::lock_guard lock(mutex_);

    for (std::size_t i = 0; i < len; ++i) {
        unsigned char b = static_cast<unsigned char>(data[i]);

        if (state_ == ParseState::Utf8) {
            utf8Buf_.push_back(static_cast<char>(b));
            --utf8Remaining_;
            if (utf8Remaining_ <= 0) {
                putUtf8(std::move(utf8Buf_));
                utf8Buf_.clear();
                state_ = ParseState::Ground;
            }
            continue;
        }

        if (oscEscPending_) {
            oscEscPending_ = false;
            if (b == '\\') {
                state_ = ParseState::Ground;
                oscPayload_.clear();
            } else {
                oscPayload_.push_back('\x1b');
                oscPayload_.push_back(static_cast<char>(b));
                state_ = ParseState::Osc;
            }
            continue;
        }

        if (state_ == ParseState::Osc) {
            if (b == 0x07) {
                state_ = ParseState::Ground;
                oscPayload_.clear();
            } else if (b == 0x1b) {
                oscEscPending_ = true;
            } else {
                oscPayload_.push_back(static_cast<char>(b));
            }
            continue;
        }

        if (state_ == ParseState::Esc) {
            if (b == '[') {
                state_ = ParseState::Csi;
                csiParams_.clear();
                csiQuestion_ = false;
            } else if (b == ']') {
                state_ = ParseState::Osc;
                oscPayload_.clear();
                oscEscPending_ = false;
            } else if (b == '7') {
                savedRow_ = cursorRow_;
                savedCol_ = cursorCol_;
                state_ = ParseState::Ground;
            } else if (b == '8') {
                cursorRow_ = std::clamp(savedRow_, 0, rows_ - 1);
                cursorCol_ = std::clamp(savedCol_, 0, cols_ - 1);
                state_ = ParseState::Ground;
            } else if (b == 'M') {
                // Reverse index
                if (cursorRow_ == 0) {
                    scrollUp();
                } else {
                    --cursorRow_;
                }
                state_ = ParseState::Ground;
            } else if (b == 'c') {
                clearScreen();
                state_ = ParseState::Ground;
            } else {
                state_ = ParseState::Ground;
            }
            continue;
        }

        if (state_ == ParseState::Csi) {
            if (b == '?') {
                csiQuestion_ = true;
                continue;
            }
            if (b >= 0x40 && b <= 0x7e) {
                auto params = parseSemicolonParams(std::string_view(csiParams_));
                if (!csiQuestion_) {
                    handleCsi(params, static_cast<char>(b));
                }
                csiParams_.clear();
                csiQuestion_ = false;
                state_ = ParseState::Ground;
            } else {
                csiParams_.push_back(static_cast<char>(b));
            }
            continue;
        }

        // Ground
        if (b == 0x1b) {
            state_ = ParseState::Esc;
            continue;
        }
        if (b == 0x07) { // bell
            continue;
        }
        if (b == 0x08) { // backspace
            if (cursorCol_ > 0) {
                --cursorCol_;
            }
            continue;
        }
        if (b == 0x09) { // tab
            int next = ((cursorCol_ / 8) + 1) * 8;
            if (next >= cols_) {
                next = cols_ - 1;
            }
            cursorCol_ = next;
            continue;
        }
        if (b == 0x0d) { // cr
            cursorCol_ = 0;
            continue;
        }
        if (b == 0x0a || b == 0x0b || b == 0x0c) { // lf / vt / ff — Unix output is often \n without \r
            if (cursorRow_ + 1 >= rows_) {
                scrollUp();
            } else {
                ++cursorRow_;
                ensureForCursor();
            }
            cursorCol_ = 0;
            continue;
        }

        // UTF-8 lead
        if (b >= 0x80) {
            if ((b & 0xE0) == 0xC0) {
                utf8Remaining_ = 1;
            } else if ((b & 0xF0) == 0xE0) {
                utf8Remaining_ = 2;
            } else if ((b & 0xF8) == 0xF0) {
                utf8Remaining_ = 3;
            } else {
                continue; // invalid, skip
            }
            utf8Buf_.assign(1, static_cast<char>(b));
            state_ = ParseState::Utf8;
            continue;
        }

        if (b >= 0x20 && b != 0x7f) {
            putUtf8(std::string(1, static_cast<char>(b)));
        }
    }
}

TermSnapshot TerminalEmulator::snapshot() const {
    std::lock_guard lock(mutex_);
    TermSnapshot s;
    s.lines = lines_;
    s.screenStart = screenStart_;
    s.cursorRow = cursorRow_;
    s.cursorCol = cursorCol_;
    s.cols = cols_;
    s.rows = rows_;
    // Virtual screen rows below stored lines (lazy buffer): render full terminal height without
    // materializing empty rows in lines_.
    const std::size_t logicalEnd = screenStart_ + static_cast<std::size_t>(rows_);
    while (s.lines.size() < logicalEnd) {
        s.lines.emplace_back();
    }
    return s;
}

} // namespace flux::term
