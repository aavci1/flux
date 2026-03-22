#include <Flux/Core/ShortcutManager.hpp>
#include <Flux/Core/ClipboardUtil.hpp>
#include <Flux/Core/Window.hpp>
#include <Flux/Core/FocusState.hpp>
#include <Flux/Core/Application.hpp>
#include <Flux/Core/Log.hpp>

namespace flux {

ShortcutManager::ShortcutManager() {
    FLUX_LOG_INFO("[SHORTCUTS] Shortcut manager initialized");
}

void ShortcutManager::registerShortcut(KeyBinding binding, std::unique_ptr<ShortcutCommand> command) {
    FLUX_LOG_DEBUG("[SHORTCUTS] Registered shortcut: %s", command->description().c_str());
    shortcuts_[binding] = std::move(command);
}

void ShortcutManager::registerShortcut(KeyBinding binding, std::function<void(Window&)> handler, const std::string& description) {
    registerShortcut(binding, std::make_unique<LambdaShortcutCommand>(std::move(handler), description));
}

void ShortcutManager::unregisterShortcut(KeyBinding binding) {
    shortcuts_.erase(binding);
}

bool ShortcutManager::handleShortcut(const KeyEvent& event, Window& window) {
    KeyBinding binding{event.key, event.modifiers};
    
    auto it = shortcuts_.find(binding);
    if (it != shortcuts_.end()) {
        FLUX_LOG_DEBUG("[SHORTCUTS] Executing: %s", it->second->description().c_str());
        it->second->execute(window);
        return true;
    }
    
    return false;
}

std::string ShortcutManager::getShortcutDescription(KeyBinding binding) const {
    auto it = shortcuts_.find(binding);
    if (it != shortcuts_.end()) {
        return it->second->description();
    }
    return "";
}

bool ShortcutManager::hasShortcut(KeyBinding binding) const {
    return shortcuts_.find(binding) != shortcuts_.end();
}

std::vector<KeyBinding> ShortcutManager::getAllShortcuts() const {
    std::vector<KeyBinding> result;
    result.reserve(shortcuts_.size());
    for (const auto& pair : shortcuts_) {
        result.push_back(pair.first);
    }
    return result;
}

void ShortcutManager::clearAllShortcuts() {
    shortcuts_.clear();
}

// Built-in command implementations

void QuitCommand::execute(Window& window) {
    (void)window;
    FLUX_LOG_INFO("[SHORTCUT] Quit application");
    Application::instance().quit();
}

void CopyCommand::execute(Window& window) {
    View* focused = window.focus().getFocusedView();
    if (!focused) return;

    std::string selected = focused->getSelectedText();
    if (!selected.empty()) {
        setClipboardText(selected);
        FLUX_LOG_DEBUG("[SHORTCUT] Copy: \"%s\"", selected.c_str());
    }
}

void PasteCommand::execute(Window& window) {
    if (hasClipboardText()) {
        std::string text = getClipboardText();
        if (!text.empty()) {
            FLUX_LOG_DEBUG("[SHORTCUT] Paste: \"%s\"", text.c_str());
            window.handleTextInput(text);
        }
    }
}

void CutCommand::execute(Window& window) {
    View* focused = window.focus().getFocusedView();
    if (!focused) return;

    std::string cut = focused->cutSelectedText();
    if (!cut.empty()) {
        setClipboardText(cut);
        FLUX_LOG_DEBUG("[SHORTCUT] Cut: \"%s\"", cut.c_str());
    }
}

void SelectAllCommand::execute(Window& window) {
    View* focused = window.focus().getFocusedView();
    if (!focused) return;

    focused->selectAll();
    FLUX_LOG_DEBUG("[SHORTCUT] Select All");
}

} // namespace flux

