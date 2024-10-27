#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

#include "check.h"

inline VkShaderModule createShaderModule(const std::vector<char>& code, const VkDevice device)
{
    VkShaderModuleCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .codeSize = code.size(),
        .pCode = reinterpret_cast<const uint32_t*>(code.data())
    };

    VkShaderModule shaderModule;
    check(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule));

    return shaderModule;
}
