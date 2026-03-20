#include <Flux/Platform/StubFontResolver.hpp>

#include <cstdio>

namespace flux {

std::optional<std::string> StubFontResolver::findFontPath(const std::string& familyName,
                                                           FontWeight weight) {
#if defined(_WIN32)
    (void)familyName;
    (void)weight;
    const char* path = "C:\\Windows\\Fonts\\arial.ttf";
    FILE* f = fopen(path, "r");
    if (f) {
        fclose(f);
        return std::string(path);
    }
#else
    (void)familyName;
    (void)weight;
#endif
    return std::nullopt;
}

std::optional<std::string> StubFontResolver::findFontPathForCodepoint(uint32_t codepoint,
                                                                       const std::string& baseFontPath) {
    (void)codepoint;
    (void)baseFontPath;
    return std::nullopt;
}

} // namespace flux
