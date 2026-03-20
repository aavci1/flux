#include "Pty.hpp"

#include <cerrno>
#include <fcntl.h>
#include <signal.h>
#include <cstring>
#include <unistd.h>

#include <cstdlib>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>

#if defined(__linux__)
#include <pty.h>
#else
#include <util.h>
#endif

namespace flux::term {

namespace {

void setWinsize(int fd, int rows, int cols) {
    if (rows < 1) rows = 24;
    if (cols < 1) cols = 80;
    struct winsize ws {};
    ws.ws_row = static_cast<unsigned short>(rows);
    ws.ws_col = static_cast<unsigned short>(cols);
    ws.ws_xpixel = 0;
    ws.ws_ypixel = 0;
    (void)ioctl(fd, TIOCSWINSZ, &ws);
}

} // namespace

Pty::Pty(Pty&& o) noexcept : masterFd_(o.masterFd_), childPid_(o.childPid_) {
    o.masterFd_ = -1;
    o.childPid_ = -1;
}

Pty& Pty::operator=(Pty&& o) noexcept {
    if (this != &o) {
        close();
        masterFd_ = o.masterFd_;
        childPid_ = o.childPid_;
        o.masterFd_ = -1;
        o.childPid_ = -1;
    }
    return *this;
}

Pty::~Pty() { close(); }

bool Pty::start(int rows, int cols, const std::string& shellPath) {
    close();

    int master = -1;
    int slave = -1;
    char name[256]{};
    if (openpty(&master, &slave, name, nullptr, nullptr) != 0) {
        return false;
    }

    setWinsize(master, rows, cols);

    pid_t pid = fork();
    if (pid < 0) {
        ::close(master);
        ::close(slave);
        return false;
    }

    if (pid == 0) {
        // Child
        setsid();
#if defined(TIOCSCTTY)
        (void)ioctl(slave, TIOCSCTTY, 0);
#endif
        dup2(slave, STDIN_FILENO);
        dup2(slave, STDOUT_FILENO);
        dup2(slave, STDERR_FILENO);
        if (slave > 2) {
            ::close(slave);
        }
        ::close(master);

        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        signal(SIGTTOU, SIG_DFL);

        setenv("TERM", "xterm-256color", 1);
        setenv("COLORTERM", "truecolor", 1);
        const char* sh = shellPath.empty() ? nullptr : shellPath.c_str();
        if (!sh) {
            sh = getenv("SHELL");
        }
        if (!sh) {
            sh = "/bin/zsh";
        }

        execl(sh, sh, "-l", static_cast<char*>(nullptr));
        _exit(127);
    }

    // Parent
    ::close(slave);
    fcntl(master, F_SETFL, fcntl(master, F_GETFL, 0) | O_NONBLOCK);
    masterFd_ = master;
    childPid_ = pid;
    return true;
}

void Pty::resize(int rows, int cols) {
    if (masterFd_ >= 0) {
        setWinsize(masterFd_, rows, cols);
    }
}

ssize_t Pty::read(char* buf, std::size_t cap) {
    if (masterFd_ < 0) {
        return -1;
    }
    ssize_t n = ::read(masterFd_, buf, cap);
    if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return 0;
    }
    return n;
}

bool Pty::write(const char* data, std::size_t len) {
    if (masterFd_ < 0 || !data || len == 0) {
        return false;
    }
    std::size_t off = 0;
    while (off < len) {
        ssize_t w = ::write(masterFd_, data + off, len - off);
        if (w < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            return false;
        }
        off += static_cast<std::size_t>(w);
    }
    return true;
}

void Pty::close() {
    if (masterFd_ >= 0) {
        ::close(masterFd_);
        masterFd_ = -1;
    }
    if (childPid_ > 0) {
        int status = 0;
        // Shell has usually already exited when we close; reap to avoid zombies.
        while (waitpid(childPid_, &status, 0) < 0 && errno == EINTR) {
        }
        childPid_ = -1;
    }
}

} // namespace flux::term
