#pragma once
#include <filesystem>

#include "Image.hpp"

class TextureImage : public Image
{
public:
	TextureImage(const std::filesystem::path& path, const Renderer& app);

	vk::raii::Sampler sampler;

private:
	void generateMipMaps(const vk::Format& imageFormat, uint32_t width, uint32_t height, uint32_t mipLevels, const Renderer& app) const;

	static Image createImageFromPath(const std::filesystem::path& path, const Renderer& app);
	static vk::raii::Sampler createTextureSampler(const vk::raii::Device& device, const vk::raii::PhysicalDevice& physicalDevice);
};
