#include "Renderer.hpp"

#include <iostream>
#include <set>

#include "check.hpp"
#include "PhysicalDeviceHelper.hpp"
#include "ValidationLayers.hpp"

Renderer::Renderer()
    : context(),
      instance(createInstance(context)),
      debugMessenger(createDebugMessenger(instance)),
      window(800, 600, onFrameBufferResized, this),
      surface(window.createWindowSurface(instance)),
      physicalDevice(pickPhysicalDevice(instance, surface)),
      queueIndices(findQueueFamilies(physicalDevice, surface)),
      device(createLogicalDevice(physicalDevice, queueIndices)),
      graphicsQueue(device.getQueue(queueIndices.graphicsFamily.value(), 0)),
      presentQueue(device.getQueue(queueIndices.presentFamily.value(), 0)),
      swapchain(device, physicalDevice, surface, window, queueIndices)
{
}

void Renderer::recreateSwapchain()
{    
    auto framebufferSize{window.getFramebufferSize()};
    while (framebufferSize.x == 0 && framebufferSize.y == 0) // TODO: This is ugly
    {
        // We are minimized, just wait
        framebufferSize = window.getFramebufferSize();
        Window::waitEvents();
    }

    device.waitIdle();
    
    swapchain = Swapchain{device, physicalDevice, surface, window, queueIndices, swapchain.swapchain};
}

void Renderer::onFrameBufferResized(GLFWwindow* window, int inWidth, int inHeight)
{
    auto* app = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window)); // TODO: This needs to be encapsulated
    app->framebufferResized = true;
}

uint32_t Renderer::findMemoryType(uint32_t typeBits, vk::MemoryPropertyFlags properties) const
{
    vk::PhysicalDeviceMemoryProperties physicalMemoryProperties{physicalDevice.getMemoryProperties()};

    for (uint32_t i = 0; i < physicalMemoryProperties.memoryTypeCount; ++i)
    {
        if ((typeBits & (1 << i)) && (physicalMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
        {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> Renderer::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) const
{
    vk::BufferCreateInfo bufferCreateInfo{{}, size, usage, vk::SharingMode::eExclusive, nullptr};

    vk::raii::Buffer buffer{device, bufferCreateInfo};
    vk::MemoryRequirements memoryRequirements{buffer.getMemoryRequirements()};

    vk::MemoryAllocateInfo memoryAllocateInfo{memoryRequirements.size, findMemoryType(memoryRequirements.memoryTypeBits, properties)};
    vk::raii::DeviceMemory memory{device.allocateMemory(memoryAllocateInfo)};

    buffer.bindMemory(memory, 0);

    return std::make_pair(std::move(buffer), std::move(memory));
}

vk::raii::CommandBuffer Renderer::beginSingleTimeCommands() const
{
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo{commandPool, vk::CommandBufferLevel::ePrimary, 1};
    vk::raii::CommandBuffer commandBuffer{std::move(device.allocateCommandBuffers(commandBufferAllocateInfo).front())};

    vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
    commandBuffer.begin(beginInfo);

    return commandBuffer;
}

void Renderer::endSingleTimeCommands(vk::raii::CommandBuffer&& commandBuffer) const
{
    commandBuffer.end();

    vk::SubmitInfo submitInfo{nullptr, nullptr, *commandBuffer, nullptr};

    graphicsQueue.submit(submitInfo, nullptr);

    graphicsQueue.waitIdle();

    // TODO: Make sure the command buffer is properly destroyed here
}

vk::raii::Instance Renderer::createInstance(const vk::raii::Context& context)
{
    if (enableValidationLayers && !checkValidationLayerSupport())
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    vk::ApplicationInfo appInfo{"Hello Triangle", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0), vk::ApiVersion14};

    const auto requiredExtensions{getRequiredExtensions()};

    const std::vector<const char*>& usedValidationLayers{enableValidationLayers ? validationLayers : std::vector<const char*>{}};
    vk::InstanceCreateInfo createInfo{{}, &appInfo, usedValidationLayers, requiredExtensions,};

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{makeDebugMessengerCreateInfo(debugCallback)};
    // TODO: Not just mega sus but also duplicate
    if constexpr (enableValidationLayers)
    {
        createInfo.pNext = &debugCreateInfo;
    }

    vk::raii::Instance instance{context, createInfo};

    {
        std::vector<vk::ExtensionProperties> extensions(vk::enumerateInstanceExtensionProperties());

        std::cout << "available extensions:\n";

        for (const auto& extension : extensions)
        {
            std::cout << '\t' << extension.extensionName << '\n';
        }
        std::cout << std::flush;
    }

    return instance;
}

vk::raii::DebugUtilsMessengerEXT Renderer::createDebugMessenger(const vk::raii::Instance& instance)
{
    if constexpr (!enableValidationLayers) return VK_NULL_HANDLE;

    return vk::raii::DebugUtilsMessengerEXT{instance, makeDebugMessengerCreateInfo(debugCallback)};
}

vk::raii::PhysicalDevice Renderer::pickPhysicalDevice(const vk::raii::Instance& instance, const vk::SurfaceKHR& surface)
{
    vk::raii::PhysicalDevices devices{instance};

    if (devices.empty())
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support");
    }

    for (const auto& physDevice : devices)
    {
        if (isDeviceSuitableForSurface(physDevice, surface))
        {
            // Pick first suitable device
            return physDevice;
        }
    }

    //if (!physicalDevice) // TODO: This is all kinda ugly
    {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

vk::raii::Device Renderer::createLogicalDevice(const vk::raii::PhysicalDevice& physicalDevice, const QueueFamilyIndices& queueIndices)
{
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value()};
    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        queueCreateInfos.emplace_back(vk::DeviceQueueCreateFlags{}, queueFamily, 1, &queuePriority);
    }

    vk::PhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = true;

    const std::vector<const char*>& usedValidationLayers{enableValidationLayers ? validationLayers : std::vector<const char*>{}};
    vk::DeviceCreateInfo createInfo{{}, queueCreateInfos, usedValidationLayers, deviceExtensions, &deviceFeatures};

    return vk::raii::Device{physicalDevice, createInfo};
}

vk::Bool32 Renderer::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity, vk::DebugUtilsMessageTypeFlagsEXT messageType,
                                   const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
    if (messageSeverity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
    {
        // Message is important enough to show
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    }

    return false;
}
