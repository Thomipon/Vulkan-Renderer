#pragma once

#include <cstdint>
#include <optional>
#include <ranges>
#include <vector>

#include "VulkanBackend.hpp"

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    [[nodiscard]] bool isComplete() const
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

inline QueueFamilyIndices findQueueFamilies(const vk::PhysicalDevice& device, const vk::SurfaceKHR& surface)
{
    QueueFamilyIndices indices;

    std::vector<vk::QueueFamilyProperties> queueFamilies(device.getQueueFamilyProperties());

    // First try to find a queue with graphics and present capabilities
    auto idealFamilies{queueFamilies | std::ranges::views::enumerate | std::ranges::views::filter([&](const auto& indexedQueueFamily)
    {
        const auto& [index, queueFamily] = indexedQueueFamily;
        const bool hasGraphics{queueFamily.queueFlags & vk::QueueFlagBits::eGraphics};
        const bool hasPresent{device.getSurfaceSupportKHR(index, surface) != 0};
        return hasGraphics && hasPresent;
    })};

    if (!std::ranges::empty(idealFamilies))
    {
        uint32_t index{static_cast<uint32_t>(std::get<0>(*idealFamilies.begin()))};
        indices.graphicsFamily = index;
        indices.presentFamily = index;

        return indices;
    }

    // Fall back to separate queues
    for (const auto& [i, queueFamily] : queueFamilies | std::ranges::views::enumerate)
    {
        if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics)
        {
            indices.graphicsFamily = static_cast<uint32_t>(i);
        }
        if (device.getSurfaceSupportKHR(i, surface))
        {
            indices.presentFamily =static_cast<uint32_t>(i);
        }
    }

    return indices;
}
