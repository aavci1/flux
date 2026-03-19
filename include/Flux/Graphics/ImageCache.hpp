#pragma once

#include <Flux/GPU/Device.hpp>
#include <string>
#include <unordered_map>
#include <memory>

namespace flux {

class ImageCache {
public:
    explicit ImageCache(gpu::Device* device);

    gpu::Texture* getOrLoad(const std::string& path);
    gpu::Texture* getById(int id) const;

    int loadFromFile(const std::string& path);
    int loadFromMemory(const uint8_t* data, int width, int height, int channels);

private:
    gpu::Device* device_;
    std::unordered_map<std::string, int> pathToId_;
    std::unordered_map<int, std::unique_ptr<gpu::Texture>> textures_;
    int nextId_ = 1;
};

} // namespace flux
