#pragma once
#include <span>
#include <string_view>
#include <slang/slang.h>

#include "ShaderObject.hpp"

class Buffer;

struct ShaderCursor
{
public:
	void write(const void* data, size_t size);

	template <typename T>
	void write(const std::span<T>& data);

	template <typename T>
	void write(const T& data);

	[[nodiscard]] ShaderCursor field(const char* name) const;
	[[nodiscard]] ShaderCursor field(uint32_t index) const;
	[[nodiscard]] ShaderCursor field(const std::string_view& name) const;

	[[nodiscard]] ShaderCursor element(uint32_t index) const;

private:
	std::shared_ptr<ShaderObject> shaderObject = nullptr;
	ShaderOffset offset{};

	slang::TypeLayoutReflection* typeLayout{nullptr};
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
