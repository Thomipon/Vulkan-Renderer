#include "Renderer.hpp"

#include <iostream>
#include <ranges>
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
      swapchain(device, physicalDevice, surface, window, queueIndices),
      renderPass(createRenderPass(device, swapchain)),
      commandPool(createCommandPool(device, queueIndices)),
      swapChainFramebuffers(createFramebuffers(device, renderPass, , swapchain.imageViews, swapchain.extent)) {
}

void Renderer::recreateSwapchain() {
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

void Renderer::onFrameBufferResized(GLFWwindow *window, int inWidth, int inHeight) {
    auto *app = reinterpret_cast<Renderer *>(glfwGetWindowUserPointer(window)); // TODO: This needs to be encapsulated
    app->framebufferResized = true;
}

uint32_t Renderer::findMemoryType(uint32_t typeBits, vk::MemoryPropertyFlags properties) const {
    vk::PhysicalDeviceMemoryProperties physicalMemoryProperties{physicalDevice.getMemoryProperties()};

    for (uint32_t i = 0; i < physicalMemoryProperties.memoryTypeCount; ++i) {
        if ((typeBits & (1 << i)) && (physicalMemoryProperties.memoryTypes[i].propertyFlags & properties) ==
            properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> Renderer::createBuffer(
    vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties) const {
    vk::BufferCreateInfo bufferCreateInfo{{}, size, usage, vk::SharingMode::eExclusive, nullptr};

    vk::raii::Buffer buffer{device, bufferCreateInfo};
    vk::MemoryRequirements memoryRequirements{buffer.getMemoryRequirements()};

    vk::MemoryAllocateInfo memoryAllocateInfo{
        memoryRequirements.size, findMemoryType(memoryRequirements.memoryTypeBits, properties)
    };
    vk::raii::DeviceMemory memory{device.allocateMemory(memoryAllocateInfo)};

    buffer.bindMemory(memory, 0);

    return std::make_pair(std::move(buffer), std::move(memory));
}

vk::raii::CommandBuffer Renderer::beginSingleTimeCommands() const {
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo{commandPool, vk::CommandBufferLevel::ePrimary, 1};
    vk::raii::CommandBuffer commandBuffer{std::move(device.allocateCommandBuffers(commandBufferAllocateInfo).front())};

    vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
    commandBuffer.begin(beginInfo);

    return commandBuffer;
}

void Renderer::endSingleTimeCommands(vk::raii::CommandBuffer &&commandBuffer) const {
    commandBuffer.end();

    vk::SubmitInfo submitInfo{nullptr, nullptr, *commandBuffer, nullptr};

    graphicsQueue.submit(submitInfo, nullptr);

    graphicsQueue.waitIdle();

    // TODO: Make sure the command buffer is properly destroyed here
}

vk::raii::Instance Renderer::createInstance(const vk::raii::Context &context) {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    vk::ApplicationInfo appInfo{
        "Hello Triangle", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0), vk::ApiVersion14
    };

    const auto requiredExtensions{getRequiredExtensions()};

    const std::vector<const char *> &usedValidationLayers{
        enableValidationLayers ? validationLayers : std::vector<const char *>{}
    };
    vk::InstanceCreateInfo createInfo{{}, &appInfo, usedValidationLayers, requiredExtensions,};

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{makeDebugMessengerCreateInfo(debugCallback)};
    // TODO: Not just mega sus but also duplicate
    if constexpr (enableValidationLayers) {
        createInfo.pNext = &debugCreateInfo;
    }

    vk::raii::Instance instance{context, createInfo}; {
        std::vector<vk::ExtensionProperties> extensions(vk::enumerateInstanceExtensionProperties());

        std::cout << "available extensions:\n";

        for (const auto &extension: extensions) {
            std::cout << '\t' << extension.extensionName << '\n';
        }
        std::cout << std::flush;
    }

    return instance;
}

vk::raii::DebugUtilsMessengerEXT Renderer::createDebugMessenger(const vk::raii::Instance &instance) {
    if constexpr (!enableValidationLayers) return VK_NULL_HANDLE;

    return vk::raii::DebugUtilsMessengerEXT{instance, makeDebugMessengerCreateInfo(debugCallback)};
}

vk::raii::PhysicalDevice
Renderer::pickPhysicalDevice(const vk::raii::Instance &instance, const vk::SurfaceKHR &surface) {
    vk::raii::PhysicalDevices devices{instance};

    if (devices.empty()) {
        throw std::runtime_error("Failed to find GPUs with Vulkan support");
    }

    for (const auto &physDevice: devices) {
        if (isDeviceSuitableForSurface(physDevice, surface)) {
            // Pick first suitable device
            return physDevice;
        }
    }

    //if (!physicalDevice) // TODO: This is all kinda ugly
    {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

vk::raii::Device Renderer::createLogicalDevice(const vk::raii::PhysicalDevice &physicalDevice,
                                               const QueueFamilyIndices &queueIndices) {
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value()};
    float queuePriority = 1.0f;

    for (uint32_t queueFamily: uniqueQueueFamilies) {
        queueCreateInfos.emplace_back(vk::DeviceQueueCreateFlags{}, queueFamily, 1, &queuePriority);
    }

    vk::PhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = true;

    const std::vector<const char *> &usedValidationLayers{
        enableValidationLayers ? validationLayers : std::vector<const char *>{}
    };
    vk::DeviceCreateInfo createInfo{{}, queueCreateInfos, usedValidationLayers, deviceExtensions, &deviceFeatures};

    return vk::raii::Device{physicalDevice, createInfo};
}

vk::raii::CommandPool Renderer::createCommandPool(const vk::raii::Device &device,
                                                  const QueueFamilyIndices &queueIndices) {
    vk::CommandPoolCreateInfo commandPoolCreateInfo{
        vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueIndices.graphicsFamily.value()
    };

    return vk::raii::CommandPool{device, commandPoolCreateInfo};
}

vk::raii::RenderPass Renderer::createRenderPass(const vk::raii::Device &device, const Swapchain &swapchain) {
    vk::AttachmentDescription colorAttachment{
        {}, swapchain.imageFormat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR
    };
    vk::AttachmentReference colorAttachmentReference{0, vk::ImageLayout::eColorAttachmentOptimal};

    vk::AttachmentDescription depthAttachment{
        {}, findDepthFormat(), vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear,
        vk::AttachmentStoreOp::eDontCare, vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal
    };
    vk::AttachmentReference depthAttachmentReference{1, vk::ImageLayout::eDepthStencilAttachmentOptimal};

    vk::SubpassDescription subpass{
        {}, vk::PipelineBindPoint::eGraphics, 0, nullptr, 1, &colorAttachmentReference, nullptr,
        &depthAttachmentReference, 0, nullptr
    };

    vk::SubpassDependency dependency{
        vk::SubpassExternal, 0,
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        vk::AccessFlagBits::eDepthStencilAttachmentWrite,
        vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite
    };

    std::array<vk::AttachmentDescription, 2> attachments{colorAttachment, depthAttachment};
    vk::RenderPassCreateInfo renderPassCreateInfo{{}, attachments, subpass, dependency};

    return vk::raii::RenderPass{device, renderPassCreateInfo};
}

std::vector<vk::raii::Framebuffer> Renderer::createFramebuffers(const vk::raii::Device &device,
                                                                const vk::raii::RenderPass &renderPass,
                                                                const vk::raii::ImageView &depthImageView,
                                                                const std::vector<vk::raii::ImageView> &imageViews,
                                                                const vk::Extent2D &swapchainExtent) {
    auto framebuffers{
        imageViews | std::ranges::views::transform([&](const auto &imageView) -> vk::raii::Framebuffer {
            std::array<vk::ImageView, 2> attachments = {imageView, depthImageView};
            vk::FramebufferCreateInfo framebufferCreateInfo{
                {}, renderPass, attachments, swapchainExtent.width, swapchainExtent.height, 1
            };
            return vk::raii::Framebuffer{device, framebufferCreateInfo};
        })
    };
    return {framebuffers.begin(), framebuffers.end()};
}

vk::Bool32 Renderer::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                   vk::DebugUtilsMessageTypeFlagsEXT messageType,
                                   const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
    if (messageSeverity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
        // Message is important enough to show
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    }

    return false;
}
