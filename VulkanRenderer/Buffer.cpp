//
// Created by Thomas on 11.02.2025.
//

#include "Buffer.hpp"

#include "PhysicalDeviceHelper.hpp"
#include "Renderer.hpp"

Buffer::Buffer(const vk::raii::Device &device, const vk::raii::PhysicalDevice &physicalDevice, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
    : Buffer(createBuffer(device, physicalDevice, size, usage, properties))
{
}

Buffer::Buffer(const Renderer &app, vk::DeviceSize size, vk::BufferUsageFlags usage,
    vk::MemoryPropertyFlags properties)
        : Buffer(app.device, app.physicalDevice, size, usage, properties)
{
}

void Buffer::copyBufferToBuffer(const Renderer& app, const Buffer &source, const Buffer &destination, vk::DeviceSize size) {
    vk::raii::CommandBuffer commandBuffer{app.beginSingleTimeCommands()};

    vk::BufferCopy copyRegion{0, 0, size};

    commandBuffer.copyBuffer(source.vkBuffer, destination.vkBuffer, copyRegion);

    app.endSingleTimeCommands(std::move(commandBuffer));
}

Buffer::Buffer(vk::raii::Buffer &&buffer, vk::raii::DeviceMemory &&memory)
    : vkBuffer(std::move(buffer)), memory(std::move(memory))
{
}

Buffer Buffer::createBuffer(const vk::raii::Device &device, const vk::raii::PhysicalDevice &physicalDevice, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) {
    vk::BufferCreateInfo bufferCreateInfo{{}, size, usage, vk::SharingMode::eExclusive, nullptr};

    vk::raii::Buffer buffer{device, bufferCreateInfo};
    vk::MemoryRequirements memoryRequirements{buffer.getMemoryRequirements()};

    vk::MemoryAllocateInfo memoryAllocateInfo{
        memoryRequirements.size, findMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, properties)
    };
    vk::raii::DeviceMemory memory{device.allocateMemory(memoryAllocateInfo)};

    buffer.bindMemory(memory, 0);

    return {std::move(buffer), std::move(memory)};
}
