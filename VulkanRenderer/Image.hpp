#pragma once

#include "VulkanBackend.hpp"

enum class CubemapSide : uint8_t
{
	Front,
	Back,
	Top,
	Bottom,
	Right,
	Left
};

class Renderer;

class Image
{
public:
	Image(const vk::raii::Device &device, const vk::PhysicalDevice& physicalDevice, uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
	      vk::MemoryPropertyFlags properties, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels = 1, vk::ImageViewType viewType = vk::ImageViewType::e2D);

	uint32_t width;
	uint32_t height;
	uint32_t mipLevels;
	vk::ImageViewType imageViewType;

	vk::raii::Image image{VK_NULL_HANDLE};
	vk::raii::DeviceMemory imageDeviceMemory{VK_NULL_HANDLE};
	vk::raii::ImageView imageView{VK_NULL_HANDLE};

	void transitionImageLayout(const Renderer &app, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels = 1) const;

	static vk::raii::ImageView createImageView(const vk::raii::Device &device, const vk::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels = 1, vk::ImageViewType viewType = vk::ImageViewType::e2D);
	static std::vector<vk::raii::ImageView> createImageViews(const vk::raii::Device &device, const std::vector<vk::Image> &images, vk::Format format,
	                                                         vk::ImageAspectFlags aspectFlags, uint32_t mipLevels = 1);

	static void copyBufferToImage(const Renderer &app, vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height, uint32_t layerCount = 1);

	static bool hasStencilComponent(vk::Format format);

	static void copyCubemapSide(void* dstData, const void* srcData, CubemapSide side, vk::DeviceSize imageSideSize, int sideWidth, int coordinateX, int coordinateY);

private:
	Image(vk::raii::Image &&image, vk::raii::DeviceMemory &&imageMemory, vk::raii::ImageView &&imageView, uint32_t width, uint32_t height, uint32_t mipLevels, vk::ImageViewType viewType);

	static Image createImage(const vk::raii::Device& device, const vk::PhysicalDevice& physicalDevice, uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
	                         vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels = 1, vk::ImageViewType viewType = vk::ImageViewType::e2D);
};
