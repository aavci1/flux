#include <Flux/Core/ShortcutManager.hpp>
#include <Flux/Core/Window.hpp>
#include <Flux/Core/Runtime.hpp>
#include <Flux/Core/Log.hpp>
#include <SDL3/SDL.h>

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
    (void)window;
    FLUX_LOG_DEBUG("[SHORTCUT] Copy");
}

void PasteCommand::execute(Window& window) {
    if (SDL_HasClipboardText()) {
        char* text = SDL_GetClipboardText();
        if (text && text[0] != '\0') {
            FLUX_LOG_DEBUG("[SHORTCUT] Paste: \"%s\"", text);
            window.handleTextInput(std::string(text));
        }
        SDL_free(text);
    }
}

void CutCommand::execute(Window& window) {
    (void)window;
    FLUX_LOG_DEBUG("[SHORTCUT] Cut");
}

void SelectAllCommand::execute(Window& window) {
    (void)window;
    FLUX_LOG_DEBUG("[SHORTCUT] Select All");
}

} // namespace flux

