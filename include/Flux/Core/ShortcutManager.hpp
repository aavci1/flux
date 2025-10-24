#pragma once

#include <Flux/Core/KeyEvent.hpp>
#include <memory>
#include <unordered_map>
#include <functional>

namespace flux {

// Forward declaration
class Window;

/**
 * @brief Keyboard shortcut binding (key + modifiers)
 */
struct KeyBinding {
    Key key;
    KeyModifier modifiers;
    
    bool operator==(const KeyBinding& other) const {
        return key == other.key && modifiers == other.modifiers;
    }
};

} // namespace flux

// Hash function for KeyBinding
namespace std {
    template<>
    struct hash<flux::KeyBinding> {
        size_t operator()(const flux::KeyBinding& binding) const {
            return hash<int>()(static_cast<int>(binding.key)) ^
                   (hash<uint32_t>()(static_cast<uint32_t>(binding.modifiers)) << 1);
        }
    };
}

namespace flux {

/**
 * @brief Command interface for keyboard shortcuts
 */
class ShortcutCommand {
public:
    virtual ~ShortcutCommand() = default;
    virtual void execute(Window& window) = 0;
    virtual std::string description() const = 0;
};

/**
 * @brief Manages keyboard shortcuts using Command pattern
 * 
 * Responsible for:
 * - Registering/unregistering keyboard shortcuts
 * - Dispatching shortcut commands when triggered
 * - Providing configurable, extensible shortcut system
 */
class ShortcutManager {
public:
    ShortcutManager();
    ~ShortcutManager() = default;

    /**
     * @brief Register a keyboard shortcut
     */
    void registerShortcut(KeyBinding binding, std::unique_ptr<ShortcutCommand> command);
    
    /**
     * @brief Register a shortcut with a lambda function
     */
    void registerShortcut(KeyBinding binding, std::function<void(Window&)> handler, const std::string& description = "");
    
    /**
     * @brief Unregister a keyboard shortcut
     */
    void unregisterShortcut(KeyBinding binding);
    
    /**
     * @brief Handle a key event, dispatching to registered shortcut if matched
     * @return true if shortcut was handled
     */
    bool handleShortcut(const KeyEvent& event, Window& window);
    
    /**
     * @brief Get description of a registered shortcut
     */
    std::string getShortcutDescription(KeyBinding binding) const;
    
    /**
     * @brief Check if a shortcut is registered
     */
    bool hasShortcut(KeyBinding binding) const;
    
    /**
     * @brief Get all registered shortcuts
     */
    std::vector<KeyBinding> getAllShortcuts() const;
    
    /**
     * @brief Clear all shortcuts
     */
    void clearAllShortcuts();

private:
    std::unordered_map<KeyBinding, std::unique_ptr<ShortcutCommand>> shortcuts_;
};

// Built-in shortcut commands

class QuitCommand : public ShortcutCommand {
public:
    void execute(Window& window) override;
    std::string description() const override { return "Quit application"; }
};

class CopyCommand : public ShortcutCommand {
public:
    void execute(Window& window) override;
    std::string description() const override { return "Copy to clipboard"; }
};

class PasteCommand : public ShortcutCommand {
public:
    void execute(Window& window) override;
    std::string description() const override { return "Paste from clipboard"; }
};

class CutCommand : public ShortcutCommand {
public:
    void execute(Window& window) override;
    std::string description() const override { return "Cut to clipboard"; }
};

class SelectAllCommand : public ShortcutCommand {
public:
    void execute(Window& window) override;
    std::string description() const override { return "Select all"; }
};

/**
 * @brief Lambda-based shortcut command for convenience
 */
class LambdaShortcutCommand : public ShortcutCommand {
public:
    LambdaShortcutCommand(std::function<void(Window&)> handler, const std::string& desc)
        : handler_(std::move(handler)), description_(desc) {}
    
    void execute(Window& window) override {
        if (handler_) handler_(window);
    }
    
    std::string description() const override { return description_; }

private:
    std::function<void(Window&)> handler_;
    std::string description_;
};

} // namespace flux

