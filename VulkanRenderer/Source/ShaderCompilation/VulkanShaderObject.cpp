//
// Created by Thomas on 02/04/2025.
//

#include "VulkanShaderObject.hpp"

#include "Buffer.hpp"

void VulkanShaderObject::write(const ShaderOffset& offset, const void* data, size_t size)
{
    if (!buffer)
    {
        return;
    }
    Buffer::copySpanToBufferStaged(*app, std::span{static_cast<const std::byte*>(data), size}, *buffer);
}

std::unique_ptr<VulkanShaderObject> VulkanShaderObject::create(slang::TypeLayoutReflection *typeLayout,
    const std::shared_ptr<Renderer> &app)
{
    auto result{std::make_unique<VulkanShaderObject>(typeLayout, app)};
    if (typeLayout->getSize() > 0)
    {
        result->buffer = std::make_unique<Buffer>(*app, vk::DeviceSize{typeLayout->getSize()}, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlags{});
    }

    const auto bindingRangeCount{typeLayout->getBindingRangeCount()};
    for (unsigned i = 0; i < bindingRangeCount; ++i)
    {

    }
    return result;
}

VulkanShaderObject::VulkanShaderObject(slang::TypeLayoutReflection* typeLayout, const std::shared_ptr<Renderer> &app)
    : typeLayout(typeLayout), app(app)
{
}
