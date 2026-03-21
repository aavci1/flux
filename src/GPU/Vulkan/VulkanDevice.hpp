#pragma once

#include <Flux/GPU/Device.hpp>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <vector>

struct SDL_Window;

namespace flux::gpu {

static constexpr uint32_t kMaxFramesInFlight = 2;

class VulkanBuffer : public Buffer {
public:
    VulkanBuffer(VmaAllocator allocator, const BufferDesc& desc);
    ~VulkanBuffer() override;
    void write(const void* data, size_t size, size_t offset = 0) override;
    size_t size() const override;
    VkBuffer native() const { return buffer_; }

private:
    VmaAllocator allocator_;
    VkBuffer buffer_ = VK_NULL_HANDLE;
    VmaAllocation allocation_ = VK_NULL_HANDLE;
    size_t size_;
};

class VulkanTexture : public Texture {
public:
    VulkanTexture(VkDevice device, VmaAllocator allocator, const TextureDesc& desc);
    VulkanTexture(VkImageView view, uint32_t w, uint32_t h);
    ~VulkanTexture() override;
    void write(const void* data, uint32_t x, uint32_t y, uint32_t w, uint32_t h,
               uint32_t srcBytesPerRow) override;
    uint32_t width() const override;
    uint32_t height() const override;
    VkImageView view() const { return view_; }
    VkImage image() const { return image_; }

    static VkFormat toVk(PixelFormat fmt);

private:
    VkDevice device_ = VK_NULL_HANDLE;
    VmaAllocator allocator_ = VK_NULL_HANDLE;
    VkImage image_ = VK_NULL_HANDLE;
    VkImageView view_ = VK_NULL_HANDLE;
    VmaAllocation allocation_ = VK_NULL_HANDLE;
    uint32_t width_, height_;
    bool ownsImage_ = true;
};

class VulkanRenderPipeline : public RenderPipeline {
public:
    VulkanRenderPipeline(VkDevice device, VkRenderPass renderPass, const RenderPipelineDesc& desc);
    ~VulkanRenderPipeline() override;
    VkPipeline native() const { return pipeline_; }
    VkPipelineLayout layout() const { return layout_; }

private:
    VkDevice device_;
    VkPipeline pipeline_ = VK_NULL_HANDLE;
    VkPipelineLayout layout_ = VK_NULL_HANDLE;

    static VkFormat toVkVertexFormat(VertexFormat fmt);
};

class VulkanRenderPassEncoder : public RenderPassEncoder {
public:
    VulkanRenderPassEncoder(VkCommandBuffer cmd, VkExtent2D extent);
    void setPipeline(RenderPipeline* pipeline) override;
    void setVertexBuffer(uint32_t slot, Buffer* buffer, size_t offset = 0) override;
    void setIndexBuffer(Buffer* buffer) override;
    void setFragmentTexture(uint32_t slot, Texture* texture) override;
    void setScissorRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h) override;
    void draw(uint32_t vertexCount, uint32_t instanceCount = 1,
              uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;
    void drawIndexed(uint32_t indexCount, uint32_t instanceCount = 1,
                     uint32_t firstIndex = 0, uint32_t firstInstance = 0) override;
    void end() override;

private:
    VkCommandBuffer cmd_;
    VkExtent2D extent_;
};

class VulkanDevice : public Device {
public:
    VulkanDevice(SDL_Window* window);
    ~VulkanDevice() override;

    std::unique_ptr<Buffer> createBuffer(const BufferDesc& desc) override;
    std::unique_ptr<Texture> createTexture(const TextureDesc& desc) override;
    std::unique_ptr<RenderPipeline> createRenderPipeline(const RenderPipelineDesc& desc) override;

    bool beginFrame() override;
    RenderPassEncoder* beginRenderPass(const RenderPassDesc& desc) override;
    void endRenderPass() override;
    void endFrame() override;

    void resize(uint32_t width, uint32_t height) override;
    PixelFormat swapchainFormat() const override;

private:
    void createInstance(SDL_Window* window);
    void pickPhysicalDevice();
    void createLogicalDevice();
    void createSwapchain(uint32_t width, uint32_t height);
    void destroySwapchain();
    void createSyncObjects();
    void createCommandBuffers();

    VkInstance instance_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice device_ = VK_NULL_HANDLE;
    VkQueue graphicsQueue_ = VK_NULL_HANDLE;
    uint32_t graphicsFamily_ = 0;
    VmaAllocator allocator_ = VK_NULL_HANDLE;

    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    VkFormat swapchainFormat_ = VK_FORMAT_B8G8R8A8_UNORM;
    VkExtent2D swapchainExtent_ = {0, 0};
    std::vector<VkImage> swapchainImages_;
    std::vector<VkImageView> swapchainViews_;

    VkRenderPass renderPass_ = VK_NULL_HANDLE;
    std::vector<VkFramebuffer> framebuffers_;

    VkCommandPool commandPool_ = VK_NULL_HANDLE;
    VkCommandBuffer commandBuffers_[kMaxFramesInFlight] = {};
    VkSemaphore imageAvailable_[kMaxFramesInFlight] = {};
    VkSemaphore renderFinished_[kMaxFramesInFlight] = {};
    VkFence inFlightFences_[kMaxFramesInFlight] = {};

    uint32_t currentFrame_ = 0;
    uint32_t imageIndex_ = 0;
    bool framebufferResized_ = false;

    std::unique_ptr<VulkanRenderPassEncoder> currentEncoder_;

    SDL_Window* window_;
};

} // namespace flux::gpu
