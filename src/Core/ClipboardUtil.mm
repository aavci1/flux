#import <AppKit/AppKit.h>

#include <Flux/Core/ClipboardUtil.hpp>

namespace flux {

void setClipboardText(const std::string& text) {
    NSString* s = [NSString stringWithUTF8String:text.c_str()];
    if (!s) return;
    [[NSPasteboard generalPasteboard] clearContents];
    [[NSPasteboard generalPasteboard] setString:s forType:NSPasteboardTypeString];
}

bool hasClipboardText() {
    NSString* s = [[NSPasteboard generalPasteboard] stringForType:NSPasteboardTypeString];
    return s != nil && s.length > 0;
}

std::string getClipboardText() {
    NSString* s = [[NSPasteboard generalPasteboard] stringForType:NSPasteboardTypeString];
    if (!s) return {};
    const char* utf8 = [s UTF8String];
    return utf8 ? std::string(utf8) : std::string{};
}

} // namespace flux
