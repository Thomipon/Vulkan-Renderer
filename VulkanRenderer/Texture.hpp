#pragma once
#include <iostream>

#include "VulkanBackend.hpp"

class Renderer;

class Texture
{
public:
    Texture() = default; // TODO: We probably don't want any of this default constructible but that would require more refactoring of the Application
    
    uint32_t textureMipLevels{0};

    vk::raii::Image image{VK_NULL_HANDLE};
    vk::raii::DeviceMemory imageDeviceMemory{VK_NULL_HANDLE};
    vk::raii::ImageView imageView{VK_NULL_HANDLE};

    static Texture createTextureImage(const std::string& path, const Renderer& app);
    static Texture createImage(const Renderer& app, uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
                               vk::MemoryPropertyFlags properties, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels = 1);

    void generateMipMaps(const vk::Format& imageFormat, int32_t width, int32_t height, uint32_t mipLevels, const Renderer& app) const;
    static vk::raii::ImageView createImageView(const Renderer& app, vk::Image image, vk::Format format, vk::ImageAspectFlags aspectFlags, uint32_t mipLevels = 1);
    void transitionImageLayout(const Renderer& app, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32_t mipLevels = 1) const;

    static void copyBufferToImage(const Renderer& app, vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height);

    static bool hasStencilComponent(vk::Format format);

private:
    Texture(vk::raii::Image&& image, vk::raii::DeviceMemory&& imageMemory, vk::raii::ImageView&& imageView);
};
