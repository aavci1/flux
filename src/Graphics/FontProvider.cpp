#include <Flux/Graphics/FontProvider.hpp>
#include <Flux/Core/Log.hpp>

#if defined(__APPLE__)
#include <CoreText/CoreText.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace flux {

namespace {

#if defined(__APPLE__)

std::optional<std::string> pathFromCTFont(CTFontRef font) {
    if (!font) return std::nullopt;
    CFURLRef url = static_cast<CFURLRef>(CTFontCopyAttribute(font, kCTFontURLAttribute));
    if (!url) return std::nullopt;
    char buf[1024];
    std::optional<std::string> result;
    if (CFURLGetFileSystemRepresentation(url, true, reinterpret_cast<UInt8*>(buf), sizeof(buf))) {
        result = std::string(buf);
    }
    CFRelease(url);
    return result;
}

CTFontSymbolicTraits weightToTraits(FontWeight weight) {
    if (static_cast<int>(weight) >= static_cast<int>(FontWeight::bold)) {
        return kCTFontBoldTrait;
    }
    return 0;
}

#endif

} // namespace

// ---------------------------------------------------------------------------
// findFontPath -- resolve a family name + weight to a file path
// ---------------------------------------------------------------------------

#if defined(__APPLE__)

std::optional<std::string> FontProvider::findFontPath(const std::string& familyName,
                                                       FontWeight weight) {
    CFStringRef cfName = CFStringCreateWithCString(nullptr, familyName.c_str(),
                                                    kCFStringEncodingUTF8);
    if (!cfName) return std::nullopt;

    CFMutableDictionaryRef attrs = CFDictionaryCreateMutable(
        nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    CFDictionarySetValue(attrs, kCTFontFamilyNameAttribute, cfName);

    CTFontSymbolicTraits traits = weightToTraits(weight);
    if (traits != 0) {
        CFMutableDictionaryRef traitDict = CFDictionaryCreateMutable(
            nullptr, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        int32_t traitsVal = static_cast<int32_t>(traits);
        CFNumberRef cfTraits = CFNumberCreate(nullptr, kCFNumberSInt32Type, &traitsVal);
        CFDictionarySetValue(traitDict, kCTFontSymbolicTrait, cfTraits);
        CFDictionarySetValue(attrs, kCTFontTraitsAttribute, traitDict);
        CFRelease(cfTraits);
        CFRelease(traitDict);
    }

    CTFontDescriptorRef descriptor = CTFontDescriptorCreateWithAttributes(attrs);
    CFURLRef url = static_cast<CFURLRef>(
        CTFontDescriptorCopyAttribute(descriptor, kCTFontURLAttribute));

    std::optional<std::string> result;
    if (url) {
        char path[1024];
        if (CFURLGetFileSystemRepresentation(url, true,
                                              reinterpret_cast<UInt8*>(path), sizeof(path))) {
            result = std::string(path);
            FLUX_LOG_DEBUG("[FontProvider] Resolved '%s' → %s", familyName.c_str(), path);
        }
        CFRelease(url);
    }

    CFRelease(descriptor);
    CFRelease(attrs);
    CFRelease(cfName);
    return result;
}

#elif defined(__linux__)

std::optional<std::string> FontProvider::findFontPath(const std::string& familyName,
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

#elif defined(_WIN32)

std::optional<std::string> FontProvider::findFontPath(const std::string& familyName,
                                                       FontWeight weight) {
    (void)familyName;
    (void)weight;
    const char* path = "C:\\Windows\\Fonts\\arial.ttf";
    FILE* f = fopen(path, "r");
    if (f) {
        fclose(f);
        return std::string(path);
    }
    return std::nullopt;
}

#else

std::optional<std::string> FontProvider::findFontPath(const std::string& familyName,
                                                       FontWeight weight) {
    (void)familyName;
    (void)weight;
    return std::nullopt;
}

#endif

// ---------------------------------------------------------------------------
// findFontPathForCodepoint -- ask the OS for a font that covers a codepoint
// ---------------------------------------------------------------------------

#if defined(__APPLE__)

std::optional<std::string> FontProvider::findFontPathForCodepoint(uint32_t codepoint,
                                                                   const std::string& baseFontPath) {
    CTFontRef baseFont = nullptr;
    if (!baseFontPath.empty()) {
        CFStringRef cfPath = CFStringCreateWithCString(nullptr, baseFontPath.c_str(),
                                                        kCFStringEncodingUTF8);
        if (cfPath) {
            CFURLRef url = CFURLCreateWithFileSystemPath(nullptr, cfPath,
                                                          kCFURLPOSIXPathStyle, false);
            if (url) {
                CFArrayRef descs = CTFontManagerCreateFontDescriptorsFromURL(url);
                if (descs && CFArrayGetCount(descs) > 0) {
                    auto d = static_cast<CTFontDescriptorRef>(CFArrayGetValueAtIndex(descs, 0));
                    baseFont = CTFontCreateWithFontDescriptor(d, 0.0, nullptr);
                }
                if (descs) CFRelease(descs);
                CFRelease(url);
            }
            CFRelease(cfPath);
        }
    }
    if (!baseFont) {
        baseFont = CTFontCreateUIFontForLanguage(kCTFontUIFontSystem, 12.0, nullptr);
    }
    if (!baseFont) {
        return std::nullopt;
    }

    UniChar utf16[2];
    CFIndex utf16Len;
    if (codepoint <= 0xFFFF) {
        utf16[0] = static_cast<UniChar>(codepoint);
        utf16Len = 1;
    } else {
        uint32_t cp = codepoint - 0x10000;
        utf16[0] = static_cast<UniChar>(0xD800 + (cp >> 10));
        utf16[1] = static_cast<UniChar>(0xDC00 + (cp & 0x3FF));
        utf16Len = 2;
    }
    CFStringRef str = CFStringCreateWithCharacters(nullptr, utf16, utf16Len);
    if (!str) {
        CFRelease(baseFont);
        return std::nullopt;
    }

    CTFontRef fallback = CTFontCreateForString(baseFont, str, CFRangeMake(0, utf16Len));
    CFRelease(str);

    std::optional<std::string> result;
    if (fallback) {
        auto path = pathFromCTFont(fallback);
        if (path.has_value()) {
            auto basePath = pathFromCTFont(baseFont);
            if (!basePath.has_value() || basePath.value() != path.value()) {
                FLUX_LOG_DEBUG("[FontProvider] Fallback for U+%04X → %s", codepoint,
                               path.value().c_str());
                result = std::move(path);
            }
        }
        CFRelease(fallback);
    }

    CFRelease(baseFont);
    return result;
}

#elif defined(__linux__)

std::optional<std::string> FontProvider::findFontPathForCodepoint(uint32_t codepoint,
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

#else

std::optional<std::string> FontProvider::findFontPathForCodepoint(uint32_t codepoint,
                                                                   const std::string& baseFontPath) {
    (void)codepoint;
    (void)baseFontPath;
    return std::nullopt;
}

#endif

} // namespace flux
