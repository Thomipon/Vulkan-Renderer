#pragma once

#include <slang/slang.h>

#include "VulkanBackend.hpp"

class Renderer;

class VulkanShaderObjectLayout
{
public:
	VulkanShaderObjectLayout(slang::TypeLayoutReflection* typeLayout, const std::shared_ptr<Renderer>& app);

	static vk::DescriptorType mapDescriptorType(slang::BindingType bindingType);

	vk::raii::DescriptorSetLayout descriptorSetLayout;
	vk::raii::DescriptorPool descriptorPool;

	slang::TypeLayoutReflection* typeLayout;
	std::shared_ptr<Renderer> app;

private:
	static VulkanShaderObjectLayout createLayout(slang::TypeLayoutReflection* typeLayout, const std::shared_ptr<Renderer>& app);

	VulkanShaderObjectLayout(slang::TypeLayoutReflection* typeLayout, const std::shared_ptr<Renderer>& app, vk::raii::DescriptorSetLayout&& descriptorSetLayout,
	                         vk::raii::DescriptorPool&& descriptorPool);
};
