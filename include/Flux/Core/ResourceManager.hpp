#pragma once

#include <string>
#include <unordered_map>
#include <cstddef>
#include <cstdint>

namespace flux {

using FontHandle = int;
using ImageHandle = int;

class ResourceManager {
public:
    static ResourceManager& instance();

    void registerFont(const std::string& key, FontHandle handle);
    FontHandle findFont(const std::string& key) const;

    void registerImage(const std::string& path, ImageHandle handle);
    ImageHandle findImage(const std::string& path) const;

    void registerSVG(const std::string& content, size_t estimatedBytes);
    bool hasSVG(const std::string& content) const;

    size_t fontCount() const { return fonts_.size(); }
    size_t imageCount() const { return images_.size(); }
    size_t svgCount() const { return svgs_.size(); }
    size_t memoryUsage() const;

    void collectUnused();
    void clear();

    ResourceManager() = default;

private:
    struct TrackedFont {
        FontHandle handle;
        uint64_t lastUsedFrame = 0;
    };

    struct TrackedImage {
        ImageHandle handle;
        size_t estimatedBytes = 0;
        uint64_t lastUsedFrame = 0;
    };

    struct TrackedSVG {
        size_t estimatedBytes = 0;
        uint64_t lastUsedFrame = 0;
    };

    std::unordered_map<std::string, TrackedFont> fonts_;
    std::unordered_map<std::string, TrackedImage> images_;
    std::unordered_map<std::string, TrackedSVG> svgs_;
    uint64_t currentFrame_ = 0;
    size_t maxCacheBytes_ = 256 * 1024 * 1024;

    friend class Runtime;
    void advanceFrame() { ++currentFrame_; }
};

} // namespace flux
