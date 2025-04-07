#pragma once
#include <utility>

#include "CommandQueues.hpp"
#include "DepthImage.hpp"
#include "Swapchain.hpp"
#include "VulkanBackend.hpp"
#include "Window.hpp"

class Scene;
class RenderSync;

class Renderer
{
public:
    Renderer();
    
    vk::raii::Context context;
    vk::raii::Instance instance;
    vk::raii::DebugUtilsMessengerEXT debugMessenger;
    Window window;
    vk::raii::SurfaceKHR surface;
    vk::raii::PhysicalDevice physicalDevice;
private:
    QueueFamilyIndices queueIndices;
public:
    vk::raii::Device device;
    vk::raii::Queue graphicsQueue;
    vk::raii::Queue presentQueue; // TODO: The queues should probably be somewhere else
    Swapchain swapchain;
	DepthImage depthImage;
    vk::raii::RenderPass renderPass;
    vk::raii::CommandPool commandPool;
    std::vector<vk::raii::CommandBuffer> commandBuffers;
    std::vector<vk::raii::Framebuffer> swapChainFramebuffers;
    std::vector<RenderSync> renderSyncObjects;

    void recreateSwapchain();
    static void onFrameBufferResized(GLFWwindow* window, int inWidth, int inHeight);

    [[nodiscard]] vk::raii::CommandBuffer beginSingleTimeCommands() const;
    void endSingleTimeCommands(vk::raii::CommandBuffer&& commandBuffer) const;

    void drawScene(const Scene& scene);

private:

    static vk::raii::Instance createInstance(const vk::raii::Context& context);
    static vk::raii::DebugUtilsMessengerEXT createDebugMessenger(const vk::raii::Instance& instance);
    static vk::raii::PhysicalDevice pickPhysicalDevice(const vk::raii::Instance& instance, const vk::SurfaceKHR& surface);
    static vk::raii::Device createLogicalDevice(const vk::raii::PhysicalDevice& physicalDevice, const QueueFamilyIndices& queueIndices);
    static vk::raii::CommandPool createCommandPool(const vk::raii::Device& device, const QueueFamilyIndices& queueIndices);
    static vk::raii::RenderPass createRenderPass(const vk::raii::Device& device, const vk::PhysicalDevice& physicalDevice, const Swapchain& swapchain);
    static std::vector<vk::raii::Framebuffer> createFramebuffers(const vk::raii::Device& device, const vk::raii::RenderPass& renderPass, const vk::raii::ImageView& depthImageView, const std::vector<vk::raii::ImageView>& imageViews, const vk::Extent2D& swapchainExtent);
    static std::vector<RenderSync> createSyncObjects(const vk::raii::Device& device, uint8_t maxFramesInFlight);

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                      vk::DebugUtilsMessageTypeFlagsEXT messageType, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

public:
    bool framebufferResized = false; // TODO: I don't really like that this is here. Also, it should not be public
};
