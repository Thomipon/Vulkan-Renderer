#pragma once

#include <slang/slang.h>

#include "ShaderOffset.hpp"
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
	[[nodiscard]] size_t getBindingSize() const;
	[[nodiscard]] size_t getByteOffsetOfExistentialObject(const size_t& existentialObjectOffset) const;
	[[nodiscard]] size_t getBindingOffsetOfExistentialObject(const size_t& existentialObjectOffset) const;

private:
	slang::VariableLayoutReflection* variableLayout;

	std::vector<slang::TypeLayoutReflection*> existentialObjectLayouts;
	std::vector<ShaderOffset> existentialObjectSizes;
	std::vector<ShaderOffset> existentialObjectOffsets;
	static std::pair<std::vector<ShaderOffset>, std::vector<ShaderOffset>> buildOffsets(slang::TypeLayoutReflection* typeLayout,
	                                                                                    const std::vector<slang::TypeLayoutReflection*>& existentialObjectLayouts);
	static size_t getOrdinaryDataSize(const std::vector<ShaderOffset>& existentialObjectSizes, const std::vector<ShaderOffset>& existentialObjectOffsets, slang::TypeLayoutReflection* typeLayout);
	static size_t getBindingSize(const std::vector<ShaderOffset>& existentialObjectSizes, const std::vector<ShaderOffset>& existentialObjectOffsets, slang::TypeLayoutReflection* typeLayout);

	static VulkanShaderObjectLayout createLayout(slang::VariableLayoutReflection* variableLayout, const std::vector<slang::TypeLayoutReflection*>& existentialObjectLayouts, const Renderer& app);

	VulkanShaderObjectLayout(slang::VariableLayoutReflection* variableLayout, const Renderer& app, vk::raii::DescriptorSetLayout&& descriptorSetLayout, vk::raii::DescriptorPool&& descriptorPool,
	                         const std::vector<slang::TypeLayoutReflection*>& existentialObjectLayouts, const std::vector<ShaderOffset>& existentialObjectSizes,
	                         const std::vector<ShaderOffset>& existentialObjectOffsets);
};
