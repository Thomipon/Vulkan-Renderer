#include "TextureImage.hpp"

#include "Buffer.hpp"
#include "Renderer.hpp"
#include "stb.hpp"

TextureImage::TextureImage(const std::filesystem::path& path, const vk::ImageViewType viewType, const Renderer& app)
	: Image(createImageFromPath(path, viewType, app)), sampler(createTextureSampler(app.device, app.physicalDevice))
{
	generateMipMaps(vk::Format::eR8G8B8A8Srgb, width, height, mipLevels, app);
}

void TextureImage::generateMipMaps(const vk::Format& imageFormat, uint32_t width, uint32_t height, uint32_t mipLevels, const Renderer& app) const
{
	if (!(app.physicalDevice.getFormatProperties(imageFormat).optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
	{
		throw std::runtime_error("Failed to generate mipmaps for image!\n Image format does not support linear blitting!");
	}
	vk::raii::CommandBuffer commandBuffer{app.beginSingleTimeCommands()};

	const bool isCube{imageViewType == vk::ImageViewType::eCube || imageViewType == vk::ImageViewType::eCubeArray};
	const unsigned arrayLayers{isCube ? 6u : 1u};

	vk::ImageMemoryBarrier barrier{
		{}, {}, vk::ImageLayout::eUndefined, vk::ImageLayout::eUndefined, vk::QueueFamilyIgnored, vk::QueueFamilyIgnored, image,
		vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, arrayLayers}
	};

	int32_t mipWidth{static_cast<int32_t>(width)};
	int32_t mipHeight{static_cast<int32_t>(height)};

	for (uint32_t i = 1; i < mipLevels; ++i)
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
		barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

		// Prepare next mip for transfer
		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, barrier);

		vk::ImageBlit blit{
			vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, i - 1, 0, arrayLayers}, std::array{vk::Offset3D{0, 0, 0}, vk::Offset3D{mipWidth, mipHeight, 1}},
			vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, i, 0, arrayLayers},
			std::array{vk::Offset3D{0, 0, 0,}, vk::Offset3D{mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1}}
		};

		// Blit image
		commandBuffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);

		barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
		barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

		// Make old mip suitable for shaders
		commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr, nullptr, barrier);

		if (mipWidth > 1)
			mipWidth /= 2;
		if (mipHeight > 1)
			mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
	barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

	// Make last mip suitable for shaders
	commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr, nullptr, barrier);

	app.endSingleTimeCommands(std::move(commandBuffer));
}

Image TextureImage::createImageFromPath(const std::filesystem::path& path, const vk::ImageViewType viewType, const Renderer& app)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels{stbi_load(path.string().c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha)};

	if (!pixels)
	{
		throw std::runtime_error("Failed to load texture image!");
	}

	const bool isCube{viewType == vk::ImageViewType::eCube || viewType == vk::ImageViewType::eCubeArray};

	if (!isCube)
	{
		vk::DeviceSize imageSize{static_cast<uint64_t>(texWidth) * texHeight * 4};
		uint32_t mipLevels{static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1};

		Buffer stagingBuffer{app, imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent};

		void* data{stagingBuffer.memory.mapMemory(0, imageSize, {})};
		std::memcpy(data, pixels, imageSize);
		stagingBuffer.memory.unmapMemory();
		stbi_image_free(pixels);

		Image image{
			app.device, app.physicalDevice, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), vk::Format::eR8G8B8A8Srgb,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc,
			// TODO: Can't we create the mips in the staging one and safe this eTransferSrc?
			vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor, mipLevels, viewType
		};

		image.transitionImageLayout(app, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels);
		copyBufferToImage(app, stagingBuffer.vkBuffer, image.image, texWidth, texHeight);

		// We do not need to transition the image layout. This is handled by generateMipMaps()
		//transitionImageLayout(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, textureMipLevels);
		return image;
	}
	else
	{
		assert(texWidth % 4 == 0);
		assert(texHeight % 3 == 0);
		assert(texWidth / 4 == texHeight / 3);

		const int sideWith{texWidth / 4};
		const int sideHeight{texHeight / 3};

		vk::DeviceSize imageSideSize{static_cast<uint64_t>(sideWith) * sideHeight * 4};
		vk::DeviceSize imageSize{imageSideSize * 6};
		uint32_t mipLevels{static_cast<uint32_t>(std::floor(std::log2(std::max(sideWith, sideHeight)))) + 1};

		Buffer stagingBuffer{app, imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent};

		void* data{stagingBuffer.memory.mapMemory(0, imageSize, {})};

		copyCubemapSide(data, pixels, CubemapSide::Front, imageSideSize, sideWith, 1, 1);
		copyCubemapSide(data, pixels, CubemapSide::Back, imageSideSize, sideWith, 3, 1);
		copyCubemapSide(data, pixels, CubemapSide::Top, imageSideSize, sideWith, 1, 0);
		copyCubemapSide(data, pixels, CubemapSide::Bottom, imageSideSize, sideWith, 1, 2);
		copyCubemapSide(data, pixels, CubemapSide::Left, imageSideSize, sideWith, 2, 1);
		copyCubemapSide(data, pixels, CubemapSide::Right, imageSideSize, sideWith, 0, 1);

		stagingBuffer.memory.unmapMemory();
		stbi_image_free(pixels);

		Image image{
			app.device, app.physicalDevice, static_cast<uint32_t>(sideWith), static_cast<uint32_t>(sideHeight), vk::Format::eR8G8B8A8Srgb,
			vk::ImageTiling::eOptimal,
			vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc,
			// TODO: Can't we create the mips in the staging one and safe this eTransferSrc?
			vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor, mipLevels, viewType
		};

		image.transitionImageLayout(app, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels);
		copyBufferToImage(app, stagingBuffer.vkBuffer, image.image, sideWith, sideHeight, 6);

		// We do not need to transition the image layout. This is handled by generateMipMaps()
		//transitionImageLayout(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, textureMipLevels);
		return image;
	}
}

vk::raii::Sampler TextureImage::createTextureSampler(const vk::raii::Device& device, const vk::raii::PhysicalDevice& physicalDevice)
{
	vk::PhysicalDeviceProperties deviceProperties{physicalDevice.getProperties()};

	vk::SamplerCreateInfo samplerInfo{
		{}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
		vk::SamplerAddressMode::eRepeat, 0.f, true, deviceProperties.limits.maxSamplerAnisotropy, false, vk::CompareOp::eAlways, 0.f, vk::LodClampNone,
		vk::BorderColor::eIntOpaqueBlack, false
	};

	return vk::raii::Sampler{device, samplerInfo};
}
