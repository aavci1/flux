#include <stb_image.h>
#include <Flux/Graphics/ImageCache.hpp>

namespace flux {

ImageCache::ImageCache(gpu::Device* device) : device_(device) {}

gpu::Texture* ImageCache::getOrLoad(const std::string& path) {
    auto it = pathToId_.find(path);
    if (it != pathToId_.end()) return getById(it->second);
    int id = loadFromFile(path);
    return id > 0 ? getById(id) : nullptr;
}

gpu::Texture* ImageCache::getById(int id) const {
    auto it = textures_.find(id);
    return it != textures_.end() ? it->second.get() : nullptr;
}

int ImageCache::loadFromFile(const std::string& path) {
    int w, h, ch;
    unsigned char* pixels = stbi_load(path.c_str(), &w, &h, &ch, 4);
    if (!pixels) return 0;

    int id = loadFromMemory(pixels, w, h, 4);
    stbi_image_free(pixels);
    if (id > 0) pathToId_[path] = id;
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
    int id = nextId_++;
    textures_[id] = std::move(tex);
    return id;
}

} // namespace flux
