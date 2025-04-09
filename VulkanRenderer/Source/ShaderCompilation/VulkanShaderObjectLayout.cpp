//
// Created by Thomas on 09/04/2025.
//

#include "VulkanShaderObjectLayout.hpp"

#include "Renderer.hpp"

vk::DescriptorType VulkanShaderObjectLayout::mapDescriptorType(slang::BindingType bindingType)
{
	switch (bindingType)
	{
	case slang::BindingType::Unknown:
		break;
	case slang::BindingType::Sampler:
		return vk::DescriptorType::eSampler;
	case slang::BindingType::Texture:
		return vk::DescriptorType::eSampledImage;
	case slang::BindingType::ConstantBuffer:
		return vk::DescriptorType::eUniformBuffer;
	case slang::BindingType::ParameterBlock:
		return vk::DescriptorType::eUniformBuffer;
	case slang::BindingType::TypedBuffer:
		break;
	case slang::BindingType::RawBuffer:
		break;
	case slang::BindingType::CombinedTextureSampler:
		return vk::DescriptorType::eCombinedImageSampler;
	case slang::BindingType::InputRenderTarget:
		break;
	case slang::BindingType::InlineUniformData:
		return vk::DescriptorType::eInlineUniformBlock;
	case slang::BindingType::RayTracingAccelerationStructure:
		return vk::DescriptorType::eAccelerationStructureKHR;
	case slang::BindingType::VaryingInput:
		break;
	case slang::BindingType::VaryingOutput:
		break;
	case slang::BindingType::ExistentialValue:
		break;
	case slang::BindingType::PushConstant:
		break;
	case slang::BindingType::MutableFlag:
		break;
	case slang::BindingType::MutableTexture:
		return vk::DescriptorType::eStorageImage;
	case slang::BindingType::MutableTypedBuffer:
		break;
	case slang::BindingType::MutableRawBuffer:
		break;
	case slang::BindingType::BaseMask:
		break;
	case slang::BindingType::ExtMask:
		break;
	}

	// TODO: Missing: eUniformTexelBuffer, eStorageTexelBuffer, eUniformBuffer, eStorageBuffer, eUniformBufferDynamic, eStorageBufferDynamic, eInputAttachment, eMutableEXT
	return vk::DescriptorType::eUniformBuffer;
}

VulkanShaderObjectLayout::VulkanShaderObjectLayout(slang::TypeLayoutReflection* typeLayout, const std::shared_ptr<Renderer>& app)
	: VulkanShaderObjectLayout(createLayout(typeLayout, app))
{
}

VulkanShaderObjectLayout VulkanShaderObjectLayout::createLayout(slang::TypeLayoutReflection* typeLayout, const std::shared_ptr<Renderer>& app)
{
	const bool hasOrdinaryData = typeLayout->getSize() > 0;

	const auto bindingRangeCount{typeLayout->getBindingRangeCount()};
	const uint32_t totalBindingCount = bindingRangeCount + hasOrdinaryData ? 1 : 0;
	std::vector<vk::DescriptorSetLayoutBinding> bindings;
	bindings.reserve(totalBindingCount);
	std::vector<vk::DescriptorPoolSize> poolSizes;
	poolSizes.reserve(totalBindingCount);
	for (unsigned i = 0; i < bindingRangeCount; ++i)
	{
		const vk::DescriptorType descriptorType{mapDescriptorType(typeLayout->getBindingRangeType(i))};
		bindings.emplace_back(i, descriptorType, static_cast<uint32_t>(typeLayout->getBindingRangeBindingCount(i)), vk::ShaderStageFlags{}, nullptr);
		poolSizes.emplace_back(descriptorType, app->maxFramesInFlight);
	}
	if (hasOrdinaryData)
	{
		bindings.emplace_back(bindingRangeCount, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlags{}, nullptr);
		poolSizes.emplace_back(vk::DescriptorType::eUniformBuffer, app->maxFramesInFlight);
	}

	vk::raii::DescriptorSetLayout descriptorSetLayout{app->device, {{}, bindings}};
	vk::raii::DescriptorPool descriptorPool{app->device, {vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, app->maxFramesInFlight, poolSizes}};
	return {typeLayout, app, std::move(descriptorSetLayout), std::move(descriptorPool)};
}

VulkanShaderObjectLayout::VulkanShaderObjectLayout(slang::TypeLayoutReflection* typeLayout, const std::shared_ptr<Renderer>& app, vk::raii::DescriptorSetLayout&& descriptorSetLayout,
	vk::raii::DescriptorPool&& descriptorPool)
		: typeLayout(typeLayout), app(app), descriptorSetLayout(std::move(descriptorSetLayout)), descriptorPool(std::move(descriptorPool))
{
}
