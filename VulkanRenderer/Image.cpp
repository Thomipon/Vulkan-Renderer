#include "Image.hpp"

#include <ranges>

#include "PhysicalDeviceHelper.hpp"
#include "Renderer.hpp"

Image::Image(const vk::raii::Device& device, const vk::PhysicalDevice& physicalDevice, uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
             vk::MemoryPropertyFlags properties,
             vk::ImageAspectFlags aspectFlags, uint32_t mipLevels, const vk::ImageViewType viewType)
	: Image(createImage(device, physicalDevice, width, height, format, tiling, usage, properties, aspectFlags, mipLevels, viewType))
{
}

Image Image::createImage(const vk::raii::Device& device, const vk::PhysicalDevice& physicalDevice, uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
                         vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels, const vk::ImageViewType viewType)
{
	const bool isCube{viewType == vk::ImageViewType::eCube || viewType == vk::ImageViewType::eCubeArray};
	const vk::ImageCreateFlags createFlags{isCube ? vk::ImageCreateFlagBits::eCubeCompatible : vk::ImageCreateFlags{}};
	const unsigned arrayLayers{isCube ? 6u : 1u};
	vk::ImageCreateInfo imageCreateInfo{
		createFlags, vk::ImageType::e2D, format, vk::Extent3D{width, height, 1}, mipLevels, arrayLayers, vk::SampleCountFlagBits::e1, tiling, usage, vk::SharingMode::eExclusive, nullptr,
		vk::ImageLayout::eUndefined
	};
	vk::raii::Image image{device, imageCreateInfo};

	vk::MemoryRequirements memoryRequirements{image.getMemoryRequirements()};
	vk::MemoryAllocateInfo memoryAllocateInfo{memoryRequirements.size, findMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, properties)};

	vk::raii::DeviceMemory imageMemory{device, memoryAllocateInfo};

	image.bindMemory(imageMemory, 0);

	vk::raii::ImageView imageView{createImageView(device, image, format, aspectFlags, mipLevels, viewType)};

	return Image{(std::move(image)), (std::move(imageMemory)), std::move(imageView), width, height, mipLevels, viewType};
}

vk::raii::ImageView Image::createImageView(const vk::raii::Device& device, const vk::Image& image, const vk::Format format, const vk::ImageAspectFlags aspectFlags, const uint32_t mipLevels, const vk::ImageViewType viewType)
{
	vk::ImageViewCreateInfo imageViewCreateInfo{
		{}, image, viewType, format, vk::ComponentMapping{}, vk::ImageSubresourceRange{aspectFlags, 0, mipLevels, 0, vk::RemainingArrayLayers} // TODO: Research what remaining array layers is
	};
	return vk::raii::ImageView{device, imageViewCreateInfo};
}

std::vector<vk::raii::ImageView> Image::createImageViews(const vk::raii::Device& device, const std::vector<vk::Image>& images, vk::Format format,
                                                         vk::ImageAspectFlags aspectFlags, uint32_t mipLevels)
{
	auto imageViews{
		images
		| std::ranges::views::transform(
			[&](const auto& image)
			{
				return Image::createImageView(device, image, format, aspectFlags, mipLevels);
			})
	};

	return {imageViews.begin(), imageViews.end()};
}

void Image::transitionImageLayout(const Renderer& app, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels) const
{
	vk::raii::CommandBuffer commandBuffer{app.beginSingleTimeCommands()};

	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;

	const bool isCube{imageViewType == vk::ImageViewType::eCube || imageViewType == vk::ImageViewType::eCubeArray};
	const unsigned arrayLayers{isCube ? 6u : 1u};

	vk::ImageMemoryBarrier barrier{
		{}, {}, oldLayout, newLayout, vk::QueueFamilyIgnored, vk::QueueFamilyIgnored, image,
		vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, arrayLayers}
	};

	if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

		if (hasStencilComponent(format))
		{
			barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
		}
	}
	else
	{
		barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
	}

	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
	{
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;
	}
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
	{
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
	}
	else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
	{
		barrier.srcAccessMask = {};
		barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
	}
	else
	{
		throw std::runtime_error("Failed to transition image layout: Unsupported layout transition");
	}

	commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, nullptr, nullptr, barrier);

	app.endSingleTimeCommands(std::move(commandBuffer));
}

void Image::copyBufferToImage(const Renderer& app, vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
{
	vk::raii::CommandBuffer commandBuffer{app.beginSingleTimeCommands()};

	vk::BufferImageCopy copyRegion{0, 0, 0, vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1}, vk::Offset3D{0, 0, 0}, vk::Extent3D{width, height, 1}};

	commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, copyRegion);

	app.endSingleTimeCommands(std::move(commandBuffer));
}

bool Image::hasStencilComponent(vk::Format format)
{
	return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

Image::Image(vk::raii::Image&& image, vk::raii::DeviceMemory&& imageMemory, vk::raii::ImageView&& imageView, uint32_t width, uint32_t height, uint32_t mipLevels, const vk::ImageViewType viewType) :
	width(width), height(height), mipLevels(mipLevels), imageViewType(viewType),
	image(std::move(image)), imageDeviceMemory(std::move(imageMemory)), imageView(std::move(imageView))
{
}
