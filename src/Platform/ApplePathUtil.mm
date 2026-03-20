#include <Flux/Platform/ApplePathUtil.hpp>

#include <mach-o/dyld.h>
#include <string>

namespace flux {

std::string ApplePathUtil::executableDirectory() {
    char buf[4096];
    uint32_t size = sizeof(buf);
    if (_NSGetExecutablePath(buf, &size) == 0) {
        std::string p(buf);
        auto pos = p.rfind('/');
        if (pos != std::string::npos) return p.substr(0, pos + 1);
    }
    return {};
}

} // namespace flux
