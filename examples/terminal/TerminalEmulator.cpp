#include "TerminalEmulator.hpp"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <string_view>

namespace flux::term {

// ---------------------------------------------------------------------------
// ColorPalette
// ---------------------------------------------------------------------------

ColorPalette::ColorPalette() {
    // 0–7: standard ANSI colors
    colors_[0]  = Color::rgb(0, 0, 0);
    colors_[1]  = Color::rgb(205, 0, 0);
    colors_[2]  = Color::rgb(0, 205, 0);
    colors_[3]  = Color::rgb(205, 205, 0);
    colors_[4]  = Color::rgb(0, 0, 238);
    colors_[5]  = Color::rgb(205, 0, 205);
    colors_[6]  = Color::rgb(0, 205, 205);
    colors_[7]  = Color::rgb(229, 229, 229);
    // 8–15: bright
    colors_[8]  = Color::rgb(127, 127, 127);
    colors_[9]  = Color::rgb(255, 0, 0);
    colors_[10] = Color::rgb(0, 255, 0);
    colors_[11] = Color::rgb(255, 255, 0);
    colors_[12] = Color::rgb(92, 92, 255);
    colors_[13] = Color::rgb(255, 0, 255);
    colors_[14] = Color::rgb(0, 255, 255);
    colors_[15] = Color::rgb(255, 255, 255);
    // 16–231: 6×6×6 color cube
    for (int i = 0; i < 216; ++i) {
        int r = i / 36, g2 = (i % 36) / 6, b = i % 6;
        auto ramp = [](int x) -> uint8_t { return x == 0 ? 0 : static_cast<uint8_t>(55 + x * 40); };
        colors_[16 + i] = Color::rgb(ramp(r), ramp(g2), ramp(b));
    }
    // 232–255: greyscale ramp
    for (int i = 0; i < 24; ++i) {
        uint8_t v = static_cast<uint8_t>(8 + i * 10);
        colors_[232 + i] = Color::rgb(v, v, v);
    }
    colors_[kDefaultFg] = Color::rgb(230, 230, 230);
    colors_[kDefaultBg] = Color::rgb(30, 30, 30);
}

uint16_t ColorPalette::match(const Color& c) const {
    for (uint16_t i = 0; i < kSize; ++i) {
        if (colors_[i] == c) return i;
    }
    return kDefaultFg;
}

const ColorPalette& globalPalette() {
    static const ColorPalette pal;
    return pal;
}

// ---------------------------------------------------------------------------
// TerminalEmulator
// ---------------------------------------------------------------------------

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
            if (cur < 0) cur = 0;
            cur = cur * 10 + (ch - '0');
        }
    }
    out.push_back(cur < 0 ? 0 : cur);
    return out;
}

bool lineIsEmpty(const std::vector<Cell>& line) {
    for (const Cell& c : line) {
        if (!c.empty()) return false;
    }
    return true;
}

} // namespace

Cell TerminalEmulator::defaultCell() const {
    Cell c{};
    c.fgIdx = defaultFgIdx_;
    c.bgIdx = defaultBgIdx_;
    c.attrs = 0;
    return c;
}

void TerminalEmulator::growLineTo(std::vector<Cell>& line, int minCols) const {
    if (minCols <= 0) return;
    if (static_cast<int>(line.size()) < minCols) {
        line.resize(static_cast<std::size_t>(minCols), defaultCell());
    }
}

void TerminalEmulator::trimTrailingEmpty(std::vector<Cell>& line) const {
    while (!line.empty() && line.back().empty()
           && line.back().fgIdx == defaultFgIdx_
           && line.back().bgIdx == defaultBgIdx_
           && line.back().attrs == 0) {
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
    if (cols < 1) cols = 80;
    if (rows < 1) rows = 24;
    if (cols == cols_ && rows == rows_ && !lines_.empty()) return;

    const std::size_t absLine = screenStart_ + static_cast<std::size_t>(cursorRow_);
    cols_ = cols;
    rows_ = rows;

    if (lines_.empty()) {
        lines_.emplace_back();
    }

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
    curFgIdx_ = defaultFgIdx_;
    curBgIdx_ = defaultBgIdx_;
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
    if (utf8.empty()) return;
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
    cell.setGrapheme(utf8);
    cell.fgIdx = curFgIdx_;
    cell.bgIdx = curBgIdx_;
    cell.setBold(bold_);
    line[static_cast<std::size_t>(cursorCol_)] = cell;
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

uint16_t TerminalEmulator::ansi16Index(int code, bool fg) {
    // Standard ANSI: codes 0–7, bright 8–15.
    // fg: 30–37 (normal), 90–97 (bright)  →  palette 0–7, 8–15
    // bg: 40–47 (normal), 100–107 (bright) →  palette 0–7, 8–15
    (void)fg;
    int c = code & 7;
    bool bright = (code >= 8);
    return static_cast<uint16_t>(bright ? 8 + c : c);
}

uint16_t TerminalEmulator::xterm256Index(int i) {
    if (i < 0) i = 0;
    if (i > 255) i = 255;
    return static_cast<uint16_t>(i);
}

void TerminalEmulator::applySgr(const std::vector<int>& params, std::size_t& i) {
    if (i >= params.size()) return;
    int code = params[i++];
    switch (code) {
    case 0:
        curFgIdx_ = defaultFgIdx_;
        curBgIdx_ = defaultBgIdx_;
        bold_ = false;
        break;
    case 1:
        bold_ = true;
        break;
    case 22:
        bold_ = false;
        break;
    case 39:
        curFgIdx_ = defaultFgIdx_;
        break;
    case 49:
        curBgIdx_ = defaultBgIdx_;
        break;
    default:
        if (code >= 30 && code <= 37) {
            curFgIdx_ = ansi16Index(code - 30, true);
        } else if (code >= 90 && code <= 97) {
            curFgIdx_ = ansi16Index((code - 90) + 8, true);
        } else if (code >= 40 && code <= 47) {
            curBgIdx_ = ansi16Index(code - 40, false);
        } else if (code >= 100 && code <= 107) {
            curBgIdx_ = ansi16Index((code - 100) + 8, false);
        } else if (code == 38 && i + 1 < params.size() && params[i] == 5) {
            curFgIdx_ = xterm256Index(params[i + 1]);
            i += 2;
        } else if (code == 48 && i + 1 < params.size() && params[i] == 5) {
            curBgIdx_ = xterm256Index(params[i + 1]);
            i += 2;
        }
        break;
    }
}

void TerminalEmulator::handleCsi(const std::vector<int>& params, char finalCh) {
    ensureScreen();
    auto p0 = [&](std::size_t idx) -> int {
        if (params.empty()) return 0;
        int v = params[idx];
        return v == 0 ? 1 : v;
    };

    switch (finalCh) {
    case 'm': {
        if (params.empty() || (params.size() == 1 && params[0] == 0)) {
            curFgIdx_ = defaultFgIdx_;
            curBgIdx_ = defaultBgIdx_;
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
        if (row < 1) row = 1;
        if (col < 1) col = 1;
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
        if (col < 1) col = 1;
        cursorCol_ = std::min(cols_ - 1, col - 1);
        break;
    }
    case 'd': {
        int row = params.empty() ? 1 : params[0];
        if (row < 1) row = 1;
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
            auto& c = line[static_cast<std::size_t>(cursorCol_ + k)];
            c = Cell{};
            c.setGrapheme(" ");
            c.fgIdx = curFgIdx_;
            c.bgIdx = curBgIdx_;
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
                auto& cell = line[static_cast<std::size_t>(c)];
                cell = Cell{};
                cell.fgIdx = defaultFgIdx_;
                cell.bgIdx = defaultBgIdx_;
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
            auto& c = line[static_cast<std::size_t>(cursorCol_ + k)];
            c = Cell{};
            c.fgIdx = curFgIdx_;
            c.bgIdx = curBgIdx_;
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
        if (b == 0x07) continue;        // bell
        if (b == 0x08) {                 // backspace
            if (cursorCol_ > 0) --cursorCol_;
            continue;
        }
        if (b == 0x09) {                 // tab
            int next = ((cursorCol_ / 8) + 1) * 8;
            if (next >= cols_) next = cols_ - 1;
            cursorCol_ = next;
            continue;
        }
        if (b == 0x0d) {                 // cr
            cursorCol_ = 0;
            continue;
        }
        if (b == 0x0a || b == 0x0b || b == 0x0c) {
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
            if ((b & 0xE0) == 0xC0)      utf8Remaining_ = 1;
            else if ((b & 0xF0) == 0xE0) utf8Remaining_ = 2;
            else if ((b & 0xF8) == 0xF0) utf8Remaining_ = 3;
            else continue;
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
    const std::size_t logicalEnd = screenStart_ + static_cast<std::size_t>(rows_);
    while (s.lines.size() < logicalEnd) {
        s.lines.emplace_back();
    }
    s.totalLineCount = s.lines.size();
    s.lineOffset = 0;
    return s;
}

TermSnapshot TerminalEmulator::snapshotRange(std::size_t fromLine, std::size_t count) const {
    std::lock_guard lock(mutex_);

    const std::size_t logicalEnd = screenStart_ + static_cast<std::size_t>(rows_);
    const std::size_t totalLogical = std::max(lines_.size(), logicalEnd);

    TermSnapshot s;
    s.screenStart = screenStart_;
    s.cursorRow = cursorRow_;
    s.cursorCol = cursorCol_;
    s.cols = cols_;
    s.rows = rows_;
    s.totalLineCount = totalLogical;
    s.lineOffset = fromLine;

    if (fromLine >= totalLogical) return s;

    const std::size_t end = std::min(fromLine + count, totalLogical);
    s.lines.reserve(end - fromLine);

    for (std::size_t i = fromLine; i < end; ++i) {
        if (i < lines_.size()) {
            s.lines.push_back(lines_[i]);
        } else {
            s.lines.emplace_back();
        }
    }

    return s;
}

std::size_t TerminalEmulator::totalLines() const {
    std::lock_guard lock(mutex_);
    const std::size_t logicalEnd = screenStart_ + static_cast<std::size_t>(rows_);
    return std::max(lines_.size(), logicalEnd);
}

} // namespace flux::term
