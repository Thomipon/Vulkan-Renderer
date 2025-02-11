#pragma once
#include "VulkanBackend.hpp"

class Renderer;

class Buffer
{
public:
	Buffer(const vk::raii::Device& device, const vk::raii::PhysicalDevice& physicalDevice, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
	Buffer(const Renderer& app, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

	template <typename T>
	Buffer(const Renderer& app, const std::vector<T>& source, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);


	vk::raii::Buffer vkBuffer;
	vk::raii::DeviceMemory memory;

	static void copyBufferToBuffer(const Renderer& app, const Buffer& source, const Buffer& destination, vk::DeviceSize size);

	template <typename T>
	static void copyVectorToBufferStaged(const Renderer& app, const std::vector<T>& source, const Buffer& destination);

private:
	Buffer(vk::raii::Buffer&& buffer, vk::raii::DeviceMemory&& memory);

	static Buffer createBuffer(const vk::raii::Device& device, const vk::raii::PhysicalDevice& physicalDevice, vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);

	template <typename T>
	static Buffer createBuffer(const Renderer& app, const std::vector<T>& source, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties);
};

template <typename T>
Buffer::Buffer(const Renderer& app, const std::vector<T>& source, vk::BufferUsageFlags usage,
               vk::MemoryPropertyFlags properties)
	: Buffer(createBuffer(app, source, usage, properties))
{
}

template <typename T>
void Buffer::copyVectorToBufferStaged(const Renderer& app, const std::vector<T>& source, const Buffer& destination)
{
	const VkDeviceSize bufferSize = sizeof(T) * source.size();

	Buffer stagingBuffer{app, bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible};

	void* data{stagingBuffer.memory.mapMemory(0, bufferSize, {})};
	std::memcpy(data, source.data(), bufferSize);
	stagingBuffer.memory.unmapMemory();

	copyBufferToBuffer(app, stagingBuffer, destination, bufferSize);
}

template <typename T>
Buffer Buffer::createBuffer(const Renderer& app, const std::vector<T>& source,
                            vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
{
	const VkDeviceSize size = sizeof(T) * source.size();

	Buffer buffer{app, size, vk::BufferUsageFlagBits::eTransferDst | usage, properties};
	copyVectorToBufferStaged(app, source, buffer);
	return buffer;
}
