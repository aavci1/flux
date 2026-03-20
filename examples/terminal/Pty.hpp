#pragma once

#include <cstddef>
#include <string>

#include <sys/types.h>

namespace flux::term {

/// POSIX pseudo-terminal: runs a login shell on the slave side.
class Pty {
public:
    Pty() = default;
    Pty(const Pty&) = delete;
    Pty& operator=(const Pty&) = delete;
    Pty(Pty&&) noexcept;
    Pty& operator=(Pty&&) noexcept;
    ~Pty();

    [[nodiscard]] bool start(int rows, int cols, const std::string& shellPath);
    void resize(int rows, int cols);

    [[nodiscard]] int masterFd() const { return masterFd_; }

    /// Non-blocking read from the PTY master. Returns 0 on EAGAIN, -1 on error.
    [[nodiscard]] ssize_t read(char* buf, std::size_t cap);

    /// Write user input to the shell.
    [[nodiscard]] bool write(const char* data, std::size_t len);

    void close();
    [[nodiscard]] bool isOpen() const { return masterFd_ >= 0; }

private:
    int masterFd_{-1};
    pid_t childPid_{-1};
    int lastWsRows_{-1};
    int lastWsCols_{-1};
};

} // namespace flux::term
