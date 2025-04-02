#pragma once
#include <memory>
#include <span>

#include "ShaderCursor.hpp"

class Buffer;

class ShaderObject
{
public:
	virtual void write(const ShaderOffset& offset, const void* data, size_t size) = 0;

	template <typename T>
	void write(const ShaderOffset& offset, const std::span<T>& data);

	template <typename T>
	void write(const ShaderOffset& offset, const T& data);

	virtual ~ShaderObject() = default;
private:
	std::shared_ptr<Buffer> buffer{nullptr};
	std::byte* bufferData{nullptr};
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
