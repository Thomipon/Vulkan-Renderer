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
	: VulkanShaderObjectLayout(createLayout(variableLayout, existentialObjectLayouts, app))
{
	// TODO: Existential object handling should be changed but apparently the slang API is not yet updated for this?
}

size_t VulkanShaderObjectLayout::getOrdinaryDataSize() const
{
	return getOrdinaryDataSize(existentialObjectSizes, existentialObjectOffsets, getTypeLayout());
}

size_t VulkanShaderObjectLayout::getBindingSize() const
{
	return getBindingSize(existentialObjectSizes, existentialObjectOffsets, getTypeLayout());
}

size_t VulkanShaderObjectLayout::getByteOffsetOfExistentialObject(const size_t& existentialObjectOffset) const
{
	return existentialObjectOffsets[existentialObjectOffset].byteOffset;
}

size_t VulkanShaderObjectLayout::getBindingOffsetOfExistentialObject(const size_t& existentialObjectOffset) const
{
	return existentialObjectOffsets[existentialObjectOffset].bindingIndex;
}

std::pair<std::vector<ShaderOffset>, std::vector<ShaderOffset>> VulkanShaderObjectLayout::buildOffsets(slang::TypeLayoutReflection* typeLayout,
                                                                                                       const std::vector<slang::TypeLayoutReflection*>& existentialObjectLayouts)
{
	std::vector<ShaderOffset> existentialObjectOffsets{};
	std::vector<ShaderOffset> existentialObjectSizes{};
	existentialObjectOffsets.reserve(existentialObjectLayouts.size());
	existentialObjectSizes.reserve(existentialObjectLayouts.size());
	size_t currentByteOffset{typeLayout->getElementVarLayout()->getTypeLayout()->getSize()};
	int64_t currentBindingOffset{typeLayout->getBindingRangeCount()};
	for (const auto& existentialObjectLayout : existentialObjectLayouts)
	{
		const size_t byteSize{existentialObjectLayout->getSize()};
		const int64_t bindingSize{existentialObjectLayout->getBindingRangeCount()};
		existentialObjectSizes.emplace_back(byteSize, bindingSize);
		existentialObjectOffsets.emplace_back(currentByteOffset, currentBindingOffset);
		currentByteOffset += byteSize;
		currentBindingOffset += bindingSize;
	}
	return {existentialObjectOffsets, existentialObjectSizes};
}

size_t VulkanShaderObjectLayout::getOrdinaryDataSize(const std::vector<ShaderOffset>& existentialObjectSizes, const std::vector<ShaderOffset>& existentialObjectOffsets,
                                                     slang::TypeLayoutReflection* typeLayout)
{
	const size_t numExistentialObjects{existentialObjectSizes.size()};
	const size_t lastSize{numExistentialObjects > 0 ? existentialObjectSizes[numExistentialObjects - 1].byteOffset : typeLayout->getElementVarLayout()->getTypeLayout()->getSize()};
	const size_t lastOffset{numExistentialObjects > 0 ? existentialObjectOffsets[numExistentialObjects - 1].byteOffset : 0};
	return lastOffset + lastSize;
}

size_t VulkanShaderObjectLayout::getBindingSize(const std::vector<ShaderOffset>& existentialObjectSizes, const std::vector<ShaderOffset>& existentialObjectOffsets,
                                                slang::TypeLayoutReflection* typeLayout)
{
	const size_t numExistentialObjects{existentialObjectSizes.size()};
	const size_t lastSize{
		numExistentialObjects > 0 ? existentialObjectSizes[numExistentialObjects - 1].bindingIndex : static_cast<size_t>(typeLayout->getElementVarLayout()->getTypeLayout()->getBindingRangeCount())
	};
	const size_t lastOffset{numExistentialObjects > 0 ? existentialObjectOffsets[numExistentialObjects - 1].bindingIndex : 0};
	return lastOffset + lastSize;
}

VulkanShaderObjectLayout VulkanShaderObjectLayout::createLayout(slang::VariableLayoutReflection* variableLayout, const std::vector<slang::TypeLayoutReflection*>& existentialObjectLayouts,
                                                                const Renderer& app)
{
	// TODO: This currently does not handle ParameterBlocks!
	// TODO: We don't need to support all shader stage flags

	const auto typeLayout{variableLayout->getTypeLayout()};

	auto [existentialObjectOffsets, existentialObjectSizes] = buildOffsets(typeLayout, existentialObjectLayouts);

	std::vector<vk::DescriptorSetLayoutBinding> bindings;
	std::vector<vk::DescriptorPoolSize> poolSizes;

	const bool hasOrdinaryData = typeLayout->getSize() > 0; // TODO: Should this consider existential values?

	const int64_t bindingRangeCount{typeLayout->getBindingRangeCount()};
	const uint32_t totalBindingCount = getBindingSize(existentialObjectSizes, existentialObjectOffsets, typeLayout) + (hasOrdinaryData ? 1 : 0);

	bindings.reserve(totalBindingCount);
	poolSizes.reserve(totalBindingCount);

	for (unsigned i = 0; i < bindingRangeCount; ++i)
	{
		const vk::DescriptorType descriptorType{mapDescriptorType(typeLayout->getBindingRangeType(i))};
		bindings.emplace_back(i, descriptorType, static_cast<uint32_t>(typeLayout->getBindingRangeBindingCount(i)), vk::ShaderStageFlagBits::eAll, nullptr);
		poolSizes.emplace_back(descriptorType, app.maxFramesInFlight);
	}
	unsigned currentBindingIndex{static_cast<uint32_t>(bindingRangeCount)};
	if (hasOrdinaryData)
	{
		bindings.emplace_back(currentBindingIndex, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll, nullptr);
		poolSizes.emplace_back(vk::DescriptorType::eUniformBuffer, app.maxFramesInFlight);
		++currentBindingIndex;
	}
	for (unsigned j = 0; j < existentialObjectLayouts.size(); ++j)
	{
		slang::TypeLayoutReflection* existentialObjectLayout{existentialObjectLayouts[j]};
		for (unsigned i = 0; i < existentialObjectSizes[j].bindingIndex; ++i)
		{
			const vk::DescriptorType descriptorType{mapDescriptorType(existentialObjectLayout->getBindingRangeType(i))};
			bindings.emplace_back(currentBindingIndex, descriptorType, static_cast<uint32_t>(existentialObjectLayout->getBindingRangeBindingCount(i)), vk::ShaderStageFlagBits::eAll, nullptr);
			poolSizes.emplace_back(descriptorType, app.maxFramesInFlight);
			++currentBindingIndex;
		}
	}

	vk::raii::DescriptorSetLayout descriptorSetLayout{app.device, {{}, bindings}};
	vk::raii::DescriptorPool descriptorPool{app.device, {vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, app.maxFramesInFlight, poolSizes}};
	return {variableLayout, app, std::move(descriptorSetLayout), std::move(descriptorPool), existentialObjectLayouts, existentialObjectSizes, existentialObjectOffsets};
}

VulkanShaderObjectLayout::VulkanShaderObjectLayout(slang::VariableLayoutReflection* variableLayout, const Renderer& app, vk::raii::DescriptorSetLayout&& descriptorSetLayout,
                                                   vk::raii::DescriptorPool&& descriptorPool, const std::vector<slang::TypeLayoutReflection*>& existentialObjectLayouts,
                                                   const std::vector<ShaderOffset>& existentialObjectSizes, const std::vector<ShaderOffset>& existentialObjectOffsets)
	: descriptorSetLayout(std::move(descriptorSetLayout)), descriptorPool(std::move(descriptorPool)), app(app), variableLayout(variableLayout), existentialObjectLayouts(existentialObjectLayouts),
	  existentialObjectSizes(existentialObjectSizes), existentialObjectOffsets(existentialObjectOffsets)
{
}
