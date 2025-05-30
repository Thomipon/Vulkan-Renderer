﻿#pragma once
#include <slang/slang.h>

#include "Buffer.hpp"
#include "ShaderObject.hpp"
#include "VulkanBackend.hpp"

class VulkanShaderObjectLayout;
class Renderer;

class VulkanShaderObject : public ShaderObject
{
public:
	VulkanShaderObject(const std::shared_ptr<VulkanShaderObjectLayout>& layout);

	// TODO: We may need to treat matrices differently: For CPU targets, they need to be forced into row-major. For GPU targets, non 4x4 matrices need to be forced into certain layouts
	virtual void write(const ShaderOffset& offset, const void* data, size_t size) override;

	virtual void writeTexture(const ShaderOffset& offset, const TextureImage& texture) override;
	virtual void writeSampler(const ShaderOffset& offset, const TextureImage& texture) override;

	virtual size_t existentialToByteOffset(const size_t& existentialObjectOffset) override;
	virtual size_t existentialToBindingOffset(const size_t& existentialObjectOffset) override;

	const std::vector<vk::raii::DescriptorSet>& getDescriptorSets() const;

private:
	static VulkanShaderObject createShaderObject(const std::shared_ptr<VulkanShaderObjectLayout>& layoutObject);

	std::optional<Buffer> buffer;
	std::vector<vk::raii::DescriptorSet> descriptorSets;

	std::shared_ptr<VulkanShaderObjectLayout> layout;
	const Renderer& app;

	void initializeGlobalDescriptorSet();

	VulkanShaderObject(slang::TypeLayoutReflection* typeLayout, const std::shared_ptr<VulkanShaderObjectLayout>& layout, std::optional<Buffer>&& buffer,
	                   std::vector<vk::raii::DescriptorSet>&& descriptorSets, const Renderer& app);
};
