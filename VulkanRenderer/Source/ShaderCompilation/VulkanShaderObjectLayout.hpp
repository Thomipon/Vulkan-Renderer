#pragma once

#include <slang/slang.h>

#include "VulkanBackend.hpp"

class Renderer;

class VulkanShaderObjectLayout
{
public:
	VulkanShaderObjectLayout(slang::VariableLayoutReflection* variableLayout, const std::vector<slang::TypeLayoutReflection*>& existentialObjectLayouts, const Renderer& app);

	static vk::DescriptorType mapDescriptorType(slang::BindingType bindingType);

	vk::raii::DescriptorSetLayout descriptorSetLayout;
	vk::raii::DescriptorPool descriptorPool;

	slang::TypeLayoutReflection* getTypeLayout() const;
	uint32_t getBindingIndex() const;

	const Renderer& app;

	[[nodiscard]] size_t getOrdinaryDataSize() const;
	[[nodiscard]] size_t getByteOffsetOfExistentialObject(const size_t& existentialObjectOffset) const;

private:
	slang::VariableLayoutReflection* variableLayout;

	std::vector<slang::TypeLayoutReflection*> existentialObjectLayouts;
	std::vector<size_t> existentialObjectSizes;
	std::vector<size_t> existentialObjectOffsets;
	void buildOffsets();

	static VulkanShaderObjectLayout createLayout(slang::VariableLayoutReflection* variableLayout, const Renderer& app);

	VulkanShaderObjectLayout(slang::VariableLayoutReflection* variableLayout, const Renderer& app, vk::raii::DescriptorSetLayout&& descriptorSetLayout,
	                         vk::raii::DescriptorPool&& descriptorPool);
};
