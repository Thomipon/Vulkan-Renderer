#pragma once
#include <span>
#include <string_view>
#include <slang/slang.h>
#include <vulkan/vulkan_raii.hpp>

#include "ShaderObject.hpp"

class Buffer;

struct ShaderCursor
{
public:
	ShaderCursor(ShaderObject* shaderObject);

	void write(const void* data, size_t size);

	void writeTexture(const TextureImage& texture);
	void writeSampler(const TextureImage& texture);

	template <typename T>
	void write(const std::span<T>& data);

	template <typename T>
	void write(const T& data);

	[[nodiscard]] ShaderCursor field(const char* name) const;
	[[nodiscard]] ShaderCursor field(uint32_t index) const;
	[[nodiscard]] ShaderCursor field(const std::string_view& name) const;

	[[nodiscard]] ShaderCursor element(uint32_t index) const;

	[[nodiscard]] const ShaderOffset& getOffset() const;

private:
	ShaderObject* shaderObject;
	ShaderOffset offset{};

	slang::TypeLayoutReflection* typeLayout;
};

template <typename T>
void ShaderCursor::write(const std::span<T>& data)
{
	write(data.data(), data.size());
}

template <typename T>
void ShaderCursor::write(const T& data)
{
	write(&data, sizeof(data));
}
