#pragma once
#include "ShaderObject.hpp"

class Renderer;

struct VulkanShaderObject : ShaderObject
{
public:
	virtual void write(const ShaderOffset& offset, const void* data, size_t size) override;

	static std::unique_ptr<VulkanShaderObject> create(slang::TypeLayoutReflection* typeLayout, const std::shared_ptr<Renderer>& app);
private:
	slang::TypeLayoutReflection* typeLayout{nullptr};
	std::shared_ptr<Buffer> buffer{nullptr};

	std::shared_ptr<Renderer> app;

	VulkanShaderObject(slang::TypeLayoutReflection* typeLayout, const std::shared_ptr<Renderer>& app);
};
