#pragma once

#include <Flux/Core/Types.hpp>

#include <cstddef>
#include <mutex>
#include <string>
#include <vector>

namespace flux::term {

struct Cell {
    std::string g; // one UTF-8 grapheme (or empty)
    Color fg{Color::rgb(230, 230, 230)};
    Color bg{Color::rgb(30, 30, 30)};
    bool bold{false};
};

struct TermSnapshot {
    std::vector<std::vector<Cell>> scrollback;
    std::vector<std::vector<Cell>> screen;
    int cursorRow{0};
    int cursorCol{0};
    int cols{80};
    int rows{24};
};

/// Minimal VT100 / xterm-style parser with scrollback (sufficient for common shells).
class TerminalEmulator {
public:
    explicit TerminalEmulator(int cols = 80, int rows = 24);

    void resize(int cols, int rows);
    void feed(const char* data, std::size_t len);

    [[nodiscard]] TermSnapshot snapshot() const;

    [[nodiscard]] int cols() const { return cols_; }
    [[nodiscard]] int rows() const { return rows_; }
    [[nodiscard]] std::size_t scrollbackLines() const;

private:
    void clearScreen();
    void fillLine(std::vector<Cell>& line) const;
    void ensureScreen();
    void scrollUp();
    void putUtf8(std::string utf8);
    void handleCsi(const std::vector<int>& params, char finalCh);
    void applySgr(const std::vector<int>& params, std::size_t& i);
    Color ansi16(int code, bool fg) const;
    static Color xterm256(int i);

    mutable std::mutex mutex_;
    int cols_{80};
    int rows_{24};

    std::vector<std::vector<Cell>> scrollback_;
    std::vector<std::vector<Cell>> screen_;

    int cursorRow_{0};
    int cursorCol_{0};

    Color defaultFg_{Color::rgb(230, 230, 230)};
    Color defaultBg_{Color::rgb(30, 30, 30)};
    Color curFg_{defaultFg_};
    Color curBg_{defaultBg_};
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
