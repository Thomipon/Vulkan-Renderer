#pragma once
#include <format>
#include <stdexcept>
#include "VulkanBackend.hpp"

inline vk::Result check(vk::Result result, const std::string& message = "")
{
    if (result != vk::Result::eSuccess && result != vk::Result::eSuboptimalKHR)
        throw std::runtime_error(std::format("Check failed with error {}:\n{}", static_cast<long>(result), message));
    return result;
}
