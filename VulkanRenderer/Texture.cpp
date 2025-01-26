#include "Texture.hpp"

#include <stb_image.h>

#include "Renderer.hpp"

Texture Texture::createTextureImage(const std::string& path, const Renderer& app)
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels{stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha)};

    if (!pixels)
    {
        throw std::runtime_error("Failed to load texture image!");
    }

    vk::DeviceSize imageSize{static_cast<uint64_t>(texWidth) * texHeight * 4};
    uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    auto [stagingBuffer, stagingBufferMemory]{
        app.createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
    };

    void* data{stagingBufferMemory.mapMemory(0, imageSize, {})};
    std::memcpy(data, pixels, imageSize);
    stagingBufferMemory.unmapMemory();
    stbi_image_free(pixels);

    Texture texture{
        createImage(app, texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
                    vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc,
                    // TODO: Can't we create the mips in the staging one and safe this eTransferSrc?
                    vk::MemoryPropertyFlagBits::eDeviceLocal, vk::ImageAspectFlagBits::eColor, mipLevels)
    };


    texture.transitionImageLayout(app, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, mipLevels);
    copyBufferToImage(app, stagingBuffer, texture.image, texWidth, texHeight);

    // We do not need to transition the image layout. This is handled by generateMipMaps()
    //transitionImageLayout(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, textureMipLevels);
    texture.generateMipMaps(vk::Format::eR8G8B8A8Srgb, texWidth, texHeight, mipLevels, app);
    return texture;
}

Texture Texture::createImage(const Renderer& app, uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                             vk::MemoryPropertyFlags properties, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels)
{
    vk::ImageCreateInfo imageCreateInfo{
        {}, vk::ImageType::e2D, format, vk::Extent3D{width, height, 1}, mipLevels, 1, vk::SampleCountFlagBits::e1, tiling, usage, vk::SharingMode::eExclusive, nullptr,
        vk::ImageLayout::eUndefined
    };
    vk::raii::Image image{app.device, imageCreateInfo};

    vk::MemoryRequirements memoryRequirements{image.getMemoryRequirements()};
    vk::MemoryAllocateInfo memoryAllocateInfo{memoryRequirements.size, app.findMemoryType(memoryRequirements.memoryTypeBits, properties)};

    vk::raii::DeviceMemory imageMemory{app.device, memoryAllocateInfo};

    image.bindMemory(imageMemory, 0);

    vk::raii::ImageView imageView{createImageView(app, image, format, aspectFlags, mipLevels)};

    return Texture{(std::move(image)), (std::move(imageMemory)), std::move(imageView)};
}

void Texture::generateMipMaps(const vk::Format& imageFormat, int32_t width, int32_t height, uint32_t mipLevels, const Renderer& app) const
{
    if (!(app.physicalDevice.getFormatProperties(imageFormat).optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
    {
        throw std::runtime_error("Failed to generate mipmaps for image!\n Image format does not support linear blitting!");
    }
    vk::raii::CommandBuffer commandBuffer{app.beginSingleTimeCommands()};

    vk::ImageMemoryBarrier barrier{
        {}, {}, vk::ImageLayout::eUndefined, vk::ImageLayout::eUndefined, vk::QueueFamilyIgnored, vk::QueueFamilyIgnored, image,
        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
    };

    int32_t mipWidth{width};
    int32_t mipHeight{height};

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
            vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, i - 1, 0, 1}, std::array{vk::Offset3D{0, 0, 0}, vk::Offset3D{mipWidth, mipHeight, 1}},
            vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, i, 0, 1},
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

vk::raii::ImageView Texture::createImageView(const Renderer& app, vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels)
{
    vk::ImageViewCreateInfo imageViewCreateInfo{
            {}, image, vk::ImageViewType::e2D, format, vk::ComponentMapping{}, vk::ImageSubresourceRange{aspectFlags, 0, mipLevels, 0, 1}
    };
    return vk::raii::ImageView{app.device, imageViewCreateInfo};
}

void Texture::transitionImageLayout(const Renderer& app, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels) const
{
        vk::raii::CommandBuffer commandBuffer{app.beginSingleTimeCommands()};

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    vk::ImageMemoryBarrier barrier{
        {}, {}, oldLayout, newLayout, vk::QueueFamilyIgnored, vk::QueueFamilyIgnored, image,
        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1}
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

void Texture::copyBufferToImage(const Renderer& app, vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
{
    vk::raii::CommandBuffer commandBuffer{app.beginSingleTimeCommands()};

    vk::BufferImageCopy copyRegion{0, 0, 0, vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1}, vk::Offset3D{0, 0, 0}, vk::Extent3D{width, height, 1}};

    commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, copyRegion);

    app.endSingleTimeCommands(std::move(commandBuffer));
}

bool Texture::hasStencilComponent(vk::Format format)
{
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

Texture::Texture(vk::raii::Image&& image, vk::raii::DeviceMemory&& imageMemory, vk::raii::ImageView&& imageView)
    : image(std::move(image)), imageDeviceMemory(std::move(imageMemory)), imageView(std::move(imageView))
{
}
