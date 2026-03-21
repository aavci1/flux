#pragma once

#include "Pty.hpp"
#include "TerminalEmulator.hpp"
#include "TerminalKeyBindings.hpp"

#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Graphics/RenderContext.hpp>

#include <atomic>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <thread>

namespace flux::term {

class TerminalSession {
public:
    TerminalSession();
    ~TerminalSession();

    TerminalSession(const TerminalSession&) = delete;
    TerminalSession& operator=(const TerminalSession&) = delete;

    [[nodiscard]] bool start();

    /// Resize the terminal grid to match the pixel viewport (monospace metrics).
    void fitToSize(float widthPx, float heightPx, TextMeasurement& measure, float fontSize,
                   const std::string& fontName);

    [[nodiscard]] TermSnapshot snapshot() const { return emu_.snapshot(); }
    [[nodiscard]] TermSnapshot snapshotRange(std::size_t fromLine, std::size_t count) const {
        return emu_.snapshotRange(fromLine, count);
    }
    [[nodiscard]] std::size_t totalLines() const { return emu_.totalLines(); }

    [[nodiscard]] bool writeBytes(const char* data, std::size_t len);
    /// Handle key using binding table when provided. If bindings is null, uses built-in defaults.
    /// When a binding is a view action, *outViewAction is set and true is returned; otherwise
    /// outViewAction is left unchanged (bytes were sent to PTY).
    [[nodiscard]] bool handleKey(const KeyEvent& e, const TerminalKeyBindings* bindings = nullptr,
                                 std::optional<TerminalViewAction>* outViewAction = nullptr);
    [[nodiscard]] bool pasteText(const std::string& utf8);

    /// Called once when the shell process exits (e.g. user typed `exit`). May run on the PTY reader thread.
    void setOnShellExit(std::function<void()> callback) { onShellExit_ = std::move(callback); }

private:
    void readerLoop();

    Pty pty_;
    TerminalEmulator emu_;
    std::thread thread_;
    std::atomic<bool> running_{false};
    std::function<void()> onShellExit_;

    /// Stabilize cols/rows when layout size hovers on a cell boundary (avoids resize thrash).
    int lastFitCols_{0};
    int lastFitRows_{0};
    /// Integer pixel dimensions — skip fitToSize work when unchanged (layout runs every frame).
    int lastLayoutPixelW_{-1};
    int lastLayoutPixelH_{-1};
    int lastLayoutFontQuant_{-1};
};

} // namespace flux::term
