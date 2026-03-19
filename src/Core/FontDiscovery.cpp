#include <Flux/Core/FontDiscovery.hpp>
#include <Flux/Core/Log.hpp>

#if defined(__APPLE__)
#include <CoreText/CoreText.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

namespace flux {

#if defined(__APPLE__)

static CTFontSymbolicTraits weightToTraits(FontWeight weight) {
    if (static_cast<int>(weight) >= static_cast<int>(FontWeight::bold)) {
        return kCTFontBoldTrait;
    }
    return 0;
}

std::optional<std::string> FontDiscovery::findFontPath(const std::string& familyName,
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
            FLUX_LOG_DEBUG("[FontDiscovery] Resolved '%s' → %s", familyName.c_str(), path);
        }
        CFRelease(url);
    }

    CFRelease(descriptor);
    CFRelease(attrs);
    CFRelease(cfName);
    return result;
}

#elif defined(__linux__)

std::optional<std::string> FontDiscovery::findFontPath(const std::string& familyName,
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

std::optional<std::string> FontDiscovery::findFontPath(const std::string& familyName,
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

std::optional<std::string> FontDiscovery::findFontPath(const std::string& familyName,
                                                        FontWeight weight) {
    (void)familyName;
    (void)weight;
    return std::nullopt;
}

#endif

} // namespace flux
