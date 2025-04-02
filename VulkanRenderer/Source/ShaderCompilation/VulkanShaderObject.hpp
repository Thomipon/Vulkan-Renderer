#pragma once
#include "ShaderObject.hpp"

struct VulkanShaderObject : ShaderObject
{
public:
	virtual void write(const ShaderOffset& offset, const void* data, size_t size) override;

private:
	slang::TypeLayoutReflection* typeLayout{nullptr};
	std::shared_ptr<Buffer> buffer{nullptr};
};
