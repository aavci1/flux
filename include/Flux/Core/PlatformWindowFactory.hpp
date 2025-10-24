#pragma once

#include <Flux/Core/Types.hpp>
#include <memory>
#include <string>

namespace flux {

// Forward declaration
class PlatformWindow;

/**
 * @brief Factory interface for creating platform-specific windows
 * 
 * Implements Strategy pattern to allow different platform window
 * implementations without Window class knowing platform details.
 */
class PlatformWindowFactory {
public:
    virtual ~PlatformWindowFactory() = default;
    
    /**
     * @brief Create a platform-specific window
     */
    virtual std::unique_ptr<PlatformWindow> createWindow(
        const std::string& title,
        const Size& size,
        bool resizable,
        bool fullscreen
    ) = 0;
    
    /**
     * @brief Get the name of this platform
     */
    virtual std::string getPlatformName() const = 0;
};

/**
 * @brief Wayland window factory implementation
 */
class WaylandWindowFactory : public PlatformWindowFactory {
public:
    std::unique_ptr<PlatformWindow> createWindow(
        const std::string& title,
        const Size& size,
        bool resizable,
        bool fullscreen
    ) override;
    
    std::string getPlatformName() const override { return "Wayland"; }
};

/**
 * @brief Get the default platform factory for current system
 */
PlatformWindowFactory* getDefaultPlatformFactory();

} // namespace flux

