#include <Flux/Core/ShortcutManager.hpp>
#include <Flux/Core/Window.hpp>
#include <Flux/Core/Application.hpp>
#include <iostream>

namespace flux {

ShortcutManager::ShortcutManager() {
    std::cout << "[SHORTCUTS] Shortcut manager initialized\n";
}

void ShortcutManager::registerShortcut(KeyBinding binding, std::unique_ptr<ShortcutCommand> command) {
    std::cout << "[SHORTCUTS] Registered shortcut: " << command->description() << "\n";
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
        std::cout << "[SHORTCUTS] Executing: " << it->second->description() << "\n";
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
    std::cout << "[SHORTCUT] Quit application\n";
    Application::instance().quit();
}

void CopyCommand::execute(Window& window) {
    (void)window;
    std::cout << "[SHORTCUT] Copy (not yet implemented)\n";
    // TODO: Implement clipboard copy
}

void PasteCommand::execute(Window& window) {
    (void)window;
    std::cout << "[SHORTCUT] Paste (not yet implemented)\n";
    // TODO: Implement clipboard paste
}

void CutCommand::execute(Window& window) {
    (void)window;
    std::cout << "[SHORTCUT] Cut (not yet implemented)\n";
    // TODO: Implement clipboard cut
}

void SelectAllCommand::execute(Window& window) {
    (void)window;
    std::cout << "[SHORTCUT] Select All (not yet implemented)\n";
    // TODO: Implement select all
}

} // namespace flux

