#include "Swapchain.hpp"

#include <ranges>

#include "CommandQueues.hpp"
#include "DeviceExtensions.hpp"
#include "Texture.hpp"
#include "Window.hpp"

Swapchain::Swapchain(const vk::raii::Device& device, const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface, const Window& window,
                     const QueueFamilyIndices& queueIndices, const vk::raii::SwapchainKHR& oldSwapchain)
    : Swapchain(device, getDefaultCreateInfo(physicalDevice, surface, window, queueIndices, oldSwapchain))
{
}

Swapchain::Swapchain(const vk::raii::Device& device, const vk::SwapchainCreateInfoKHR& createInfo)
    : swapchain(device, createInfo),
      images(swapchain.getImages()),
      imageFormat(createInfo.imageFormat),
      extent(createInfo.imageExtent),
      imageViews(Texture::createImageViews(device, images, imageFormat, vk::ImageAspectFlagBits::eColor)),
      swapChainFramebuffers(createFramebuffers(imageViews))
{
}

vk::SurfaceFormatKHR Swapchain::chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
    for (const auto& availableFormat : availableFormats)
    {
        if (availableFormat.format == vk::Format::eR8G8B8Srgb && availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
        {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

vk::PresentModeKHR Swapchain::choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
    for (const auto& availablePresentMode : availablePresentModes)
    {
        if (availablePresentMode == vk::PresentModeKHR::eMailbox)
        {
            return availablePresentMode;
        }
    }

    return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Swapchain::chooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities, const Window& window)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        vk::Extent2D actualExtent{window.getWindowExtent()};

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

vk::SwapchainCreateInfoKHR Swapchain::getDefaultCreateInfo(const vk::PhysicalDevice& physicalDevice, const vk::SurfaceKHR& surface, const Window& window,
                                                           const QueueFamilyIndices& queueIndices, const vk::SwapchainKHR& oldSwapchain)
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

    vk::SurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = choosePresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseExtent(swapChainSupport.capabilities, window);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR swapchainCreateInfo{
        {}, surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace, extent, 1, vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, 0,
        nullptr, swapChainSupport.capabilities.currentTransform, vk::CompositeAlphaFlagBitsKHR::eOpaque, presentMode, true, oldSwapchain
    };

    const uint32_t queueFamilyList[]{queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value()}; // TODO: This should be in inner scope but can't?
    if (queueIndices.graphicsFamily != queueIndices.presentFamily)
    {
        swapchainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapchainCreateInfo.queueFamilyIndexCount = 2;
        swapchainCreateInfo.pQueueFamilyIndices = queueFamilyList;
    }

    return swapchainCreateInfo;
}

std::vector<vk::raii::Framebuffer> Swapchain::createFramebuffers(const std::vector<vk::raii::ImageView>& imageViews)
{


}
