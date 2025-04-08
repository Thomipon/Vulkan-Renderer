#include "VulkanShaderObject.hpp"

#include "Buffer.hpp"
#include "Renderer.hpp"

vk::DescriptorType mapDescriptorType(slang::BindingType bindingType)
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

void VulkanShaderObject::write(const ShaderOffset& offset, const void* data, size_t size)
{
	if (!buffer)
	{
		return;
	}
	Buffer::copySpanToBufferStaged(*app, std::span{static_cast<const std::byte*>(data), size}, *buffer, offset.byteOffset); // TODO: Support none-staged buffers
}

std::unique_ptr<VulkanShaderObject> VulkanShaderObject::create(slang::TypeLayoutReflection* typeLayout, const std::shared_ptr<Renderer>& app) // TODO: Stage flags as param
{
	std::unique_ptr<VulkanShaderObject> result{new VulkanShaderObject(typeLayout, app)};

	const bool hasOrdinaryData = typeLayout->getSize() > 0;
	if (hasOrdinaryData)
	{
		result->buffer = std::make_unique<Buffer>(*app, vk::DeviceSize{typeLayout->getSize()}, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlags{});
	}

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

	result->descriptorSetLayout = vk::raii::DescriptorSetLayout{app->device, {{}, bindings}};
	result->descriptorPool = vk::raii::DescriptorPool{app->device, {vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, app->maxFramesInFlight, poolSizes}};

	std::vector<vk::DescriptorSetLayout> layouts(app->maxFramesInFlight, result->descriptorSetLayout);
	const vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{result->descriptorPool, layouts};
	result->descriptorSets = app->device.allocateDescriptorSets(descriptorSetAllocateInfo);

	return result;
}

VulkanShaderObject::VulkanShaderObject(slang::TypeLayoutReflection* typeLayout, const std::shared_ptr<Renderer>& app)
	: typeLayout(typeLayout), app(app)
{
}
