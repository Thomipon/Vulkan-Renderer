#pragma once

#include <slang/slang.h>

#include "VulkanBackend.hpp"

class Renderer;

class VulkanShaderObjectLayout
{
public:
	VulkanShaderObjectLayout(slang::VariableLayoutReflection* variableLayout, const Renderer& app);

	static vk::DescriptorType mapDescriptorType(slang::BindingType bindingType);

	vk::raii::DescriptorSetLayout descriptorSetLayout;
	vk::raii::DescriptorPool descriptorPool;

	slang::TypeLayoutReflection* getTypeLayout() const;
	uint32_t getBindingIndex() const;

	const Renderer& app;

private:
	slang::VariableLayoutReflection* variableLayout;

	static VulkanShaderObjectLayout createLayout(slang::VariableLayoutReflection* variableLayout, const Renderer& app);

	VulkanShaderObjectLayout(slang::VariableLayoutReflection* variableLayout, const Renderer& app, vk::raii::DescriptorSetLayout&& descriptorSetLayout,
	                         vk::raii::DescriptorPool&& descriptorPool);
};
