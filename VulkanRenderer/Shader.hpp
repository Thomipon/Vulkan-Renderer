#pragma once

#include <vector>
#include <slang/slang-com-ptr.h>

#include "VulkanBackend.hpp"

#include "check.hpp"

inline vk::raii::ShaderModule createShaderModule(const std::vector<char>& code, const vk::raii::Device& device)
{
    const vk::ShaderModuleCreateInfo createInfo{{}, code.size(), reinterpret_cast<const uint32_t*>(code.data())};

    return device.createShaderModule(createInfo);
}

inline vk::raii::ShaderModule createShaderModule(const Slang::ComPtr<slang::IBlob>& codeBlob, const vk::raii::Device& device)
{
    const vk::ShaderModuleCreateInfo createInfo{{}, codeBlob->getBufferSize(), static_cast<const uint32_t*>(codeBlob->getBufferPointer())};
    return device.createShaderModule(createInfo);
}
