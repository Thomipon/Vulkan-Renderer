#pragma once

#include "CommandQueues.hpp"
#include "DeviceExtensions.hpp"
#include "VulkanBackend.hpp"

inline bool isDeviceSuitableForSurface(const vk::PhysicalDevice& physDevice, const vk::SurfaceKHR& surface)
{
    vk::PhysicalDeviceFeatures deviceFeatures{physDevice.getFeatures()};

    QueueFamilyIndices queueFamilyIndices{findQueueFamilies(physDevice, surface)};

    bool extensionsSupported{CheckDeviceExtensionSupport(physDevice)};

    bool swapChainAdequate{false};
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physDevice, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return queueFamilyIndices.isComplete() && extensionsSupported && swapChainAdequate && deviceFeatures.samplerAnisotropy;
}