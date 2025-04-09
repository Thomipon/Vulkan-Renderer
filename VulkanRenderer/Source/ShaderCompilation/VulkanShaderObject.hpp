#pragma once
#include <slang/slang.h>

#include "Buffer.hpp"
#include "ShaderObject.hpp"
#include "VulkanBackend.hpp"

class VulkanShaderObjectLayout;
class Renderer;

class VulkanShaderObject : ShaderObject
{
public:
	VulkanShaderObject(const std::shared_ptr<VulkanShaderObjectLayout>& layout);

	virtual void write(const ShaderOffset& offset, const void* data, size_t size) override;

	virtual void writeTexture(const ShaderOffset& offset, const TextureImage& texture) override;
	virtual void writeSampler(const ShaderOffset& offset, const TextureImage& texture) override;

private:
	static VulkanShaderObject createShaderObject(const std::shared_ptr<VulkanShaderObjectLayout>& layoutObject);

	std::optional<Buffer> buffer;
	std::vector<vk::raii::DescriptorSet> descriptorSets;

	std::shared_ptr<VulkanShaderObjectLayout> layout;
	std::shared_ptr<Renderer> app;

	VulkanShaderObject(slang::TypeLayoutReflection* typeLayout, const std::shared_ptr<VulkanShaderObjectLayout>& layout, std::optional<Buffer>&& buffer,
	                   std::vector<vk::raii::DescriptorSet>&& descriptorSets, const std::shared_ptr<Renderer>& app);
};
