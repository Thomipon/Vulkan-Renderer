#pragma once
#include <slang/slang.h>

#include "ShaderObject.hpp"
#include "VulkanBackend.hpp"

class Renderer;

class VulkanShaderObject : ShaderObject
{
public:
	virtual void write(const ShaderOffset& offset, const void* data, size_t size) override;

	static std::unique_ptr<VulkanShaderObject> create(slang::TypeLayoutReflection* typeLayout, const std::shared_ptr<Renderer>& app);
private:
	slang::TypeLayoutReflection* typeLayout{nullptr};
	std::shared_ptr<Buffer> buffer{nullptr}; // TODO: This might not have to be a pointer
	vk::raii::DescriptorSetLayout descriptorSetLayout{nullptr};
	vk::raii::DescriptorPool descriptorPool{nullptr};
	std::vector<vk::raii::DescriptorSet> descriptorSets{};

	std::shared_ptr<Renderer> app;

	VulkanShaderObject(slang::TypeLayoutReflection* typeLayout, const std::shared_ptr<Renderer>& app);
};
