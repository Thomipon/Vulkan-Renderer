#pragma once

#include <vector>
#include "VulkanBackend.hpp"

#include "check.hpp"

inline vk::raii::ShaderModule createShaderModule(const std::vector<char>& code, const vk::raii::Device& device)
{
    vk::ShaderModuleCreateInfo createInfo{{}, code.size(), reinterpret_cast<const uint32_t*>(code.data())};

    return device.createShaderModule(createInfo);
}
