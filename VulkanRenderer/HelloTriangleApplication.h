#pragma once
#define GLFW_INCLUDE_VULKAN
#include <vector>
#include <GLFW/glfw3.h>

class HelloTriangleApplication
{
public:
    void run();

    inline static uint32_t width{1920};
    inline static uint32_t height{1080};
    inline static uint32_t maxFramesInFlight{2};
    
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

    std::vector<VkImage> swapChainImages{};
    VkFormat swapChainImageFormat{};
    VkExtent2D swapChainExtent{};

    std::vector<VkImageView> swapChainImageViews{};
    void createImageViews();

    VkRenderPass renderPass{VK_NULL_HANDLE};
    void createRenderPass();

    VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
    VkPipeline graphicsPipeline{VK_NULL_HANDLE};
    void createGraphicsPipeline();

    std::vector<VkFramebuffer> swapChainFramebuffers{};
    void createFramebuffers();

    VkCommandPool commandPool{VK_NULL_HANDLE};
    void createCommandPool();

    VkCommandBuffer commandBuffer{VK_NULL_HANDLE};
    void createCommandBuffer();

    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    void drawFrame();

    VkSemaphore imageAvailableSemaphore{VK_NULL_HANDLE};
    VkSemaphore renderFinishedSemaphore{VK_NULL_HANDLE};
    VkFence inFlightFence{VK_NULL_HANDLE};
    void createSyncObjects();
};
