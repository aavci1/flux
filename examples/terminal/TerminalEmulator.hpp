#pragma once

#include <Flux/Core/Types.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <mutex>
#include <string>
#include <vector>

namespace flux::term {

// ---------------------------------------------------------------------------
// ColorPalette — maps 8-bit indices to RGBA Color values.
// Indices 0–15 are the standard ANSI colors. 16–231 are the 6×6×6 cube.
// 232–255 are the greyscale ramp. Index 256 = default fg, 257 = default bg.
// ---------------------------------------------------------------------------
class ColorPalette {
public:
    static constexpr uint16_t kDefaultFg = 256;
    static constexpr uint16_t kDefaultBg = 257;
    static constexpr uint16_t kSize = 258;

    ColorPalette();
    Color operator[](uint16_t idx) const { return colors_[idx]; }
    uint16_t match(const Color& c) const;

private:
    std::array<Color, kSize> colors_;
};

const ColorPalette& globalPalette();

// ---------------------------------------------------------------------------
// Cell — compact per-character storage: 8 bytes instead of the former 64.
// ---------------------------------------------------------------------------
struct Cell {
    char g[4]{};
    uint8_t gLen{0};
    uint16_t fgIdx{ColorPalette::kDefaultFg};
    uint16_t bgIdx{ColorPalette::kDefaultBg};
    uint8_t attrs{0};

    static constexpr uint8_t kBold = 1;

    bool bold() const { return (attrs & kBold) != 0; }
    void setBold(bool b) { if (b) attrs |= kBold; else attrs &= ~kBold; }

    bool empty() const { return gLen == 0; }

    std::string grapheme() const { return std::string(g, gLen); }
    void setGrapheme(const std::string& s) {
        gLen = static_cast<uint8_t>(s.size() <= 4 ? s.size() : 4);
        std::memcpy(g, s.data(), gLen);
    }

    Color fg() const { return globalPalette()[fgIdx]; }
    Color bg() const { return globalPalette()[bgIdx]; }
};
static_assert(sizeof(Cell) <= 12, "Cell should be compact");

struct TermSnapshot {
    std::vector<std::vector<Cell>> lines;
    std::size_t screenStart{0};
    int cursorRow{0};
    int cursorCol{0};
    int cols{80};
    int rows{24};

    std::size_t totalLineCount{0};
    std::size_t lineOffset{0};
};

/// Minimal VT100 / xterm-style parser with scrollback (sufficient for common shells).
class TerminalEmulator {
public:
    explicit TerminalEmulator(int cols = 80, int rows = 24);

    void resize(int cols, int rows);
    void feed(const char* data, std::size_t len);

    [[nodiscard]] TermSnapshot snapshot() const;

    /// Return only lines in [fromLine, fromLine+count). Much cheaper than full snapshot().
    [[nodiscard]] TermSnapshot snapshotRange(std::size_t fromLine, std::size_t count) const;

    [[nodiscard]] int cols() const { return cols_; }
    [[nodiscard]] int rows() const { return rows_; }
    [[nodiscard]] std::size_t scrollbackLines() const;
    [[nodiscard]] std::size_t totalLines() const;

private:
    [[nodiscard]] Cell defaultCell() const;
    void growLineTo(std::vector<Cell>& line, int minCols) const;
    void trimTrailingEmpty(std::vector<Cell>& line) const;
    void setCellDefault(std::vector<Cell>& line, int c) const;

    void clearScreen();
    void ensureScreen();
    void ensureForCursor();
    void scrollUp();
    void putUtf8(std::string utf8);
    void handleCsi(const std::vector<int>& params, char finalCh);
    void applySgr(const std::vector<int>& params, std::size_t& i);
    static uint16_t ansi16Index(int code, bool fg);
    static uint16_t xterm256Index(int i);

    mutable std::mutex mutex_;
    int cols_{80};
    int rows_{24};

    std::vector<Cell>& screenRow(int r) { return lines_[screenStart_ + static_cast<std::size_t>(r)]; }

    std::vector<std::vector<Cell>> lines_;
    std::size_t screenStart_{0};

    int cursorRow_{0};
    int cursorCol_{0};

    uint16_t defaultFgIdx_{ColorPalette::kDefaultFg};
    uint16_t defaultBgIdx_{ColorPalette::kDefaultBg};
    uint16_t curFgIdx_{defaultFgIdx_};
    uint16_t curBgIdx_{defaultBgIdx_};
    bool bold_{false};

    int savedRow_{0};
    int savedCol_{0};

    // Parser
    enum class ParseState { Ground, Esc, Csi, Osc, Utf8 };
    ParseState state_{ParseState::Ground};
    std::string csiParams_;
    std::string oscPayload_;
    std::string utf8Buf_;
    int utf8Remaining_{0};
    bool csiQuestion_{false};
    bool oscEscPending_{false};

    static constexpr std::size_t kMaxScrollback = 10000;
};

} // namespace flux::term
