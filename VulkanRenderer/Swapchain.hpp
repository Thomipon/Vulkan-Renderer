#pragma once

#include <vector>
#include "VulkanBackend.hpp"

struct QueueFamilyIndices;
class Window;

class Swapchain
{
public:
    Swapchain(const vk::raii::Device& device, const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface, const Window& window,
              const QueueFamilyIndices& queueIndices, const vk::raii::SwapchainKHR& oldSwapchain = VK_NULL_HANDLE);
    Swapchain(const vk::raii::Device& device, const vk::SwapchainCreateInfoKHR& createInfo);

    vk::raii::SwapchainKHR swapchain;
    std::vector<vk::Image> images; // TODO: Does this really not need to be in ::raii ?
    vk::Format imageFormat;
    vk::Extent2D extent;
    std::vector<vk::raii::ImageView> imageViews;
    std::vector<vk::raii::Framebuffer> swapChainFramebuffers;

private:
    static vk::SurfaceFormatKHR chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    static vk::PresentModeKHR choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    static vk::Extent2D chooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const Window& window);
    static vk::SwapchainCreateInfoKHR getDefaultCreateInfo(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface, const Window& window,
                                                           const QueueFamilyIndices& queueIndices, const vk::SwapchainKHR& oldSwapchain = VK_NULL_HANDLE);
    static std::vector<vk::raii::Framebuffer> createFramebuffers(const std::vector<vk::raii::ImageView>& imageViews);
};
