#include <Flux/Platform/AppleFontResolver.hpp>
#include <Flux/Core/Log.hpp>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreText/CoreText.h>

namespace flux {

namespace {

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

} // namespace

std::optional<std::string> AppleFontResolver::findFontPath(const std::string& familyName,
                                                            FontWeight weight) {
    CFStringRef cfName = CFStringCreateWithCString(nullptr, familyName.c_str(), kCFStringEncodingUTF8);
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
    CFURLRef url = static_cast<CFURLRef>(CTFontDescriptorCopyAttribute(descriptor, kCTFontURLAttribute));

    std::optional<std::string> result;
    if (url) {
        char path[1024];
        if (CFURLGetFileSystemRepresentation(url, true, reinterpret_cast<UInt8*>(path), sizeof(path))) {
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

std::optional<std::string> AppleFontResolver::findFontPathForCodepoint(uint32_t codepoint,
                                                                        const std::string& baseFontPath) {
    CTFontRef baseFont = nullptr;
    if (!baseFontPath.empty()) {
        CFStringRef cfPath = CFStringCreateWithCString(nullptr, baseFontPath.c_str(),
                                                        kCFStringEncodingUTF8);
        if (cfPath) {
            CFURLRef url = CFURLCreateWithFileSystemPath(nullptr, cfPath, kCFURLPOSIXPathStyle, false);
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
                FLUX_LOG_DEBUG("[FontProvider] Fallback for U+%04X → %s", codepoint, path.value().c_str());
                result = std::move(path);
            }
        }
        CFRelease(fallback);
    }

    CFRelease(baseFont);
    return result;
}

} // namespace flux
