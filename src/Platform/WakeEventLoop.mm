#import <AppKit/AppKit.h>

#include <Flux/Platform/EventLoopWake.hpp>

namespace flux {

void wakePlatformEventLoop() {
    if (!NSApp) return;
    NSEvent* ev =
        [NSEvent otherEventWithType:NSEventTypeApplicationDefined
                           location:NSZeroPoint
                      modifierFlags:0
                          timestamp:0
                       windowNumber:0
                            context:nil
                            subtype:0
                              data1:0
                              data2:0];
    [NSApp postEvent:ev atStart:NO];
}

} // namespace flux
