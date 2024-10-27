#pragma once
#include <format>
#include <stdexcept>
#include <vulkan/vulkan.h>

inline VkResult check(VkResult result, const std::string& message = "")
{
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        throw std::runtime_error(std::format("Check failed with error {}:\n{}", static_cast<long>(result), message));
    return result;
}
