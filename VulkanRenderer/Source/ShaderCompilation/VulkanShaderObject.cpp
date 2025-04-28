#include "VulkanShaderObject.hpp"

#include "Buffer.hpp"
#include "Renderer.hpp"
#include "ShaderCursor.hpp"
#include "TextureImage.hpp"
#include "VulkanShaderObjectLayout.hpp"

VulkanShaderObject::VulkanShaderObject(const std::shared_ptr<VulkanShaderObjectLayout>& layout)
	: VulkanShaderObject(createShaderObject(layout))
{
}

void VulkanShaderObject::write(const ShaderOffset& offset, const void* data, size_t size)
{
	if (!buffer)
	{
		return;
	}
	Buffer::copySpanToBufferStaged(app, std::span{static_cast<const std::byte*>(data), size}, *buffer, offset.byteOffset); // TODO: Support non-staged buffers
}

void VulkanShaderObject::writeTexture(const ShaderOffset& offset, const TextureImage& texture)
{
	const uint32_t bindingIndex = offset.bindingIndex; //typeLayout->getBindingRangeIndexOffset(offset.bindingIndex);

	vk::DescriptorImageInfo image{texture.sampler, texture.imageView, vk::ImageLayout::eShaderReadOnlyOptimal}; // TODO: Sampler is right now here and in the sampler. TODO: Is this always the correct layout?

	std::vector<vk::WriteDescriptorSet> descriptorWrites{};
	descriptorWrites.reserve(descriptorSets.size());
	for (const auto& descriptorSet : descriptorSets)
	{
		descriptorWrites.emplace_back(descriptorSet, bindingIndex, offset.bindingArrayElement, 1,
									  vk::DescriptorType::eCombinedImageSampler/* TODO: VulkanShaderObjectLayout::mapDescriptorType(typeLayout->getBindingRangeType(bindingIndex))*/, &image);
	}
	app.device.updateDescriptorSets(descriptorWrites, nullptr);
}

void VulkanShaderObject::writeSampler(const ShaderOffset& offset, const TextureImage& texture)
{
	const uint32_t bindingIndex = offset.bindingIndex; //typeLayout->getBindingRangeIndexOffset(offset.bindingIndex);

	vk::DescriptorImageInfo image{texture.sampler};

	std::vector<vk::WriteDescriptorSet> descriptorWrites{};
	descriptorWrites.reserve(descriptorSets.size());
	for (const auto& descriptorSet : descriptorSets)
	{
		descriptorWrites.emplace_back(descriptorSet, bindingIndex, offset.bindingArrayElement, 1, VulkanShaderObjectLayout::mapDescriptorType(typeLayout->getBindingRangeType(bindingIndex)), &image);
	}
	app.device.updateDescriptorSets(descriptorWrites, {});
}

size_t VulkanShaderObject::existentialToByteOffset(const size_t& existentialObjectOffset)
{
	return layout->getByteOffsetOfExistentialObject(existentialObjectOffset);
}

size_t VulkanShaderObject::existentialToBindingOffset(const size_t& existentialObjectOffset)
{
	return layout->getBindingOffsetOfExistentialObject(existentialObjectOffset);
}

const std::vector<vk::raii::DescriptorSet>& VulkanShaderObject::getDescriptorSets() const
{
	return descriptorSets;
}

VulkanShaderObject VulkanShaderObject::createShaderObject(const std::shared_ptr<VulkanShaderObjectLayout>& layoutObject) // TODO: Stage flags as param
{
	const auto typeLayout{layoutObject->getTypeLayout()->getElementVarLayout()->getTypeLayout()};
	const bool hasOrdinaryData{layoutObject->getOrdinaryDataSize() > 0};
	std::optional<Buffer> buffer{};
	if (hasOrdinaryData)
	{
		// TODO: Give the option to have this host visible instead
		buffer = Buffer{
			layoutObject->app, vk::DeviceSize{layoutObject->getOrdinaryDataSize()}, vk::BufferUsageFlagBits::eUniformBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::MemoryPropertyFlags{}
		};
	}

	std::vector<vk::DescriptorSetLayout> layouts(layoutObject->app.maxFramesInFlight, layoutObject->descriptorSetLayout);
	const vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{layoutObject->descriptorPool, layouts};
	std::vector<vk::raii::DescriptorSet> descriptorSets{layoutObject->app.device.allocateDescriptorSets(descriptorSetAllocateInfo)};

	return {typeLayout, layoutObject, std::move(buffer), std::move(descriptorSets), layoutObject->app};
}

void VulkanShaderObject::initializeGlobalDescriptorSet()
{
	if (buffer)
	{
		const uint32_t bindingIndex = layout->getBindingIndex(); //offset.bindingIndex; //typeLayout->getBindingRangeIndexOffset(offset.bindingIndex);

		vk::DescriptorBufferInfo bufferInfo{buffer->vkBuffer, 0, layout->getOrdinaryDataSize()};

		std::vector<vk::WriteDescriptorSet> descriptorWrites{};
		descriptorWrites.reserve(descriptorSets.size());
		for (const auto& descriptorSet : descriptorSets)
		{
			descriptorWrites.emplace_back(descriptorSet, bindingIndex, 0 /* TODO: This might be needed some day */, 1,
			                              VulkanShaderObjectLayout::mapDescriptorType(typeLayout->getBindingRangeType(bindingIndex)), nullptr, &bufferInfo);
		}
		app.device.updateDescriptorSets(descriptorWrites, {});
	}
}

VulkanShaderObject::VulkanShaderObject(slang::TypeLayoutReflection* typeLayout, const std::shared_ptr<VulkanShaderObjectLayout>& layout, std::optional<Buffer>&& buffer,
                                       std::vector<vk::raii::DescriptorSet>&& descriptorSets, const Renderer& app)
	: ShaderObject(typeLayout), buffer(std::move(buffer)), descriptorSets(std::move(descriptorSets)), layout(layout), app(app)
{
	initializeGlobalDescriptorSet();
}
