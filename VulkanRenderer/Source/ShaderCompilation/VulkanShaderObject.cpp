#include "VulkanShaderObject.hpp"

#include "Buffer.hpp"
#include "Renderer.hpp"
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
	Buffer::copySpanToBufferStaged(*app, std::span{static_cast<const std::byte*>(data), size}, *buffer, offset.byteOffset); // TODO: Support none-staged buffers
}

void VulkanShaderObject::writeTexture(const ShaderOffset& offset, const TextureImage& texture)
{
	const uint32_t bindingIndex = offset.bindingIndex; //typeLayout->getBindingRangeIndexOffset(offset.bindingIndex);

	vk::DescriptorImageInfo image{{}, texture.imageView};

	std::vector<vk::WriteDescriptorSet> descriptorWrites{};
	descriptorWrites.reserve(descriptorSets.size());
	for (const auto& descriptorSet : descriptorSets)
	{
		descriptorWrites.emplace_back(descriptorSet, bindingIndex, offset.bindingArrayElement, 1, VulkanShaderObjectLayout::mapDescriptorType(typeLayout->getBindingRangeType(bindingIndex)), &image);
	}
	app->device.updateDescriptorSets(descriptorWrites, {});
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
	app->device.updateDescriptorSets(descriptorWrites, {});
}

VulkanShaderObject VulkanShaderObject::createShaderObject(const std::shared_ptr<VulkanShaderObjectLayout>& layoutObject) // TODO: Stage flags as param
{
	const bool hasOrdinaryData = layoutObject->typeLayout->getSize() > 0;
	std::optional<Buffer> buffer{};
	if (hasOrdinaryData)
	{
		buffer = Buffer{*layoutObject->app, vk::DeviceSize{layoutObject->typeLayout->getSize()}, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlags{}};
	}

	std::vector<vk::DescriptorSetLayout> layouts(layoutObject->app->maxFramesInFlight, layoutObject->descriptorSetLayout);
	const vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{layoutObject->descriptorPool, layouts};
	std::vector<vk::raii::DescriptorSet> descriptorSets{layoutObject->app->device.allocateDescriptorSets(descriptorSetAllocateInfo)};

	return {layoutObject->typeLayout, layoutObject, std::move(buffer), std::move(descriptorSets), layoutObject->app};
}

VulkanShaderObject::VulkanShaderObject(slang::TypeLayoutReflection* typeLayout, const std::shared_ptr<VulkanShaderObjectLayout>& layout, std::optional<Buffer>&& buffer,
                                       std::vector<vk::raii::DescriptorSet>&& descriptorSets, const std::shared_ptr<const Renderer>& app)
	: ShaderObject(typeLayout), buffer(std::move(buffer)), descriptorSets(std::move(descriptorSets)), layout(layout), app(app)
{
}
