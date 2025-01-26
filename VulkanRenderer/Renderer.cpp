#include "Renderer.hpp"

uint32_t Renderer::findMemoryType(uint32_t typeBits, vk::MemoryPropertyFlags properties) const
{
    vk::PhysicalDeviceMemoryProperties physicalMemoryProperties{physicalDevice.getMemoryProperties()};

    for (uint32_t i = 0; i < physicalMemoryProperties.memoryTypeCount; ++i)
    {
        if ((typeBits & (1 << i)) && (physicalMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> Renderer::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) const
{
    vk::BufferCreateInfo bufferCreateInfo{{}, size, usage, vk::SharingMode::eExclusive, nullptr};

    vk::raii::Buffer buffer{device, bufferCreateInfo};
    vk::MemoryRequirements memoryRequirements{buffer.getMemoryRequirements()};

    vk::MemoryAllocateInfo memoryAllocateInfo{memoryRequirements.size, findMemoryType(memoryRequirements.memoryTypeBits, properties)};
    vk::raii::DeviceMemory memory{device.allocateMemory(memoryAllocateInfo)};

    buffer.bindMemory(memory, 0);

    return std::make_pair(std::move(buffer), std::move(memory));
}

vk::raii::CommandBuffer Renderer::beginSingleTimeCommands() const
{
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo{commandPool, vk::CommandBufferLevel::ePrimary, 1};
    vk::raii::CommandBuffer commandBuffer{std::move(device.allocateCommandBuffers(commandBufferAllocateInfo).front())};

    vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
    commandBuffer.begin(beginInfo);

    return commandBuffer;
}

void Renderer::endSingleTimeCommands(vk::raii::CommandBuffer&& commandBuffer) const
{
    commandBuffer.end();

    vk::SubmitInfo submitInfo{nullptr, nullptr, *commandBuffer, nullptr};

    graphicsQueue.submit(submitInfo, nullptr);

    graphicsQueue.waitIdle();

    // TODO: Make sure the command buffer is properly destroyed here
}
