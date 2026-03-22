#include <Flux.hpp>

#include <Flux/Core/KeyEvent.hpp>
#include <Flux/Core/ShortcutManager.hpp>

#include "TerminalKeyBindings.hpp"
#include "TerminalSession.hpp"
#include "TerminalView.hpp"

#include <memory>

using namespace flux;

int main(int argc, char* argv[]) {
    Application app(argc, argv);

    auto session = std::make_shared<flux::term::TerminalSession>();
    if (!session->start()) {
        return 1;
    }

    auto& window = app.createWindow({
        .size = {920, 640},
        .title = "Flux Terminal",
    });

    session->setOnShellExit([&app] { app.quit(); });

    flux::term::TerminalView terminalView;
    terminalView.session = session;
    // Use default bindings; can be replaced or modified for custom shortcuts.
    terminalView.keyBindings = std::make_shared<flux::term::TerminalKeyBindings>(
        flux::term::TerminalKeyBindings::defaultBindings());

    window.setRootView(terminalView);

    // Keyboard/text is dispatched only to the focused view; focus starts empty until the first click.
    auto initialFocusDone = std::make_shared<bool>(false);
    window.setPostRenderCallback([&window, initialFocusDone]() {
        if (!*initialFocusDone && window.focus().getFocusableViewCount() > 0) {
            *initialFocusDone = true;
            window.focus().focusNext();
        }
    });

    return app.exec();
}
