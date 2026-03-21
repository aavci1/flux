#import <AppKit/AppKit.h>
#import <Carbon/Carbon.h>
#import <QuartzCore/QuartzCore.h>

#include <Flux/Platform/MacWindow.hpp>
#include <Flux/Platform/GPUPlatformRenderer.hpp>
#include <Flux/Platform/NativeGraphicsSurface.hpp>
#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Core/Window.hpp>
#include <Flux/Core/Log.hpp>

#include <algorithm>
#include <cmath>

namespace flux {
namespace detail {

static void ensureNSApplication() {
    static bool ready = false;
    if (ready) return;
    ready = true;
    [NSApplication sharedApplication];
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp finishLaunching];
}

static Key keyCodeToFluxKey(unsigned short keyCode) {
    switch (keyCode) {
        case kVK_ANSI_A: return Key::A;
        case kVK_ANSI_B: return Key::B;
        case kVK_ANSI_C: return Key::C;
        case kVK_ANSI_D: return Key::D;
        case kVK_ANSI_E: return Key::E;
        case kVK_ANSI_F: return Key::F;
        case kVK_ANSI_G: return Key::G;
        case kVK_ANSI_H: return Key::H;
        case kVK_ANSI_I: return Key::I;
        case kVK_ANSI_J: return Key::J;
        case kVK_ANSI_K: return Key::K;
        case kVK_ANSI_L: return Key::L;
        case kVK_ANSI_M: return Key::M;
        case kVK_ANSI_N: return Key::N;
        case kVK_ANSI_O: return Key::O;
        case kVK_ANSI_P: return Key::P;
        case kVK_ANSI_Q: return Key::Q;
        case kVK_ANSI_R: return Key::R;
        case kVK_ANSI_S: return Key::S;
        case kVK_ANSI_T: return Key::T;
        case kVK_ANSI_U: return Key::U;
        case kVK_ANSI_V: return Key::V;
        case kVK_ANSI_W: return Key::W;
        case kVK_ANSI_X: return Key::X;
        case kVK_ANSI_Y: return Key::Y;
        case kVK_ANSI_Z: return Key::Z;
        case kVK_ANSI_0: return Key::Num0;
        case kVK_ANSI_1: return Key::Num1;
        case kVK_ANSI_2: return Key::Num2;
        case kVK_ANSI_3: return Key::Num3;
        case kVK_ANSI_4: return Key::Num4;
        case kVK_ANSI_5: return Key::Num5;
        case kVK_ANSI_6: return Key::Num6;
        case kVK_ANSI_7: return Key::Num7;
        case kVK_ANSI_8: return Key::Num8;
        case kVK_ANSI_9: return Key::Num9;
        case kVK_F1: return Key::F1;
        case kVK_F2: return Key::F2;
        case kVK_F3: return Key::F3;
        case kVK_F4: return Key::F4;
        case kVK_F5: return Key::F5;
        case kVK_F6: return Key::F6;
        case kVK_F7: return Key::F7;
        case kVK_F8: return Key::F8;
        case kVK_F9: return Key::F9;
        case kVK_F10: return Key::F10;
        case kVK_F11: return Key::F11;
        case kVK_F12: return Key::F12;
        case kVK_Escape: return Key::Escape;
        case kVK_Tab: return Key::Tab;
        case kVK_Delete: return Key::Backspace;
        case kVK_Return: return Key::Enter;
        case kVK_Space: return Key::Space;
        case kVK_ForwardDelete: return Key::Delete;
        case kVK_Home: return Key::Home;
        case kVK_End: return Key::End;
        case kVK_PageUp: return Key::PageUp;
        case kVK_PageDown: return Key::PageDown;
        case kVK_LeftArrow: return Key::Left;
        case kVK_RightArrow: return Key::Right;
        case kVK_UpArrow: return Key::Up;
        case kVK_DownArrow: return Key::Down;
        case kVK_Shift: return Key::LeftShift;
        case kVK_RightShift: return Key::RightShift;
        case kVK_Control: return Key::LeftCtrl;
        case kVK_RightControl: return Key::RightCtrl;
        case kVK_Option: return Key::LeftAlt;
        case kVK_RightOption: return Key::RightAlt;
        case kVK_Command: return Key::LeftSuper;
        case kVK_RightCommand: return Key::RightSuper;
        case kVK_CapsLock: return Key::CapsLock;
        case kVK_ANSI_Minus: return Key::Minus;
        case kVK_ANSI_Equal: return Key::Equal;
        case kVK_ANSI_KeypadPlus: return Key::Equal;
        case kVK_ANSI_KeypadMinus: return Key::Minus;
        case kVK_ANSI_Keypad0: return Key::Num0;
        case kVK_ANSI_LeftBracket: return Key::LeftBracket;
        case kVK_ANSI_RightBracket: return Key::RightBracket;
        case kVK_ANSI_Semicolon: return Key::Semicolon;
        case kVK_ANSI_Quote: return Key::Apostrophe;
        case kVK_ANSI_Grave: return Key::Grave;
        case kVK_ANSI_Backslash: return Key::Backslash;
        case kVK_ANSI_Comma: return Key::Comma;
        case kVK_ANSI_Period: return Key::Period;
        case kVK_ANSI_Slash: return Key::Slash;
        case kVK_Function: return Key::Unknown;
        case kVK_VolumeUp: return Key::Unknown;
        case kVK_VolumeDown: return Key::Unknown;
        case kVK_Mute: return Key::Unknown;
        case kVK_ISO_Section: return Key::Unknown;
        case kVK_JIS_Yen: return Key::Unknown;
        case kVK_JIS_Underscore: return Key::Unknown;
        case kVK_JIS_KeypadComma: return Key::Unknown;
        case kVK_JIS_Eisu: return Key::Unknown;
        case kVK_JIS_Kana: return Key::Unknown;
        default: return Key::Unknown;
    }
}

static KeyModifier nsModifierFlagsToFlux(NSEventModifierFlags f) {
    uint32_t bits = 0;
    if (f & NSEventModifierFlagShift) {
        bits |= static_cast<uint32_t>(KeyModifier::Shift);
    }
    if (f & NSEventModifierFlagControl) {
        bits |= static_cast<uint32_t>(KeyModifier::Ctrl);
    }
    if (f & NSEventModifierFlagOption) {
        bits |= static_cast<uint32_t>(KeyModifier::Alt);
    }
    if (f & NSEventModifierFlagCommand) {
        bits |= static_cast<uint32_t>(KeyModifier::Super);
    }
    return static_cast<KeyModifier>(bits);
}

static int mouseButtonNumber(NSEventType t, NSInteger buttonNumber) {
    if (t == NSEventTypeLeftMouseDown || t == NSEventTypeLeftMouseUp) return 0;
    if (t == NSEventTypeRightMouseDown || t == NSEventTypeRightMouseUp) return 1;
    if (t == NSEventTypeOtherMouseDown || t == NSEventTypeOtherMouseUp)
        return static_cast<int>(buttonNumber);
    return 0;
}

static NSCursor* nscursorForFlux(CursorType c) {
    switch (c) {
        case CursorType::Pointer: return [NSCursor pointingHandCursor];
        case CursorType::Text: return [NSCursor IBeamCursor];
        case CursorType::Crosshair: return [NSCursor crosshairCursor];
        case CursorType::Move: return [NSCursor openHandCursor];
        case CursorType::ResizeNS: return [NSCursor resizeUpDownCursor];
        case CursorType::ResizeEW: return [NSCursor resizeLeftRightCursor];
        case CursorType::ResizeNESW: return [NSCursor closedHandCursor];
        case CursorType::ResizeNWSE: return [NSCursor closedHandCursor];
        case CursorType::NotAllowed: return [NSCursor operationNotAllowedCursor];
        case CursorType::Wait: return [NSCursor arrowCursor];
        case CursorType::Progress: return [NSCursor arrowCursor];
        default: return [NSCursor arrowCursor];
    }
}

} // namespace detail

struct MacWindowImpl {
    NSWindow* window = nil;
    NSView* view = nil;
    id windowDelegate = nil;
};

} // namespace flux

@interface FluxWindowDelegate : NSObject <NSWindowDelegate>
/// `flux::MacWindow*`
@property (nonatomic, assign) void* hostPtr;
@end

@interface FluxMetalView : NSView<NSTextInputClient>
/// `flux::MacWindow*`
@property (nonatomic, assign) void* hostPtr;
@end

@implementation FluxWindowDelegate
- (BOOL)windowShouldClose:(NSWindow*)sender {
    (void)sender;
    auto* host = static_cast<flux::MacWindow*>(self.hostPtr);
    if (host) host->notifyCloseRequested();
    return NO;
}

- (void)windowDidResize:(NSNotification*)notification {
    (void)notification;
    auto* host = static_cast<flux::MacWindow*>(self.hostPtr);
    if (host) host->handleFramebufferResize();
}

- (void)windowDidBecomeKey:(NSNotification*)notification {
    (void)notification;
    auto* host = static_cast<flux::MacWindow*>(self.hostPtr);
    if (!host) return;
    if (flux::Window* w = host->eventTarget()) w->requestRedraw();
}

- (void)windowDidExpose:(NSNotification*)notification {
    (void)notification;
    auto* host = static_cast<flux::MacWindow*>(self.hostPtr);
    if (!host) return;
    if (flux::Window* w = host->eventTarget()) w->requestRedraw();
}
@end

@implementation FluxMetalView
+ (Class)layerClass {
    return [CAMetalLayer class];
}

- (void)updateTrackingAreas {
    [super updateTrackingAreas];
    for (NSTrackingArea* a in self.trackingAreas) {
        [self removeTrackingArea:a];
    }
    NSTrackingAreaOptions opts =
        NSTrackingMouseMoved | NSTrackingActiveInKeyWindow | NSTrackingInVisibleRect;
    NSTrackingArea* ta =
        [[NSTrackingArea alloc] initWithRect:self.bounds options:opts owner:self userInfo:nil];
    [self addTrackingArea:ta];
}

- (BOOL)isFlipped {
    return YES;
}

- (BOOL)acceptsFirstResponder {
    return YES;
}

- (BOOL)becomeFirstResponder {
    return YES;
}

- (void)mouseMoved:(NSEvent*)event {
    auto* host = static_cast<flux::MacWindow*>(self.hostPtr);
    if (!host) return;
    NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
    if (flux::Window* w = host->eventTarget())
        w->handleMouseMove(static_cast<float>(p.x), static_cast<float>(p.y));
}

- (void)mouseDown:(NSEvent*)event {
    [self handleMouseButton:event down:YES];
}
- (void)mouseUp:(NSEvent*)event {
    [self handleMouseButton:event down:NO];
}
- (void)rightMouseDown:(NSEvent*)event {
    [self handleMouseButton:event down:YES];
}
- (void)rightMouseUp:(NSEvent*)event {
    [self handleMouseButton:event down:NO];
}
- (void)otherMouseDown:(NSEvent*)event {
    [self handleMouseButton:event down:YES];
}
- (void)otherMouseUp:(NSEvent*)event {
    [self handleMouseButton:event down:NO];
}

- (void)handleMouseButton:(NSEvent*)event down:(BOOL)down {
    if (down && self.window) {
        // NSTextInputClient only receives insertText: when this view is first responder.
        // Clicks do not automatically restore first responder after other controls steal it.
        [self.window makeFirstResponder:self];
    }
    auto* host = static_cast<flux::MacWindow*>(self.hostPtr);
    if (!host) return;
    NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
    int btn = flux::detail::mouseButtonNumber([event type], [event buttonNumber]);
    if (flux::Window* w = host->eventTarget()) {
        if (down) w->handleMouseDown(btn, static_cast<float>(p.x), static_cast<float>(p.y));
        else w->handleMouseUp(btn, static_cast<float>(p.x), static_cast<float>(p.y));
    }
}

- (void)scrollWheel:(NSEvent*)event {
    auto* host = static_cast<flux::MacWindow*>(self.hostPtr);
    if (!host) return;
    NSPoint p = [self convertPoint:[event locationInWindow] fromView:nil];
    if (flux::Window* w = host->eventTarget()) {
        // Precise (trackpad): scrollingDelta* is in points. Non-precise (typical wheel mice): use
        // delta* in line units — scrollingDelta* is often zero or unreliable; see NSEvent docs.
        CGFloat sx;
        CGFloat sy;
        if ([event hasPreciseScrollingDeltas]) {
            sx = [event scrollingDeltaX];
            sy = [event scrollingDeltaY];
        } else {
            const CGFloat kLineScrollPx = 40.0;
            sx = [event deltaX] * kLineScrollPx;
            sy = [event deltaY] * kLineScrollPx;
        }
        w->handleMouseScroll(static_cast<float>(p.x), static_cast<float>(p.y), static_cast<float>(sx),
                             static_cast<float>(sy));
    }
}

/// Deliver all Cmd/Ctrl key equivalents to the application so ShortcutManager and focused views
/// handle them in one place. Return YES so the menu bar does not steal the event.
- (BOOL)performKeyEquivalent:(NSEvent*)event {
    NSEventModifierFlags mf = [event modifierFlags];
    bool cmdOrCtrl = (mf & NSEventModifierFlagCommand) || (mf & NSEventModifierFlagControl);
    if (!cmdOrCtrl) {
        return [super performKeyEquivalent:event];
    }
    auto* host = static_cast<flux::MacWindow*>(self.hostPtr);
    if (!host) {
        return NO;
    }
    flux::Key k = flux::detail::keyCodeToFluxKey([event keyCode]);
    if (k == flux::Key::Unknown) {
        return [super performKeyEquivalent:event];
    }
    flux::KeyModifier mods = flux::detail::nsModifierFlagsToFlux(mf);
    if (flux::Window* w = host->eventTarget()) {
        w->handleKeyDown(static_cast<int>(k), mods);
    }
    return YES;
}

- (void)keyDown:(NSEvent*)event {
    auto* host = static_cast<flux::MacWindow*>(self.hostPtr);
    if (!host) return;
    flux::Key k = flux::detail::keyCodeToFluxKey([event keyCode]);
    flux::Window* w = host->eventTarget();
    if (!w) return;

    NSEventModifierFlags mf = [event modifierFlags];
    flux::KeyModifier mods = flux::detail::nsModifierFlagsToFlux(mf);
    // On macOS, Control+C sometimes arrives with characters "\x03" but modifierFlags may not
    // include NSEventModifierFlagControl in all input contexts. Ensure Ctrl is set so the
    // terminal receives SIGINT (ASCII ETX) instead of literal 'c'.
    NSString* chars = [event characters];
    if (chars.length == 1 && [event keyCode] == kVK_ANSI_C) {
        unsigned int c = [chars characterAtIndex:0];
        if (c == 0x03) {
            mods = static_cast<flux::KeyModifier>(static_cast<uint32_t>(mods) | static_cast<uint32_t>(flux::KeyModifier::Ctrl));
        }
    }
    w->handleKeyDown(static_cast<int>(k), mods);

    // Cocoa often never calls insertText: on a plain NSView even with NSTextInputClient;
    // interpretKeyEvents does not deliver text reliably here. Pipe the same UTF-8 the field
    // editor would get from [NSEvent characters] (HIG / TextEdit-style hosting).
    unsigned short code = [event keyCode];
    bool skipChars =
        (code == kVK_Return || code == kVK_ANSI_KeypadEnter || code == kVK_Tab || code == kVK_Delete ||
         code == kVK_ForwardDelete ||
         code == kVK_LeftArrow || code == kVK_RightArrow || code == kVK_UpArrow || code == kVK_DownArrow ||
         code == kVK_Home || code == kVK_End || code == kVK_PageUp || code == kVK_PageDown);
    // Do not send character text when Command or Control is held — those are shortcuts
    // (font zoom, quit, etc.); otherwise '=' still arrives as text for Cmd+=.
    bool skipTextForShortcuts =
        (mf & NSEventModifierFlagCommand) != 0 || (mf & NSEventModifierFlagControl) != 0;
    if (!skipTextForShortcuts && !skipChars) {
        NSString* chars = [event characters];
        if (chars.length > 0) {
            std::string utf8([chars UTF8String] ?: "");
            if (!utf8.empty()) {
                w->handleTextInput(utf8);
            }
        }
    }
    // Do not call interpretKeyEvents here: it would duplicate text if insertText: is also
    // invoked. Latin input is delivered via [event characters] above; IME/marked text
    // can be wired later via NSTextInputClient if needed.
}

- (void)keyUp:(NSEvent*)event {
    auto* host = static_cast<flux::MacWindow*>(self.hostPtr);
    if (!host) return;
    flux::Key k = flux::detail::keyCodeToFluxKey([event keyCode]);
    NSEventModifierFlags mf = [event modifierFlags];
    flux::KeyModifier mods = flux::detail::nsModifierFlagsToFlux(mf);
    if (flux::Window* w = host->eventTarget()) w->handleKeyUp(static_cast<int>(k), mods);
}

- (void)flagsChanged:(NSEvent*)event {
    (void)event;
    // Modifier transitions are visible via keyDown/keyUp for most shortcuts; extend here if needed.
}

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange {
    (void)replacementRange;
    auto* host = static_cast<flux::MacWindow*>(self.hostPtr);
    if (!host) return;
    NSEvent* ev = [NSApp currentEvent];
    if (ev != nil) {
        NSEventModifierFlags mf = [ev modifierFlags];
        if ((mf & NSEventModifierFlagCommand) != 0 || (mf & NSEventModifierFlagControl) != 0) {
            return;
        }
    }
    NSString* s = string;
    if (s.length == 0) return;
    std::string utf8([s UTF8String] ?: "");
    if (flux::Window* w = host->eventTarget()) w->handleTextInput(utf8);
}

- (NSRange)selectedRange {
    return NSMakeRange(NSNotFound, 0);
}
- (void)setMarkedText:(id)string selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange {
    (void)string;
    (void)selectedRange;
    (void)replacementRange;
}
- (void)unmarkText {
}
- (NSArray<NSAttributedStringKey>*)
    validAttributesForMarkedText {
    return @[];
}
- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range actualRange:(NSRangePointer)actualRange {
    (void)range;
    (void)actualRange;
    return nil;
}
- (NSUInteger)characterIndexOfPoint:(NSPoint)point {
    (void)point;
    return 0;
}
- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(NSRangePointer)actualRange {
    (void)range;
    (void)actualRange;
    return NSZeroRect;
}
- (void)doCommandBySelector:(SEL)selector {
    (void)selector;
}
@end

namespace flux {

MacWindow::MacWindow(const std::string& title, const Size& size, bool resizable, bool fullscreen,
                     RenderBackendType backend)
    : size_(size)
    , fullscreen_(fullscreen)
    , backendType_(backend) {
    if (backend == RenderBackendType::GPU_Vulkan) {
        throw std::runtime_error("macOS native build supports Metal only");
    }
    if (backend == RenderBackendType::GPU_Auto) backendType_ = RenderBackendType::GPU_Metal;

    detail::ensureNSApplication();

    impl_ = std::make_unique<MacWindowImpl>();

    NSUInteger style = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
    if (resizable) style |= NSWindowStyleMaskResizable;

    NSRect rect = NSMakeRect(100, 100, size.width, size.height);
    impl_->window = [[NSWindow alloc] initWithContentRect:rect
                                                styleMask:style
                                                  backing:NSBackingStoreBuffered
                                                    defer:NO];
    [impl_->window setTitle:@(title.c_str())];
    [impl_->window setReleasedWhenClosed:NO];

    FluxMetalView* view = [[FluxMetalView alloc] initWithFrame:NSMakeRect(0, 0, size.width, size.height)];
    view.wantsLayer = YES;
    view.hostPtr = this;
    impl_->view = view;
    impl_->window.contentView = view;

    FluxWindowDelegate* del = [[FluxWindowDelegate alloc] init];
    del.hostPtr = this;
    impl_->windowDelegate = del;
    impl_->window.delegate = del;

    float dpi = static_cast<float>(impl_->window.backingScaleFactor);
    if (dpi <= 0.f) dpi = 1.f;

    auto gpu = std::make_unique<GPUPlatformRenderer>(gpu::Backend::Metal);
    gpu->setGraphicsSurface(gpu::NativeGraphicsSurface::fromAppleView((__bridge void*)view));
    if (!gpu->initialize(static_cast<int>(size.width), static_cast<int>(size.height), dpi, dpi)) {
        throw std::runtime_error("Failed to initialize Metal GPU renderer");
    }
    renderer_ = std::move(gpu);

    [impl_->window setAcceptsMouseMovedEvents:YES];
    [impl_->window makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
    [impl_->window makeFirstResponder:view];
}

MacWindow::~MacWindow() {
    renderer_.reset();
    if (impl_ && impl_->window) {
        impl_->window.delegate = nil;
        impl_->windowDelegate = nil;
        [impl_->window close];
    }
    impl_.reset();
}

void MacWindow::notifyCloseRequested() {
    shouldClose_ = true;
}

void MacWindow::handleFramebufferResize() {
    if (!impl_ || !impl_->view || !fluxWindow_) return;
    NSRect b = impl_->view.bounds;
    float w = std::max(static_cast<float>(b.size.width), 1.f);
    float h = std::max(static_cast<float>(b.size.height), 1.f);
    size_ = {w, h};
    float dpi = static_cast<float>(impl_->window.backingScaleFactor);
    if (dpi <= 0.f) dpi = 1.f;
    renderer_->resize(static_cast<int>(size_.width), static_cast<int>(size_.height));
    if (auto* gpu = dynamic_cast<GPUPlatformRenderer*>(platformRenderer())) {
        gpu->updateDPIScale(dpi, dpi);
    }
    fluxWindow_->handleResize(size_);
}

void* MacWindow::metalContentView() const {
    return impl_ ? (__bridge void*)impl_->view : nullptr;
}

void MacWindow::resize(const Size& newSize) {
    float w = std::max(newSize.width, 1.f);
    float h = std::max(newSize.height, 1.f);
    size_ = {w, h};
    if (impl_->window && impl_->view) {
        NSSize desired = NSMakeSize(w, h);
        NSSize current = impl_->view.bounds.size;
        // Content size in points; do not use setFrame — frame size includes title bar and
        // would shrink the client area each resize (feedback loop → height 0).
        if (std::fabs(current.width - desired.width) > 0.5f ||
            std::fabs(current.height - desired.height) > 0.5f) {
            [impl_->window setContentSize:desired];
        }
    }
    renderer_->resize(static_cast<int>(size_.width), static_cast<int>(size_.height));
}

void MacWindow::setFullscreen(bool fullscreen) {
    fullscreen_ = fullscreen;
    if (impl_->window) {
        [impl_->window toggleFullScreen:nil];
    }
}

void MacWindow::setTitle(const std::string& title) {
    if (impl_->window) [impl_->window setTitle:@(title.c_str())];
}

unsigned int MacWindow::windowID() const {
    return impl_->window ? static_cast<unsigned int>(impl_->window.windowNumber) : 0;
}

RenderContext* MacWindow::renderContext() {
    return renderer_ ? renderer_->renderContext() : nullptr;
}

PlatformRenderer* MacWindow::platformRenderer() {
    return renderer_.get();
}

void MacWindow::swapBuffers() {
}

void MacWindow::setGpuReadbackEnabled(bool enabled) {
    if (auto* gpu = dynamic_cast<GPUPlatformRenderer*>(renderer_.get())) {
        gpu->setReadbackEnabled(enabled);
    }
}

float MacWindow::dpiScaleX() const {
    if (!impl_->window) return 1.f;
    return static_cast<float>(impl_->window.backingScaleFactor);
}

float MacWindow::dpiScaleY() const {
    return dpiScaleX();
}

Size MacWindow::currentSize() const {
    return size_;
}

bool MacWindow::isFullscreen() const {
    return fullscreen_;
}

bool MacWindow::shouldClose() const {
    return shouldClose_;
}

void MacWindow::setFluxWindow(Window* window) {
    fluxWindow_ = window;
}

void MacWindow::setCursor(CursorType cursor) {
    if (cursor == currentCursor_) return;
    currentCursor_ = cursor;
    [detail::nscursorForFlux(cursor) set];
}

CursorType MacWindow::currentCursor() const {
    return currentCursor_;
}

void MacWindow::processEvents() {
    NSEvent* event = nil;
    while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                       untilDate:[NSDate distantPast]
                                          inMode:NSDefaultRunLoopMode
                                         dequeue:YES])) {
        [NSApp sendEvent:event];
    }
}

void MacWindow::waitForEvents(int timeoutMs) {
    NSDate* until = (timeoutMs < 0) ? [NSDate distantFuture]
                                    : [NSDate dateWithTimeIntervalSinceNow:timeoutMs / 1000.0];
    NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:until
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];
    if (event) [NSApp sendEvent:event];
    while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                       untilDate:[NSDate distantPast]
                                          inMode:NSDefaultRunLoopMode
                                         dequeue:YES])) {
        [NSApp sendEvent:event];
    }
}

} // namespace flux
