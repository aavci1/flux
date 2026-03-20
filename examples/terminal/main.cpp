#include <Flux.hpp>

#include "TerminalSession.hpp"
#include "TerminalView.hpp"

#include <memory>

using namespace flux;

int main(int argc, char* argv[]) {
    Runtime runtime(argc, argv);

    auto session = std::make_shared<flux::term::TerminalSession>();
    if (!session->start()) {
        return 1;
    }

    auto& window = runtime.createWindow({
        .size = {920, 640},
        .title = "Flux Terminal",
    });

    session->setOnShellExit([&runtime] { runtime.quit(); });

    flux::term::TerminalView terminalView;
    terminalView.session = session;

    window.setRootView(terminalView);

    // Keyboard/text is dispatched only to the focused view; focus starts empty until the first click.
    auto initialFocusDone = std::make_shared<bool>(false);
    window.setPostRenderCallback([&window, initialFocusDone]() {
        if (!*initialFocusDone && window.focus().getFocusableViewCount() > 0) {
            *initialFocusDone = true;
            window.focus().focusNext();
        }
    });

    return runtime.run();
}
