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

inline std::optional<vk::Format> findSupportedFormat(const vk::PhysicalDevice& physicalDevice, const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
    for (const auto& format : candidates)
    {
        vk::FormatProperties formatProperties{physicalDevice.getFormatProperties(format)};

        if (tiling == vk::ImageTiling::eLinear && (formatProperties.linearTilingFeatures & features) == features)
        {
            return format;
        }
        if (tiling == vk::ImageTiling::eOptimal && (formatProperties.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    return {};
}

inline uint32_t findMemoryType(const vk::PhysicalDevice& physicalDevice, uint32_t typeBits, vk::MemoryPropertyFlags properties) {
	vk::PhysicalDeviceMemoryProperties physicalMemoryProperties{physicalDevice.getMemoryProperties()};

	for (uint32_t i = 0; i < physicalMemoryProperties.memoryTypeCount; ++i) {
		if ((typeBits & (1 << i)) && (physicalMemoryProperties.memoryTypes[i].propertyFlags & properties) ==
			properties) {
			return i;
			}
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}