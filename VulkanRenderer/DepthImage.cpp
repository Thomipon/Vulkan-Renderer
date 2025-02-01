#include "DepthImage.hpp"

#include "PhysicalDeviceHelper.hpp"

DepthImage::DepthImage(const vk::raii::Device& device, const vk::PhysicalDevice& physicalDevice, vk::Extent2D swapchainExtent)
	: Image(device, physicalDevice, swapchainExtent.width, swapchainExtent.height, findDepthFormat(physicalDevice), vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
	        vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eDepth)
{
}

vk::Format DepthImage::findDepthFormat(const vk::PhysicalDevice& physicalDevice)
{
	const auto result{
		findSupportedFormat(physicalDevice, {vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD32Sfloat}, vk::ImageTiling::eOptimal,
		                    vk::FormatFeatureFlagBits::eDepthStencilAttachment)
	};
	if (!result)
	{
		throw std::runtime_error("failed to find depth image!");
	}
	return result.value();
}
