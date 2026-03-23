#include <Flux/Core/ResourceManager.hpp>
#include <Flux/Core/Application.hpp>
#include <algorithm>
#include <vector>

namespace flux {

ResourceManager& ResourceManager::instance() {
    return Application::instance().resourceManager();
}

void ResourceManager::registerFont(const std::string& key, FontHandle handle) {
    fonts_[key] = {handle, currentFrame_};
}

FontHandle ResourceManager::findFont(const std::string& key) const {
    auto it = fonts_.find(key);
    if (it != fonts_.end()) {
        const_cast<TrackedFont&>(it->second).lastUsedFrame = currentFrame_;
        return it->second.handle;
    }
    return -1;
}

void ResourceManager::registerImage(const std::string& path, ImageHandle handle) {
    images_[path] = {handle, 0, currentFrame_};
}

ImageHandle ResourceManager::findImage(const std::string& path) const {
    auto it = images_.find(path);
    if (it != images_.end()) {
        const_cast<TrackedImage&>(it->second).lastUsedFrame = currentFrame_;
        return it->second.handle;
    }
    return -1;
}

size_t ResourceManager::memoryUsage() const {
    size_t total = 0;
    for (const auto& [_, img] : images_) {
        total += img.estimatedBytes;
    }
    total += fonts_.size() * 64 * 1024;
    return total;
}

void ResourceManager::collectUnused() {
    if (currentFrame_ < 300) return;
    uint64_t threshold = currentFrame_ - 300;

    for (auto it = images_.begin(); it != images_.end();) {
        if (it->second.lastUsedFrame < threshold) {
            it = images_.erase(it);
        } else {
            ++it;
        }
    }

}

void ResourceManager::clear() {
    fonts_.clear();
    images_.clear();
}

} // namespace flux
