#pragma once
#include <utility>
#include "VulkanBackend.hpp"

class Renderer
{
public:
    vk::raii::Context context;
    vk::raii::Instance instance{VK_NULL_HANDLE};
    vk::raii::Device device{VK_NULL_HANDLE};
    vk::raii::PhysicalDevice physicalDevice{VK_NULL_HANDLE};
    vk::raii::Queue graphicsQueue{VK_NULL_HANDLE};
    vk::raii::CommandPool commandPool{VK_NULL_HANDLE};


    uint32_t findMemoryType(uint32_t typeBits, vk::MemoryPropertyFlags properties) const;

    std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) const;

    vk::raii::CommandBuffer beginSingleTimeCommands() const;
    void endSingleTimeCommands(vk::raii::CommandBuffer&& commandBuffer) const;
};
