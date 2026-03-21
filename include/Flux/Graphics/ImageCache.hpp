#pragma once

#include <Flux/GPU/Device.hpp>
#include <string>
#include <unordered_map>
#include <list>
#include <memory>

namespace flux {

class ImageCache {
public:
    explicit ImageCache(gpu::Device* device, size_t maxEntries = 256);

    gpu::Texture* getOrLoad(const std::string& path);
    gpu::Texture* getById(int id);

    int loadFromFile(const std::string& path);
    int loadFromMemory(const uint8_t* data, int width, int height, int channels);

    void removeById(int id);

    size_t size() const { return idIndex_.size(); }

private:
    gpu::Device* device_;
    size_t maxEntries_;

    struct Entry {
        int id;
        std::unique_ptr<gpu::Texture> texture;
    };

    std::list<Entry> lru_;
    std::unordered_map<int, std::list<Entry>::iterator> idIndex_;
    std::unordered_map<std::string, int> pathToId_;
    std::unordered_map<int, std::string> idToPath_;
    int nextId_ = 1;

    void promote(std::list<Entry>::iterator it);
    void evictOldest();
};

} // namespace flux
