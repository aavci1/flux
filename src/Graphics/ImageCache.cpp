#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <Flux/Graphics/ImageCache.hpp>

namespace flux {

ImageCache::ImageCache(gpu::Device* device, size_t maxEntries)
    : device_(device), maxEntries_(maxEntries) {}

gpu::Texture* ImageCache::getOrLoad(const std::string& path) {
    auto pit = pathToId_.find(path);
    if (pit != pathToId_.end()) return getById(pit->second);
    int id = loadFromFile(path);
    return id > 0 ? getById(id) : nullptr;
}

gpu::Texture* ImageCache::getById(int id) {
    auto it = idIndex_.find(id);
    if (it == idIndex_.end()) return nullptr;
    promote(it->second);
    return it->second->texture.get();
}

int ImageCache::loadFromFile(const std::string& path) {
    int w, h, ch;
    unsigned char* pixels = stbi_load(path.c_str(), &w, &h, &ch, 4);
    if (!pixels) return 0;

    int id = loadFromMemory(pixels, w, h, 4);
    stbi_image_free(pixels);
    if (id > 0) {
        pathToId_[path] = id;
        idToPath_[id] = path;
    }
    return id;
}

int ImageCache::loadFromMemory(const uint8_t* data, int width, int height, int channels) {
    (void)channels;
    gpu::TextureDesc desc;
    desc.width = static_cast<uint32_t>(width);
    desc.height = static_cast<uint32_t>(height);
    desc.format = gpu::PixelFormat::RGBA8;
    auto tex = device_->createTexture(desc);
    if (!tex) return 0;

    tex->write(data, 0, 0, desc.width, desc.height);

    if (idIndex_.size() >= maxEntries_) {
        evictOldest();
    }

    int id = nextId_++;
    lru_.push_back({id, std::move(tex)});
    idIndex_[id] = std::prev(lru_.end());
    return id;
}

void ImageCache::removeById(int id) {
    auto it = idIndex_.find(id);
    if (it == idIndex_.end()) return;

    auto pathIt = idToPath_.find(id);
    if (pathIt != idToPath_.end()) {
        pathToId_.erase(pathIt->second);
        idToPath_.erase(pathIt);
    }

    lru_.erase(it->second);
    idIndex_.erase(it);
}

void ImageCache::promote(std::list<Entry>::iterator it) {
    lru_.splice(lru_.end(), lru_, it);
}

void ImageCache::evictOldest() {
    if (lru_.empty()) return;
    auto& front = lru_.front();

    auto pathIt = idToPath_.find(front.id);
    if (pathIt != idToPath_.end()) {
        pathToId_.erase(pathIt->second);
        idToPath_.erase(pathIt);
    }

    idIndex_.erase(front.id);
    lru_.pop_front();
}

} // namespace flux
