#pragma once

#include <vector>
#include "VulkanBackend.hpp"

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

struct SwapChainSupportDetails
{
    vk::SurfaceCapabilitiesKHR capabilities;
    std::vector<vk::SurfaceFormatKHR> formats;
    std::vector<vk::PresentModeKHR> presentModes;
};

inline bool CheckDeviceExtensionSupport(const vk::PhysicalDevice& physicalDevice)
{
    std::vector<vk::ExtensionProperties> availableExtensions(physicalDevice.enumerateDeviceExtensionProperties());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) // TODO: Set difference is probably better
    {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

inline SwapChainSupportDetails querySwapChainSupport(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface)
{
    return SwapChainSupportDetails{
        .capabilities = device.getSurfaceCapabilitiesKHR(surface),
        .formats = device.getSurfaceFormatsKHR(surface),
        .presentModes = device.getSurfacePresentModesKHR(surface),
    };
}
