#pragma once

namespace flux {

/// Wake a blocking `PlatformWindow::waitForEvents` (e.g. test server, async redraw).
void wakePlatformEventLoop();

} // namespace flux
