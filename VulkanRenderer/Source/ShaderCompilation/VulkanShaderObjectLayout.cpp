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

slang::TypeLayoutReflection* VulkanShaderObjectLayout::getTypeLayout() const
{
	return variableLayout->getTypeLayout();
}

uint32_t VulkanShaderObjectLayout::getBindingIndex() const
{
	return variableLayout->getBindingIndex();
}

VulkanShaderObjectLayout::VulkanShaderObjectLayout(slang::VariableLayoutReflection* variableLayout, const std::vector<slang::TypeLayoutReflection*>& existentialObjectLayouts, const Renderer& app)
	: VulkanShaderObjectLayout(createLayout(variableLayout, app))
{
	this->existentialObjectLayouts = existentialObjectLayouts; // TODO: This should be changed but apparently the slang API is not yet updated for this?
	buildOffsets();
}

size_t VulkanShaderObjectLayout::getOrdinaryDataSize() const
{
	const size_t numExistentialObjects{existentialObjectLayouts.size()};
	const size_t lastSize{numExistentialObjects > 0 ? existentialObjectSizes[numExistentialObjects - 1] : getTypeLayout()->getElementVarLayout()->getTypeLayout()->getSize()};
	const size_t lastOffset{numExistentialObjects > 0 ? existentialObjectOffsets[numExistentialObjects - 1] : 0};
	return lastOffset + lastSize;
}

size_t VulkanShaderObjectLayout::getByteOffsetOfExistentialObject(const size_t& existentialObjectOffset) const
{
	return existentialObjectOffsets[existentialObjectOffset];
}

void VulkanShaderObjectLayout::buildOffsets()
{
	existentialObjectOffsets.clear();
	existentialObjectSizes.clear();
	existentialObjectOffsets.reserve(existentialObjectLayouts.size());
	existentialObjectSizes.reserve(existentialObjectLayouts.size());
	size_t currentOffset{getTypeLayout()->getElementVarLayout()->getTypeLayout()->getSize()};
	for (const auto& existentialObjectLayout : existentialObjectLayouts)
	{
		const size_t size{existentialObjectLayout->getSize()};
		existentialObjectSizes.emplace_back(size);
		existentialObjectOffsets.emplace_back(currentOffset);
		currentOffset += size;
	}
}

VulkanShaderObjectLayout VulkanShaderObjectLayout::createLayout(slang::VariableLayoutReflection* variableLayout, const Renderer& app)
{
	// TODO: This currently does not handle ParameterBlocks!
	// TODO: We don't need to support all shader stage flags

	const auto typeLayout{variableLayout->getTypeLayout()};

	std::vector<vk::DescriptorSetLayoutBinding> bindings;
	std::vector<vk::DescriptorPoolSize> poolSizes;

	const bool hasOrdinaryData = typeLayout->getSize() > 0;

	const int64_t bindingRangeCount{typeLayout->getBindingRangeCount()};
	const uint32_t totalBindingCount = bindingRangeCount + hasOrdinaryData ? 1 : 0;

	bindings.reserve(bindings.size() + totalBindingCount);
	poolSizes.reserve(poolSizes.size() + totalBindingCount);

	for (unsigned i = 0; i < bindingRangeCount; ++i)
	{
		const vk::DescriptorType descriptorType{mapDescriptorType(typeLayout->getBindingRangeType(i))};
		bindings.emplace_back(i, descriptorType, static_cast<uint32_t>(typeLayout->getBindingRangeBindingCount(i)), vk::ShaderStageFlagBits::eAll, nullptr);
		poolSizes.emplace_back(descriptorType, app.maxFramesInFlight);
	}
	if (hasOrdinaryData)
	{
		bindings.emplace_back(static_cast<uint32_t>(bindingRangeCount), vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll, nullptr);
		poolSizes.emplace_back(vk::DescriptorType::eUniformBuffer, app.maxFramesInFlight);
	}

	vk::raii::DescriptorSetLayout descriptorSetLayout{app.device, {{}, bindings}};
	vk::raii::DescriptorPool descriptorPool{app.device, {vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, app.maxFramesInFlight, poolSizes}};
	return {variableLayout, app, std::move(descriptorSetLayout), std::move(descriptorPool)};
}

VulkanShaderObjectLayout::VulkanShaderObjectLayout(slang::VariableLayoutReflection* variableLayout, const Renderer& app, vk::raii::DescriptorSetLayout&& descriptorSetLayout,
                                                   vk::raii::DescriptorPool&& descriptorPool)
	: descriptorSetLayout(std::move(descriptorSetLayout)), descriptorPool(std::move(descriptorPool)), variableLayout(variableLayout), app(app)
{
}
