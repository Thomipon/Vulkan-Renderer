#pragma once
#include <vector>
#include "VulkanBackend.hpp"

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

inline bool checkValidationLayerSupport()
{
    std::vector<vk::LayerProperties> availableLayers(vk::enumerateInstanceLayerProperties());

    for (const char* layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

inline std::vector<const char*> getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

// TODO: technically not necessary with Vulkan hpp
inline vk::raii::DebugUtilsMessengerEXT CreateDebugUtilsMessengerEXT(const vk::raii::Instance& instance, const vk::DebugUtilsMessengerCreateInfoEXT& createInfo,
                                                                     const vk::Optional<const vk::AllocationCallbacks>& allocator = nullptr)
{
    return vk::raii::DebugUtilsMessengerEXT{instance, createInfo, allocator};
}

// TODO: Remove
/*
inline void DestroyDebugUtilsMessengerEXT(vk::Instance instance, vk::DebugUtilsMessengerEXT debugMessenger,
                                          const vk::AllocationCallbacks pAllocator)
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr)
    {
        func(instance, debugMessenger, );
    }
}*/

inline vk::DebugUtilsMessengerCreateInfoEXT makeDebugMessengerCreateInfo(const vk::PFN_DebugUtilsMessengerCallbackEXT debugCallback)
{
    return vk::DebugUtilsMessengerCreateInfoEXT{
        {}, vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation,
        debugCallback, nullptr
    };
}
