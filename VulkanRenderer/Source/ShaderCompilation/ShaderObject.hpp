#pragma once
#include <memory>
#include <span>
#include <slang/slang.h>

#include "ShaderOffset.hpp"

class TextureImage;
class Buffer;

class ShaderObject
{
public:
	virtual void write(const ShaderOffset& offset, const void* data, size_t size) = 0;

	virtual void writeTexture(const ShaderOffset& offset, const TextureImage& texture) = 0;
	virtual void writeSampler(const ShaderOffset& offset, const TextureImage& texture) = 0;

	template <typename T>
	void write(const ShaderOffset& offset, const std::span<T>& data);

	template <typename T>
	void write(const ShaderOffset& offset, const T& data);

	virtual ~ShaderObject() = default;

	slang::TypeLayoutReflection* typeLayout;

protected:
	explicit ShaderObject(slang::TypeLayoutReflection* typeLayout);
};

template <typename T>
void ShaderObject::write(const ShaderOffset& offset, const std::span<T>& data)
{
	write(offset, data.data(), data.size());
}

template <typename T>
void ShaderObject::write(const ShaderOffset& offset, const T& data)
{
	write(offset, &data, sizeof(data));
}
