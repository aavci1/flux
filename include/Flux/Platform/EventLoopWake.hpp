#pragma once

namespace flux {

class EventLoopWake {
public:
    virtual ~EventLoopWake() = default;
    virtual void wake() = 0;
};

/// Wake a blocking `PlatformWindow::waitForEvents` (e.g. test server, async redraw).
void wakePlatformEventLoop();

} // namespace flux
