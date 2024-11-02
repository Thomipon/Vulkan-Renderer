#pragma once
#define GLFW_INCLUDE_VULKAN
#include <string>
#include <vector>
#include <GLFW/glfw3.h>

#include "Vertex.h"

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

    VkInstance instance{VK_NULL_HANDLE};
    void createInstance();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

    VkDebugUtilsMessengerEXT debugMessenger{};
    void setupDebugMessenger();

    VkPhysicalDevice physicalDevice{VK_NULL_HANDLE};
    void pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice physDevice) const;

    VkDevice device{VK_NULL_HANDLE};
    VkQueue graphicsQueue{VK_NULL_HANDLE};
    void createLogicalDevice();

    VkSurfaceKHR surface{VK_NULL_HANDLE};
    void createSurface();

    VkQueue presentQueue{VK_NULL_HANDLE};

    VkSwapchainKHR swapChain{VK_NULL_HANDLE};
    void createSwapchain();
    void recreateSwapchain();
    void cleanupSwapchain();
    VkResult checkForBadSwapchain(VkResult inResult);

    std::vector<VkImage> swapChainImages{};
    VkFormat swapChainImageFormat{};
    VkExtent2D swapChainExtent{};

    std::vector<VkImageView> swapChainImageViews{};
    void createImageViews();

    VkRenderPass renderPass{VK_NULL_HANDLE};
    void createRenderPass();

    VkDescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
    VkPipeline graphicsPipeline{VK_NULL_HANDLE};
    void createGraphicsPipeline();

    std::vector<VkFramebuffer> swapChainFramebuffers{};
    void createFramebuffers();

    VkCommandPool commandPool{VK_NULL_HANDLE};
    void createCommandPool();

    std::vector<VkCommandBuffer> commandBuffers{};
    void createCommandBuffers();

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    VkCommandBuffer beginSingleTimeCommands();
    void endSindleTimeCommands(VkCommandBuffer commandBuffer);

    uint32_t currentFrame{0};
    void drawFrame();

    void updateUniformBuffer(uint32_t frameIndex);

    std::vector<VkSemaphore> imageAvailableSemaphores{};
    std::vector<VkSemaphore> renderFinishedSemaphores{};
    std::vector<VkFence> inFlightFences{};
    void createSyncObjects();

    bool frameBufferResized{false};

    static void frameBufferResizedCallback(GLFWwindow* window, int inWidth, int inHeight);

    VkBuffer vertexBuffer{VK_NULL_HANDLE};
    VkDeviceMemory vertexBufferMemory{VK_NULL_HANDLE};
    void createVertexBuffer();

    VkBuffer indexBuffer{VK_NULL_HANDLE};
    VkDeviceMemory indexBufferMemory{VK_NULL_HANDLE};
    void createIndexBuffer();

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBufferMemories;
    std::vector<void*> uniformBuffersMapped;
    void createUniformBuffers();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory);
    void copyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize size);

    uint32_t findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties) const;

    void createDescriptorSetLayout();

    VkDescriptorPool descriptorPool{VK_NULL_HANDLE};
    void createDescriptorPool();

    std::vector<VkDescriptorSet> descriptorSets{};
    void createDescriptorSets();

    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    
    VkImage textureImage{VK_NULL_HANDLE};
    VkDeviceMemory textureImageMemory{VK_NULL_HANDLE};
    void createTextureImage();
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

    VkImageView textureImageView{VK_NULL_HANDLE};
    void createTextureImageView();

    VkSampler textureSampler{VK_NULL_HANDLE};
    void createTextureSampler();
    
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    VkImage depthImage{VK_NULL_HANDLE};
    VkDeviceMemory depthImageMemory{VK_NULL_HANDLE};
    VkImageView depthImageView{VK_NULL_HANDLE};
    void createDepthResources();
    VkFormat findDepthFormat();
    static bool hasStencilComponent(VkFormat format);
    
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

    std::vector<Vertex> meshVertices{};
    std::vector<uint32_t> meshIndices{};
    void loadModel();
};
