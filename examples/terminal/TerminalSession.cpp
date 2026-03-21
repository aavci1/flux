#include "TerminalSession.hpp"

#include <Flux/Core/Property.hpp>
#include <Flux/Core/Runtime.hpp>
#include <Flux/Core/Typography.hpp>
#include <Flux/Platform/EventLoopWake.hpp>

#include <algorithm>
#include <optional>
#include <poll.h>
#include <unistd.h>
#include <variant>

namespace flux::term {

TerminalSession::TerminalSession() = default;

TerminalSession::~TerminalSession() {
    running_ = false;
    if (thread_.joinable()) {
        thread_.join();
    }
    pty_.close();
}

bool TerminalSession::start() {
    if (running_.exchange(true)) {
        return true;
    }
    emu_.resize(80, 24);
    if (!pty_.start(emu_.rows(), emu_.cols(), "")) {
        running_ = false;
        return false;
    }
    thread_ = std::thread([this] { readerLoop(); });
    return true;
}

void TerminalSession::readerLoop() {
    char buf[16384];
    while (running_) {
        int fd = pty_.masterFd();
        if (fd < 0) {
            break;
        }
        pollfd pfd{};
        pfd.fd = fd;
        pfd.events = POLLIN;
        int pr = poll(&pfd, 1, 120);
        if (pr < 0) {
            break;
        }
        if (pr > 0 && (pfd.revents & (POLLIN | POLLHUP))) {
            ssize_t n = pty_.read(buf, sizeof(buf));
            if (n > 0) {
                emu_.feed(buf, static_cast<std::size_t>(n));
                requestApplicationRedraw();
            } else if (n == 0) {
                // EOF on the PTY master: shell closed the slave (e.g. `exit`).
                running_ = false;
                pty_.close();
                if (onShellExit_) {
                    onShellExit_();
                }
                wakePlatformEventLoop();
                break;
            } else {
                break;
            }
        }
    }
}

void TerminalSession::fitToSize(float widthPx, float heightPx, TextMeasurement& measure, float fontSize,
                                const std::string& fontName) {
    if (widthPx < 1.0f || heightPx < 1.0f) {
        return;
    }
    const int pixelW = static_cast<int>(widthPx);
    const int pixelH = static_cast<int>(heightPx);
    const int fontQ = static_cast<int>(fontSize * 256.0f);
    if (pixelW == lastLayoutPixelW_ && pixelH == lastLayoutPixelH_ && fontQ == lastLayoutFontQuant_) {
        return;
    }
    lastLayoutPixelW_ = pixelW;
    lastLayoutPixelH_ = pixelH;
    lastLayoutFontQuant_ = fontQ;

    std::string face = fontName.empty() ? std::string("Monaco") : fontName;
    // Must match TerminalView render (semibold "M" cell width + same line height rules).
    TextStyle st = makeTextStyle(face, FontWeight::semibold, fontSize, Typography::lineHeightTight, 0.0f);
    float cellW = measure.measureText("M", st).width;
    if (cellW < 4.0f) {
        cellW = 8.0f;
    }
    float lineH = fontSize * Typography::lineHeightTight;
    if (lineH < fontSize * 1.1f) {
        lineH = fontSize * 1.2f;
    }
    int cols = std::max(1, static_cast<int>(static_cast<float>(pixelW) / cellW));
    int rows = std::max(1, static_cast<int>(static_cast<float>(pixelH) / lineH));

    if (lastFitCols_ > 0) {
        if (cols == lastFitCols_ - 1 && widthPx > (static_cast<float>(lastFitCols_) - 0.5f) * cellW) {
            cols = lastFitCols_;
        } else if (cols == lastFitCols_ + 1 && widthPx < (static_cast<float>(lastFitCols_) + 0.5f) * cellW) {
            cols = lastFitCols_;
        }
    }
    if (lastFitRows_ > 0) {
        if (rows == lastFitRows_ - 1 && heightPx > (static_cast<float>(lastFitRows_) - 0.5f) * lineH) {
            rows = lastFitRows_;
        } else if (rows == lastFitRows_ + 1 && heightPx < (static_cast<float>(lastFitRows_) + 0.5f) * lineH) {
            rows = lastFitRows_;
        }
    }

    if (cols != emu_.cols() || rows != emu_.rows()) {
        emu_.resize(cols, rows);
        pty_.resize(rows, cols);
        requestApplicationRedraw();
    }
    lastFitCols_ = cols;
    lastFitRows_ = rows;
}

bool TerminalSession::writeBytes(const char* data, std::size_t len) {
    return pty_.write(data, len);
}

bool TerminalSession::pasteText(const std::string& utf8) {
    return writeBytes(utf8.data(), utf8.size());
}

bool TerminalSession::handleKey(const KeyEvent& e, const TerminalKeyBindings* bindings,
                                std::optional<TerminalViewAction>* outViewAction) {
    const TerminalKeyBindings* table = bindings;
    if (!table) {
        static const TerminalKeyBindings defaultTable = TerminalKeyBindings::defaultBindings();
        table = &defaultTable;
    }
    std::optional<TerminalBindingValue> value = table->lookup(e);
    if (!value) {
        return false;
    }
    if (std::holds_alternative<TerminalViewAction>(*value)) {
        if (outViewAction) {
            *outViewAction = std::get<TerminalViewAction>(*value);
        }
        return true;
    }
    const std::string& bytes = std::get<std::string>(*value);
    return writeBytes(bytes.data(), bytes.size());
}

} // namespace flux::term
