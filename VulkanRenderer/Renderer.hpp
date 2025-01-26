#pragma once
#include <utility>

#include "CommandQueues.hpp"
#include "Swapchain.hpp"
#include "VulkanBackend.hpp"
#include "Window.hpp"

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
    vk::raii::CommandPool commandPool{VK_NULL_HANDLE};

    void recreateSwapchain();
    static void onFrameBufferResized(GLFWwindow* window, int inWidth, int inHeight);
    
    uint32_t findMemoryType(uint32_t typeBits, vk::MemoryPropertyFlags properties) const;

    std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) const;

    vk::raii::CommandBuffer beginSingleTimeCommands() const;
    void endSingleTimeCommands(vk::raii::CommandBuffer&& commandBuffer) const;

private:

    static vk::raii::Instance createInstance(const vk::raii::Context& context);
    static vk::raii::DebugUtilsMessengerEXT createDebugMessenger(const vk::raii::Instance& instance);
    static vk::raii::PhysicalDevice pickPhysicalDevice(const vk::raii::Instance& instance, const vk::SurfaceKHR& surface);
    static vk::raii::Device createLogicalDevice(const vk::raii::PhysicalDevice& physicalDevice, const QueueFamilyIndices& queueIndices);

    static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                      vk::DebugUtilsMessageTypeFlagsEXT messageType, const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

    bool framebufferResized = false; // TODO: I don't really like that this is here
};
