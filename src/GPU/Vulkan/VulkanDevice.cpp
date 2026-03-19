#define VMA_IMPLEMENTATION
#include "VulkanDevice.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <stdexcept>
#include <cstring>
#include <algorithm>

namespace flux::gpu {

static void vkCheck(VkResult result, const char* msg) {
    if (result != VK_SUCCESS) {
        throw std::runtime_error(std::string(msg) + " (VkResult=" + std::to_string(result) + ")");
    }
}

// =============================================================================
// VulkanBuffer
// =============================================================================

VulkanBuffer::VulkanBuffer(VmaAllocator allocator, const BufferDesc& desc)
    : allocator_(allocator), size_(desc.size)
{
    VkBufferCreateInfo ci{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    ci.size = size_;
    switch (desc.usage) {
        case BufferUsage::Vertex:  ci.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; break;
        case BufferUsage::Index:   ci.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT; break;
        case BufferUsage::Uniform: ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; break;
    }

    VmaAllocationCreateInfo ai{};
    ai.usage = VMA_MEMORY_USAGE_AUTO;
    ai.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
               VMA_ALLOCATION_CREATE_MAPPED_BIT;

    vkCheck(vmaCreateBuffer(allocator_, &ci, &ai, &buffer_, &allocation_, nullptr),
            "Failed to create Vulkan buffer");
}

VulkanBuffer::~VulkanBuffer() {
    if (buffer_) vmaDestroyBuffer(allocator_, buffer_, allocation_);
}

void VulkanBuffer::write(const void* data, size_t size, size_t offset) {
    void* mapped = nullptr;
    vmaMapMemory(allocator_, allocation_, &mapped);
    std::memcpy(static_cast<uint8_t*>(mapped) + offset, data, size);
    vmaUnmapMemory(allocator_, allocation_);
}

size_t VulkanBuffer::size() const { return size_; }

// =============================================================================
// VulkanTexture
// =============================================================================

VulkanTexture::VulkanTexture(VkDevice device, VmaAllocator allocator, const TextureDesc& desc)
    : device_(device), allocator_(allocator), width_(desc.width), height_(desc.height), ownsImage_(true)
{
    VkImageCreateInfo ci{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    ci.imageType = VK_IMAGE_TYPE_2D;
    ci.format = toVk(desc.format);
    ci.extent = {width_, height_, 1};
    ci.mipLevels = 1;
    ci.arrayLayers = 1;
    ci.samples = VK_SAMPLE_COUNT_1_BIT;
    ci.tiling = VK_IMAGE_TILING_OPTIMAL;
    ci.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    if (desc.renderTarget) ci.usage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VmaAllocationCreateInfo ai{};
    ai.usage = VMA_MEMORY_USAGE_AUTO;

    vkCheck(vmaCreateImage(allocator_, &ci, &ai, &image_, &allocation_, nullptr),
            "Failed to create Vulkan image");

    VkImageViewCreateInfo vci{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    vci.image = image_;
    vci.viewType = VK_IMAGE_VIEW_TYPE_2D;
    vci.format = ci.format;
    vci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

    vkCheck(vkCreateImageView(device_, &vci, nullptr, &view_),
            "Failed to create image view");
}

VulkanTexture::VulkanTexture(VkImageView view, uint32_t w, uint32_t h)
    : view_(view), width_(w), height_(h), ownsImage_(false)
{
}

VulkanTexture::~VulkanTexture() {
    if (ownsImage_) {
        if (view_) vkDestroyImageView(device_, view_, nullptr);
        if (image_) vmaDestroyImage(allocator_, image_, allocation_);
    }
}

void VulkanTexture::write(const void*, uint32_t, uint32_t, uint32_t, uint32_t) {
    // TODO: staging buffer upload with command buffer
}

uint32_t VulkanTexture::width() const { return width_; }
uint32_t VulkanTexture::height() const { return height_; }

VkFormat VulkanTexture::toVk(PixelFormat fmt) {
    switch (fmt) {
        case PixelFormat::RGBA8:    return VK_FORMAT_R8G8B8A8_UNORM;
        case PixelFormat::BGRA8:    return VK_FORMAT_B8G8R8A8_UNORM;
        case PixelFormat::R8:       return VK_FORMAT_R8_UNORM;
        case PixelFormat::Depth32F: return VK_FORMAT_D32_SFLOAT;
    }
    return VK_FORMAT_B8G8R8A8_UNORM;
}

// =============================================================================
// VulkanRenderPipeline
// =============================================================================

VulkanRenderPipeline::VulkanRenderPipeline(VkDevice device, VkRenderPass renderPass,
                                           const RenderPipelineDesc& desc)
    : device_(device)
{
    auto createModule = [&](std::span<const uint8_t> spirv) -> VkShaderModule {
        VkShaderModuleCreateInfo ci{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
        ci.codeSize = spirv.size();
        ci.pCode = reinterpret_cast<const uint32_t*>(spirv.data());
        VkShaderModule mod;
        vkCheck(vkCreateShaderModule(device_, &ci, nullptr, &mod), "Failed to create shader module");
        return mod;
    };

    VkShaderModule vertMod = createModule(desc.vertexShader.spirv);
    VkShaderModule fragMod = createModule(desc.fragmentShader.spirv);

    VkPipelineShaderStageCreateInfo stages[2]{};
    stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    stages[0].module = vertMod;
    stages[0].pName = "main";
    stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    stages[1].module = fragMod;
    stages[1].pName = "main";

    std::vector<VkVertexInputBindingDescription> bindings;
    std::vector<VkVertexInputAttributeDescription> attributes;
    for (uint32_t i = 0; i < desc.vertexBuffers.size(); i++) {
        const auto& layout = desc.vertexBuffers[i];
        VkVertexInputBindingDescription bd{};
        bd.binding = i;
        bd.stride = layout.stride;
        bd.inputRate = layout.perInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;
        bindings.push_back(bd);
        for (const auto& attr : layout.attributes) {
            VkVertexInputAttributeDescription ad{};
            ad.location = attr.location;
            ad.binding = i;
            ad.format = toVkVertexFormat(attr.format);
            ad.offset = attr.offset;
            attributes.push_back(ad);
        }
    }

    VkPipelineVertexInputStateCreateInfo vertexInput{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
    vertexInput.vertexBindingDescriptionCount = static_cast<uint32_t>(bindings.size());
    vertexInput.pVertexBindingDescriptions = bindings.data();
    vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributes.size());
    vertexInput.pVertexAttributeDescriptions = attributes.data();

    VkPipelineInputAssemblyStateCreateInfo ia{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineViewportStateCreateInfo vp{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    vp.viewportCount = 1;
    vp.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo raster{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    raster.polygonMode = VK_POLYGON_MODE_FILL;
    raster.cullMode = VK_CULL_MODE_NONE;
    raster.frontFace = VK_FRONT_FACE_CLOCKWISE;
    raster.lineWidth = 1.0f;

    VkPipelineMultisampleStateCreateInfo ms{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState blend{};
    blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                           VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    if (desc.blendEnabled) {
        blend.blendEnable = VK_TRUE;
        blend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        blend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blend.colorBlendOp = VK_BLEND_OP_ADD;
        blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        blend.alphaBlendOp = VK_BLEND_OP_ADD;
    }

    VkPipelineColorBlendStateCreateInfo cb{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    cb.attachmentCount = 1;
    cb.pAttachments = &blend;

    VkDynamicState dynStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dyn{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dyn.dynamicStateCount = 2;
    dyn.pDynamicStates = dynStates;

    VkPipelineLayoutCreateInfo layoutCI{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    vkCheck(vkCreatePipelineLayout(device_, &layoutCI, nullptr, &layout_),
            "Failed to create pipeline layout");

    VkGraphicsPipelineCreateInfo ci{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
    ci.stageCount = 2;
    ci.pStages = stages;
    ci.pVertexInputState = &vertexInput;
    ci.pInputAssemblyState = &ia;
    ci.pViewportState = &vp;
    ci.pRasterizationState = &raster;
    ci.pMultisampleState = &ms;
    ci.pColorBlendState = &cb;
    ci.pDynamicState = &dyn;
    ci.layout = layout_;
    ci.renderPass = renderPass;
    ci.subpass = 0;

    vkCheck(vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &ci, nullptr, &pipeline_),
            "Failed to create graphics pipeline");

    vkDestroyShaderModule(device_, vertMod, nullptr);
    vkDestroyShaderModule(device_, fragMod, nullptr);
}

VulkanRenderPipeline::~VulkanRenderPipeline() {
    if (pipeline_) vkDestroyPipeline(device_, pipeline_, nullptr);
    if (layout_) vkDestroyPipelineLayout(device_, layout_, nullptr);
}

VkFormat VulkanRenderPipeline::toVkVertexFormat(VertexFormat fmt) {
    switch (fmt) {
        case VertexFormat::Float:      return VK_FORMAT_R32_SFLOAT;
        case VertexFormat::Float2:     return VK_FORMAT_R32G32_SFLOAT;
        case VertexFormat::Float3:     return VK_FORMAT_R32G32B32_SFLOAT;
        case VertexFormat::Float4:     return VK_FORMAT_R32G32B32A32_SFLOAT;
        case VertexFormat::UByte4Norm: return VK_FORMAT_R8G8B8A8_UNORM;
    }
    return VK_FORMAT_R32G32B32A32_SFLOAT;
}

// =============================================================================
// VulkanRenderPassEncoder
// =============================================================================

VulkanRenderPassEncoder::VulkanRenderPassEncoder(VkCommandBuffer cmd, VkExtent2D extent)
    : cmd_(cmd), extent_(extent)
{
}

void VulkanRenderPassEncoder::setPipeline(RenderPipeline* pipeline) {
    auto* vp = static_cast<VulkanRenderPipeline*>(pipeline);
    vkCmdBindPipeline(cmd_, VK_PIPELINE_BIND_POINT_GRAPHICS, vp->native());

    VkViewport viewport{};
    viewport.width = static_cast<float>(extent_.width);
    viewport.height = static_cast<float>(extent_.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd_, 0, 1, &viewport);

    VkRect2D scissor{{0, 0}, extent_};
    vkCmdSetScissor(cmd_, 0, 1, &scissor);
}

void VulkanRenderPassEncoder::setVertexBuffer(uint32_t slot, Buffer* buffer, size_t offset) {
    auto* vb = static_cast<VulkanBuffer*>(buffer);
    VkBuffer b = vb->native();
    VkDeviceSize o = offset;
    vkCmdBindVertexBuffers(cmd_, slot, 1, &b, &o);
}

void VulkanRenderPassEncoder::setIndexBuffer(Buffer* buffer) {
    auto* vb = static_cast<VulkanBuffer*>(buffer);
    vkCmdBindIndexBuffer(cmd_, vb->native(), 0, VK_INDEX_TYPE_UINT16);
}

void VulkanRenderPassEncoder::setFragmentTexture(uint32_t, Texture*) {
    // TODO: descriptor set binding
}

void VulkanRenderPassEncoder::setScissorRect(uint32_t x, uint32_t y, uint32_t w, uint32_t h) {
    VkRect2D scissor{{static_cast<int32_t>(x), static_cast<int32_t>(y)}, {w, h}};
    vkCmdSetScissor(cmd_, 0, 1, &scissor);
}

void VulkanRenderPassEncoder::draw(uint32_t vertexCount, uint32_t instanceCount,
                                   uint32_t firstVertex, uint32_t firstInstance) {
    vkCmdDraw(cmd_, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanRenderPassEncoder::drawIndexed(uint32_t indexCount, uint32_t instanceCount,
                                          uint32_t firstIndex, uint32_t firstInstance) {
    vkCmdDrawIndexed(cmd_, indexCount, instanceCount, firstIndex, 0, firstInstance);
}

void VulkanRenderPassEncoder::end() {
    vkCmdEndRenderPass(cmd_);
}

// =============================================================================
// VulkanDevice — Init
// =============================================================================

VulkanDevice::VulkanDevice(SDL_Window* window) : window_(window) {
    createInstance(window);

    if (!SDL_Vulkan_CreateSurface(window_, instance_, nullptr, &surface_)) {
        throw std::runtime_error(std::string("SDL_Vulkan_CreateSurface failed: ") + SDL_GetError());
    }

    pickPhysicalDevice();
    createLogicalDevice();

    int w, h;
    SDL_GetWindowSizeInPixels(window_, &w, &h);
    createSwapchain(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
    createSyncObjects();
    createCommandBuffers();
}

VulkanDevice::~VulkanDevice() {
    if (device_) vkDeviceWaitIdle(device_);

    currentEncoder_.reset();

    for (uint32_t i = 0; i < kMaxFramesInFlight; i++) {
        if (imageAvailable_[i]) vkDestroySemaphore(device_, imageAvailable_[i], nullptr);
        if (renderFinished_[i]) vkDestroySemaphore(device_, renderFinished_[i], nullptr);
        if (inFlightFences_[i]) vkDestroyFence(device_, inFlightFences_[i], nullptr);
    }
    if (commandPool_) vkDestroyCommandPool(device_, commandPool_, nullptr);

    destroySwapchain();

    if (allocator_) vmaDestroyAllocator(allocator_);
    if (device_) vkDestroyDevice(device_, nullptr);
    if (surface_) vkDestroySurfaceKHR(instance_, surface_, nullptr);
    if (instance_) vkDestroyInstance(instance_, nullptr);
}

static bool hasExtension(const std::vector<VkExtensionProperties>& props, const char* name) {
    for (const auto& p : props)
        if (std::strcmp(p.extensionName, name) == 0) return true;
    return false;
}

void VulkanDevice::createInstance(SDL_Window* window) {
    VkApplicationInfo appInfo{VK_STRUCTURE_TYPE_APPLICATION_INFO};
    appInfo.pApplicationName = "Flux";
    appInfo.applicationVersion = VK_MAKE_VERSION(2, 0, 0);
    appInfo.pEngineName = "Flux GPU";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    uint32_t sdlExtCount = 0;
    const char* const* sdlExts = SDL_Vulkan_GetInstanceExtensions(&sdlExtCount);
    std::vector<const char*> extensions(sdlExts, sdlExts + sdlExtCount);

    uint32_t availCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &availCount, nullptr);
    std::vector<VkExtensionProperties> availExts(availCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &availCount, availExts.data());

    VkInstanceCreateInfo ci{VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
    ci.pApplicationInfo = &appInfo;

#ifdef __APPLE__
    if (hasExtension(availExts, "VK_KHR_portability_enumeration")) {
        extensions.push_back("VK_KHR_portability_enumeration");
        ci.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
    }
#endif

    ci.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    ci.ppEnabledExtensionNames = extensions.data();

    vkCheck(vkCreateInstance(&ci, nullptr, &instance_), "Failed to create Vulkan instance");
}

void VulkanDevice::pickPhysicalDevice() {
    uint32_t count = 0;
    vkEnumeratePhysicalDevices(instance_, &count, nullptr);
    if (count == 0) throw std::runtime_error("No Vulkan-capable GPU found");

    std::vector<VkPhysicalDevice> devices(count);
    vkEnumeratePhysicalDevices(instance_, &count, devices.data());

    for (auto pd : devices) {
        uint32_t qfCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &qfCount, nullptr);
        std::vector<VkQueueFamilyProperties> qfs(qfCount);
        vkGetPhysicalDeviceQueueFamilyProperties(pd, &qfCount, qfs.data());

        for (uint32_t i = 0; i < qfCount; i++) {
            VkBool32 present = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(pd, i, surface_, &present);
            if ((qfs[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && present) {
                physicalDevice_ = pd;
                graphicsFamily_ = i;
                return;
            }
        }
    }
    throw std::runtime_error("No suitable Vulkan GPU found");
}

void VulkanDevice::createLogicalDevice() {
    float priority = 1.0f;
    VkDeviceQueueCreateInfo qci{VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    qci.queueFamilyIndex = graphicsFamily_;
    qci.queueCount = 1;
    qci.pQueuePriorities = &priority;

    uint32_t devExtCount = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &devExtCount, nullptr);
    std::vector<VkExtensionProperties> devExts(devExtCount);
    vkEnumerateDeviceExtensionProperties(physicalDevice_, nullptr, &devExtCount, devExts.data());

    std::vector<const char*> deviceExts = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    if (hasExtension(devExts, "VK_KHR_portability_subset")) {
        deviceExts.push_back("VK_KHR_portability_subset");
    }

    VkDeviceCreateInfo ci{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    ci.queueCreateInfoCount = 1;
    ci.pQueueCreateInfos = &qci;
    ci.enabledExtensionCount = static_cast<uint32_t>(deviceExts.size());
    ci.ppEnabledExtensionNames = deviceExts.data();

    vkCheck(vkCreateDevice(physicalDevice_, &ci, nullptr, &device_),
            "Failed to create Vulkan device");
    vkGetDeviceQueue(device_, graphicsFamily_, 0, &graphicsQueue_);

    VmaAllocatorCreateInfo ai{};
    ai.physicalDevice = physicalDevice_;
    ai.device = device_;
    ai.instance = instance_;
    ai.vulkanApiVersion = VK_API_VERSION_1_0;
    vkCheck(vmaCreateAllocator(&ai, &allocator_), "Failed to create VMA allocator");
}

void VulkanDevice::createSwapchain(uint32_t width, uint32_t height) {
    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice_, surface_, &caps);

    uint32_t fmtCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface_, &fmtCount, nullptr);
    std::vector<VkSurfaceFormatKHR> fmts(fmtCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface_, &fmtCount, fmts.data());

    swapchainFormat_ = fmts[0].format;
    VkColorSpaceKHR colorSpace = fmts[0].colorSpace;
    for (const auto& f : fmts) {
        if (f.format == VK_FORMAT_B8G8R8A8_UNORM && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            swapchainFormat_ = f.format;
            colorSpace = f.colorSpace;
            break;
        }
    }

    swapchainExtent_ = {
        std::clamp(width, caps.minImageExtent.width, caps.maxImageExtent.width),
        std::clamp(height, caps.minImageExtent.height, caps.maxImageExtent.height)
    };

    uint32_t imageCount = caps.minImageCount + 1;
    if (caps.maxImageCount > 0) imageCount = std::min(imageCount, caps.maxImageCount);

    VkSwapchainCreateInfoKHR ci{VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR};
    ci.surface = surface_;
    ci.minImageCount = imageCount;
    ci.imageFormat = swapchainFormat_;
    ci.imageColorSpace = colorSpace;
    ci.imageExtent = swapchainExtent_;
    ci.imageArrayLayers = 1;
    ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ci.preTransform = caps.currentTransform;
    ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    ci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    ci.clipped = VK_TRUE;

    vkCheck(vkCreateSwapchainKHR(device_, &ci, nullptr, &swapchain_),
            "Failed to create swapchain");

    uint32_t imgCount = 0;
    vkGetSwapchainImagesKHR(device_, swapchain_, &imgCount, nullptr);
    swapchainImages_.resize(imgCount);
    vkGetSwapchainImagesKHR(device_, swapchain_, &imgCount, swapchainImages_.data());

    swapchainViews_.resize(imgCount);
    for (uint32_t i = 0; i < imgCount; i++) {
        VkImageViewCreateInfo vci{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
        vci.image = swapchainImages_[i];
        vci.viewType = VK_IMAGE_VIEW_TYPE_2D;
        vci.format = swapchainFormat_;
        vci.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        vkCheck(vkCreateImageView(device_, &vci, nullptr, &swapchainViews_[i]),
                "Failed to create swapchain image view");
    }

    // Render pass
    VkAttachmentDescription att{};
    att.format = swapchainFormat_;
    att.samples = VK_SAMPLE_COUNT_1_BIT;
    att.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    att.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference ref{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
    VkSubpassDescription sub{};
    sub.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    sub.colorAttachmentCount = 1;
    sub.pColorAttachments = &ref;

    VkSubpassDependency dep{};
    dep.srcSubpass = VK_SUBPASS_EXTERNAL;
    dep.dstSubpass = 0;
    dep.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo rpci{VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO};
    rpci.attachmentCount = 1;
    rpci.pAttachments = &att;
    rpci.subpassCount = 1;
    rpci.pSubpasses = &sub;
    rpci.dependencyCount = 1;
    rpci.pDependencies = &dep;

    vkCheck(vkCreateRenderPass(device_, &rpci, nullptr, &renderPass_),
            "Failed to create render pass");

    // Framebuffers
    framebuffers_.resize(imgCount);
    for (uint32_t i = 0; i < imgCount; i++) {
        VkFramebufferCreateInfo fci{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        fci.renderPass = renderPass_;
        fci.attachmentCount = 1;
        fci.pAttachments = &swapchainViews_[i];
        fci.width = swapchainExtent_.width;
        fci.height = swapchainExtent_.height;
        fci.layers = 1;
        vkCheck(vkCreateFramebuffer(device_, &fci, nullptr, &framebuffers_[i]),
                "Failed to create framebuffer");
    }
}

void VulkanDevice::destroySwapchain() {
    for (auto fb : framebuffers_) if (fb) vkDestroyFramebuffer(device_, fb, nullptr);
    framebuffers_.clear();
    if (renderPass_) { vkDestroyRenderPass(device_, renderPass_, nullptr); renderPass_ = VK_NULL_HANDLE; }
    for (auto v : swapchainViews_) if (v) vkDestroyImageView(device_, v, nullptr);
    swapchainViews_.clear();
    swapchainImages_.clear();
    if (swapchain_) { vkDestroySwapchainKHR(device_, swapchain_, nullptr); swapchain_ = VK_NULL_HANDLE; }
}

void VulkanDevice::createSyncObjects() {
    VkSemaphoreCreateInfo sci{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fci{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
    fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (uint32_t i = 0; i < kMaxFramesInFlight; i++) {
        vkCheck(vkCreateSemaphore(device_, &sci, nullptr, &imageAvailable_[i]),
                "Failed to create semaphore");
        vkCheck(vkCreateSemaphore(device_, &sci, nullptr, &renderFinished_[i]),
                "Failed to create semaphore");
        vkCheck(vkCreateFence(device_, &fci, nullptr, &inFlightFences_[i]),
                "Failed to create fence");
    }
}

void VulkanDevice::createCommandBuffers() {
    VkCommandPoolCreateInfo pci{VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO};
    pci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    pci.queueFamilyIndex = graphicsFamily_;
    vkCheck(vkCreateCommandPool(device_, &pci, nullptr, &commandPool_),
            "Failed to create command pool");

    VkCommandBufferAllocateInfo ai{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
    ai.commandPool = commandPool_;
    ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    ai.commandBufferCount = kMaxFramesInFlight;
    vkCheck(vkAllocateCommandBuffers(device_, &ai, commandBuffers_),
            "Failed to allocate command buffers");
}

// =============================================================================
// VulkanDevice — Resource creation
// =============================================================================

std::unique_ptr<Buffer> VulkanDevice::createBuffer(const BufferDesc& desc) {
    return std::make_unique<VulkanBuffer>(allocator_, desc);
}

std::unique_ptr<Texture> VulkanDevice::createTexture(const TextureDesc& desc) {
    return std::make_unique<VulkanTexture>(device_, allocator_, desc);
}

std::unique_ptr<RenderPipeline> VulkanDevice::createRenderPipeline(const RenderPipelineDesc& desc) {
    return std::make_unique<VulkanRenderPipeline>(device_, renderPass_, desc);
}

// =============================================================================
// VulkanDevice — Frame cycle
// =============================================================================

bool VulkanDevice::beginFrame() {
    vkWaitForFences(device_, 1, &inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX,
                                            imageAvailable_[currentFrame_], VK_NULL_HANDLE,
                                            &imageIndex_);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        int w, h;
        SDL_GetWindowSizeInPixels(window_, &w, &h);
        resize(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
        return false;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) return false;

    vkResetFences(device_, 1, &inFlightFences_[currentFrame_]);

    VkCommandBuffer cmd = commandBuffers_[currentFrame_];
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo bi{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
    vkCheck(vkBeginCommandBuffer(cmd, &bi), "Failed to begin command buffer");

    return true;
}

RenderPassEncoder* VulkanDevice::beginRenderPass(const RenderPassDesc& desc) {
    VkCommandBuffer cmd = commandBuffers_[currentFrame_];

    VkClearValue clearVal{};
    clearVal.color = {{desc.clearColor.r, desc.clearColor.g, desc.clearColor.b, desc.clearColor.a}};

    VkRenderPassBeginInfo rpbi{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    rpbi.renderPass = renderPass_;
    rpbi.framebuffer = framebuffers_[imageIndex_];
    rpbi.renderArea = {{0, 0}, swapchainExtent_};
    rpbi.clearValueCount = 1;
    rpbi.pClearValues = &clearVal;

    vkCmdBeginRenderPass(cmd, &rpbi, VK_SUBPASS_CONTENTS_INLINE);

    currentEncoder_ = std::make_unique<VulkanRenderPassEncoder>(cmd, swapchainExtent_);
    return currentEncoder_.get();
}

void VulkanDevice::endRenderPass() {
    if (currentEncoder_) {
        currentEncoder_->end();
        currentEncoder_.reset();
    }
}

void VulkanDevice::endFrame() {
    VkCommandBuffer cmd = commandBuffers_[currentFrame_];
    vkCheck(vkEndCommandBuffer(cmd), "Failed to end command buffer");

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    VkSubmitInfo si{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    si.waitSemaphoreCount = 1;
    si.pWaitSemaphores = &imageAvailable_[currentFrame_];
    si.pWaitDstStageMask = &waitStage;
    si.commandBufferCount = 1;
    si.pCommandBuffers = &cmd;
    si.signalSemaphoreCount = 1;
    si.pSignalSemaphores = &renderFinished_[currentFrame_];

    vkCheck(vkQueueSubmit(graphicsQueue_, 1, &si, inFlightFences_[currentFrame_]),
            "Failed to submit draw command buffer");

    VkPresentInfoKHR pi{VK_STRUCTURE_TYPE_PRESENT_INFO_KHR};
    pi.waitSemaphoreCount = 1;
    pi.pWaitSemaphores = &renderFinished_[currentFrame_];
    pi.swapchainCount = 1;
    pi.pSwapchains = &swapchain_;
    pi.pImageIndices = &imageIndex_;

    VkResult result = vkQueuePresentKHR(graphicsQueue_, &pi);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized_) {
        framebufferResized_ = false;
        int w, h;
        SDL_GetWindowSizeInPixels(window_, &w, &h);
        resize(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
    }

    currentFrame_ = (currentFrame_ + 1) % kMaxFramesInFlight;
}

void VulkanDevice::resize(uint32_t width, uint32_t height) {
    vkDeviceWaitIdle(device_);
    destroySwapchain();
    createSwapchain(width, height);
}

PixelFormat VulkanDevice::swapchainFormat() const {
    if (swapchainFormat_ == VK_FORMAT_B8G8R8A8_UNORM) return PixelFormat::BGRA8;
    return PixelFormat::RGBA8;
}

std::unique_ptr<Device> createVulkanDevice(void* sdlWindow) {
    return std::make_unique<VulkanDevice>(static_cast<SDL_Window*>(sdlWindow));
}

} // namespace flux::gpu
