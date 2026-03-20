#include <Flux/Platform/LinuxPathUtil.hpp>

#include <climits>
#include <string>
#include <unistd.h>

namespace flux {

std::string LinuxPathUtil::executableDirectory() {
    char buf[PATH_MAX];
    ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
    if (len > 0) {
        buf[len] = '\0';
        std::string p(buf);
        auto pos = p.rfind('/');
        if (pos != std::string::npos) return p.substr(0, pos + 1);
    }
    return {};
}

} // namespace flux
