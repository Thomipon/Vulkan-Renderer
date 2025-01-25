#pragma once

#include <string>
#include <vector>

#include "VulkanBackend.hpp"
#include <GLFW/glfw3.h>

#include "Vertex.hpp"

class HelloTriangleApplication
{
public:
    void run();

    inline static uint32_t width{1920};
    inline static uint32_t height{1080};
    inline static uint32_t maxFramesInFlight{2};

    inline static std::string modelPath{"Meshes/viking_room.obj"};
    inline static std::string texturePath{"Textures/viking_room.png"};
private:
    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();

    GLFWwindow* window{nullptr};

    vk::raii::Context context;
    vk::raii::Instance instance{VK_NULL_HANDLE};
    void createInstance();
    
    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                          vk::DebugUtilsMessageTypeFlagsEXT messageType, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

    vk::raii::DebugUtilsMessengerEXT debugMessenger{VK_NULL_HANDLE};
    void setupDebugMessenger();

    vk::raii::PhysicalDevice physicalDevice{VK_NULL_HANDLE};
    void pickPhysicalDevice();
    bool isDeviceSuitable(const vk::PhysicalDevice& physDevice) const;

    vk::raii::Device device{VK_NULL_HANDLE};
    vk::raii::Queue graphicsQueue{VK_NULL_HANDLE};
    void createLogicalDevice();

    vk::raii::SurfaceKHR surface{VK_NULL_HANDLE};
    void createSurface();

    vk::raii::Queue presentQueue{VK_NULL_HANDLE};

    vk::raii::SwapchainKHR swapChain{VK_NULL_HANDLE};
    void createSwapchain();
    void recreateSwapchain();
    vk::Result checkForBadSwapchain(vk::Result inResult);

    std::vector<vk::Image> swapChainImages{}; // TODO: Does this really not need to be in ::raii ?
    vk::Format swapChainImageFormat{};
    vk::Extent2D swapChainExtent{};

    std::vector<vk::raii::ImageView> swapChainImageViews{};
    void createImageViews();

    vk::raii::RenderPass renderPass{VK_NULL_HANDLE};
    void createRenderPass();

    vk::raii::DescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
    vk::raii::PipelineLayout pipelineLayout{VK_NULL_HANDLE};
    vk::raii::Pipeline graphicsPipeline{VK_NULL_HANDLE};
    void createGraphicsPipeline();
    
    std::vector<vk::raii::Framebuffer> swapChainFramebuffers{};
    void createFramebuffers();

    vk::raii::CommandPool commandPool{VK_NULL_HANDLE};
    void createCommandPool();

    std::vector<vk::raii::CommandBuffer> commandBuffers{};
    void createCommandBuffers();

    void recordCommandBuffer(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex);

    vk::raii::CommandBuffer beginSingleTimeCommands();
    void endSindleTimeCommands(vk::raii::CommandBuffer&& commandBuffer);

    uint32_t currentFrame{0};
    void drawFrame();

    void updateUniformBuffer(uint32_t frameIndex);

    std::vector<vk::raii::Semaphore> imageAvailableSemaphores{};
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores{};
    std::vector<vk::raii::Fence> inFlightFences{};
    void createSyncObjects();

    bool frameBufferResized{false};

    static void frameBufferResizedCallback(GLFWwindow* window, int inWidth, int inHeight);

    vk::raii::Buffer vertexBuffer{VK_NULL_HANDLE};
    vk::raii::DeviceMemory vertexBufferMemory{VK_NULL_HANDLE};
    void createVertexBuffer();

    vk::raii::Buffer indexBuffer{VK_NULL_HANDLE};
    vk::raii::DeviceMemory indexBufferMemory{VK_NULL_HANDLE};
    void createIndexBuffer();

    std::vector<vk::raii::Buffer> uniformBuffers;
    std::vector<vk::raii::DeviceMemory> uniformBufferMemories;
    std::vector<void*> uniformBuffersMapped;
    void createUniformBuffers();

    std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
    void copyBuffer(vk::Buffer sourceBuffer, vk::Buffer destinationBuffer, vk::DeviceSize size);

    uint32_t findMemoryType(uint32_t typeBits, vk::MemoryPropertyFlags properties) const;

    void createDescriptorSetLayout();

    vk::raii::DescriptorPool descriptorPool{VK_NULL_HANDLE};
    void createDescriptorPool();

    std::vector<vk::raii::DescriptorSet> descriptorSets{};
    void createDescriptorSets();

    std::pair<vk::raii::Image, vk::raii::DeviceMemory> createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, uint32_t mipLevels);
    
    vk::raii::Image textureImage{VK_NULL_HANDLE};
    vk::raii::DeviceMemory textureImageMemory{VK_NULL_HANDLE};
    uint32_t textureMipLevels{0};
    void createTextureImage();

    void generateMipMaps(const vk::Image& image, int32_t width, int32_t height, uint32_t mipLevels);
    void transitionImageLayout(const vk::Image& image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels);
    void copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);

    vk::raii::ImageView textureImageView{VK_NULL_HANDLE};
    void createTextureImageView();

    vk::raii::Sampler textureSampler{VK_NULL_HANDLE};
    void createTextureSampler();
    
    vk::raii::ImageView createImageView(const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels = 1);

    vk::raii::Image depthImage{VK_NULL_HANDLE};
    vk::raii::DeviceMemory depthImageMemory{VK_NULL_HANDLE};
    vk::raii::ImageView depthImageView{VK_NULL_HANDLE};
    void createDepthResources();
    vk::Format findDepthFormat();
    static bool hasStencilComponent(vk::Format format);

    [[nodiscard]] vk::Format findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const;

    std::vector<Vertex> meshVertices{};
    std::vector<uint32_t> meshIndices{};
    void loadModel();
};
