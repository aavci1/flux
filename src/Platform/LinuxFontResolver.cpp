#include <Flux/Platform/LinuxFontResolver.hpp>
#include <Flux/Core/Log.hpp>

#include <cstdio>
#include <cstdint>

namespace flux {

std::optional<std::string> LinuxFontResolver::findFontPath(const std::string& familyName,
                                                            FontWeight weight) {
    (void)weight;
    const char* paths[] = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/noto/NotoSans-Regular.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
        "/usr/share/fonts/liberation/LiberationSans-Regular.ttf",
        nullptr
    };
    (void)familyName;
    for (int i = 0; paths[i]; ++i) {
        FILE* f = fopen(paths[i], "r");
        if (f) {
            fclose(f);
            return std::string(paths[i]);
        }
    }
    return std::nullopt;
}

std::optional<std::string> LinuxFontResolver::findFontPathForCodepoint(uint32_t codepoint,
                                                                       const std::string& baseFontPath) {
    (void)baseFontPath;
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "fc-match -f '%%{file}' ':charset=%x'", codepoint);
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return std::nullopt;
    char buf[1024];
    std::string path;
    while (fgets(buf, sizeof(buf), pipe)) {
        path += buf;
    }
    int status = pclose(pipe);
    if (status != 0 || path.empty()) return std::nullopt;
    while (!path.empty() && (path.back() == '\n' || path.back() == '\r')) {
        path.pop_back();
    }
    if (path.empty()) return std::nullopt;
    FLUX_LOG_DEBUG("[FontProvider] Fallback for U+%04X → %s", codepoint, path.c_str());
    return path;
}

} // namespace flux
