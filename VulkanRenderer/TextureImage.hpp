#pragma once
#include <filesystem>

#include "Image.hpp"

class TextureImage : public Image
{
public:
	TextureImage(const std::filesystem::path& path, const Renderer& app);

private:
	void generateMipMaps(const vk::Format& imageFormat, int32_t width, int32_t height, uint32_t mipLevels, const Renderer& app) const;

	static TextureImage createTextureImage(const std::filesystem::path& path, const Renderer& app);

	explicit TextureImage(Image&& image);
};
