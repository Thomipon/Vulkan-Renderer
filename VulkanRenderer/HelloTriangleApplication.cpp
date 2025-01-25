#include "HelloTriangleApplication.hpp"

#include <chrono>
#include <iostream>
#include <set>
#include <vector>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/gtc/matrix_transform.hpp>

#include "check.hpp"
#include "CommandQueues.hpp"
#include "DeviceExtensions.hpp"
#include "IOHelper.hpp"
#include "Shader.hpp"
#include "SwapChain.hpp"
#include "Uniforms.hpp"
#include "ValidationLayers.hpp"
#include "Vertex.hpp"
#include "Image.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <unordered_map>

#include "tiny_obj_loader.h"

void HelloTriangleApplication::run()
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void HelloTriangleApplication::initWindow()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, frameBufferResizedCallback);
}

void HelloTriangleApplication::initVulkan()
{
    createInstance();
    setupDebugMessenger();
    createSurface();

    pickPhysicalDevice();
    createLogicalDevice();
    createSwapchain();

    createImageViews();
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();

    createCommandPool();

    createDepthResources();
    createFramebuffers();

    loadModel();

    createTextureImage();
    createTextureImageView();
    createTextureSampler();

    createVertexBuffer();
    createIndexBuffer();

    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();

    createCommandBuffers();
    createSyncObjects();
}

void HelloTriangleApplication::mainLoop()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        drawFrame();
    }

    device.waitIdle();
}

void HelloTriangleApplication::cleanup()
{
    // TODO: Wrap this somewhere
    glfwDestroyWindow(window);
    glfwTerminate();
}

void HelloTriangleApplication::createInstance()
{
    if (enableValidationLayers && !checkValidationLayerSupport())
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    vk::ApplicationInfo appInfo{"Hello Triangle", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0), vk::ApiVersion13};

    const auto requiredExtensions{getRequiredExtensions()};

    const std::vector<const char*>& usedValidationLayers{enableValidationLayers ? validationLayers : std::vector<const char*>{}};
    vk::InstanceCreateInfo createInfo{{}, &appInfo, usedValidationLayers, requiredExtensions,};

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{makeDebugMessengerCreateInfo(reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(debugCallback))};
    // TODO: Not just mega sus but also duplicate
    if constexpr (enableValidationLayers)
    {
        createInfo.pNext = &debugCreateInfo;
    }

    instance = vk::raii::Instance{context, createInfo};

    {
        std::vector<vk::ExtensionProperties> extensions(vk::enumerateInstanceExtensionProperties());

        std::cout << "available extensions:\n";

        for (const auto& extension : extensions)
        {
            std::cout << '\t' << extension.extensionName << '\n';
        }
        std::cout << std::flush;
    }
}

vk::Bool32 HelloTriangleApplication::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                   vk::DebugUtilsMessageTypeFlagsEXT messageType,
                                                   const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                   void* pUserData)
{
    if (messageSeverity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
    {
        // Message is important enough to show
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    }

    return false;
}

void HelloTriangleApplication::setupDebugMessenger()
{
    if constexpr (!enableValidationLayers) return;

    vk::DebugUtilsMessengerCreateInfoEXT createInfo{makeDebugMessengerCreateInfo(reinterpret_cast<PFN_vkDebugUtilsMessengerCallbackEXT>(debugCallback))};
    // TODO: This is MEGA SUS

    debugMessenger = CreateDebugUtilsMessengerEXT(instance, createInfo);
}

void HelloTriangleApplication::pickPhysicalDevice()
{
    vk::raii::PhysicalDevices devices{instance};

    if (devices.empty())
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support");
    }

    for (const auto& physDevice : devices)
    {
        if (isDeviceSuitable(physDevice))
        {
            physicalDevice = physDevice;
            return; // Pick first suitable device
        }
    }

    //if (!physicalDevice) // TODO: This is all kinda ugly
    {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

bool HelloTriangleApplication::isDeviceSuitable(const vk::PhysicalDevice& physDevice) const
{
    vk::PhysicalDeviceProperties deviceProperties{physDevice.getProperties()};
    vk::PhysicalDeviceFeatures deviceFeatures{physDevice.getFeatures()};

    QueueFamilyIndices queueFamilyIndices{findQueueFamilies(physDevice, surface)};

    bool extensionsSupported{CheckDeviceExtensionSupport(physDevice)};

    bool swapChainAdequate{false};
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physDevice, surface);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return queueFamilyIndices.isComplete() && extensionsSupported && swapChainAdequate && deviceFeatures.samplerAnisotropy;
}

void HelloTriangleApplication::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        queueCreateInfos.emplace_back(vk::DeviceQueueCreateFlags{}, queueFamily, 1, &queuePriority);
    }

    vk::PhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = true;

    const std::vector<const char*>& usedValidationLayers{enableValidationLayers ? validationLayers : std::vector<const char*>{}};
    vk::DeviceCreateInfo createInfo{{}, queueCreateInfos, usedValidationLayers, deviceExtensions, &deviceFeatures};

    device = vk::raii::Device{physicalDevice, createInfo};
    graphicsQueue = device.getQueue(indices.graphicsFamily.value(), 0);
    presentQueue = device.getQueue(indices.presentFamily.value(), 0);
}

void HelloTriangleApplication::createSurface()
{
    VkSurfaceKHR rawSurface{VK_NULL_HANDLE};
    check(static_cast<vk::Result>(glfwCreateWindowSurface(*instance, window, nullptr, &rawSurface)), "Failed to create window surface");

    surface = vk::raii::SurfaceKHR{instance, rawSurface};
}

void HelloTriangleApplication::createSwapchain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

    vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    vk::PresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    vk::Extent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    vk::SwapchainCreateInfoKHR swapChainCreateInfo{
        {}, surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace, extent, 1, vk::ImageUsageFlagBits::eColorAttachment, vk::SharingMode::eExclusive, 0,
        nullptr, swapChainSupport.capabilities.currentTransform, vk::CompositeAlphaFlagBitsKHR::eOpaque, presentMode, true, swapChain
    };

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

    const uint32_t queueFamilyList[]{indices.graphicsFamily.value(), indices.presentFamily.value()}; // TODO: This should be in inner scope but can't?
    if (indices.graphicsFamily != indices.presentFamily)
    {
        swapChainCreateInfo.imageSharingMode = vk::SharingMode::eConcurrent;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyList;
    }

    swapChain = vk::raii::SwapchainKHR{device, swapChainCreateInfo};
    swapChainImages = swapChain.getImages(); // TODO: raii maybe

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void HelloTriangleApplication::recreateSwapchain()
{
    int winWidth, winHeight;
    glfwGetFramebufferSize(window, &winWidth, &winHeight);
    while (winWidth == 0 || winHeight == 0) // TODO: This is ugly
    {
        // We are minimized, just wait
        glfwGetFramebufferSize(window, &winWidth, &winHeight);
        glfwWaitEvents();
    }

    device.waitIdle();

    createSwapchain();
    createImageViews();
    createDepthResources();
    createFramebuffers();
}

vk::Result HelloTriangleApplication::checkForBadSwapchain(vk::Result inResult)
{
    if (inResult == vk::Result::eErrorOutOfDateKHR)
    {
        recreateSwapchain();
        frameBufferResized = false;
        return inResult;
    }
    if (inResult == vk::Result::eSuboptimalKHR)
    {
        return inResult;
    }
    return check(inResult);
}

void HelloTriangleApplication::createImageViews()
{
    swapChainImageViews.clear();
    swapChainImageViews.reserve(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        swapChainImageViews.emplace_back(createImageView(swapChainImages[i], swapChainImageFormat, vk::ImageAspectFlagBits::eColor));
    }
}

void HelloTriangleApplication::createRenderPass()
{
    vk::AttachmentDescription colorAttachment{
        {}, swapChainImageFormat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR
    };
    vk::AttachmentReference colorAttachmentReference{0, vk::ImageLayout::eColorAttachmentOptimal};

    vk::AttachmentDescription depthAttachment{
        {}, findDepthFormat(), vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare, vk::AttachmentLoadOp::eDontCare,
        vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal
    };
    vk::AttachmentReference depthAttachmentReference{1, vk::ImageLayout::eDepthStencilAttachmentOptimal};

    vk::SubpassDescription subpass{
        {}, vk::PipelineBindPoint::eGraphics, 0, nullptr, 1, &colorAttachmentReference, nullptr,
        &depthAttachmentReference, 0, nullptr
    };

    vk::SubpassDependency dependency{
        vk::SubpassExternal, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests,
        vk::PipelineStageFlagBits::eColorAttachmentOutput | vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::AccessFlagBits::eDepthStencilAttachmentWrite,
        vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentWrite
    };

    std::array<vk::AttachmentDescription, 2> attachments{colorAttachment, depthAttachment};
    vk::RenderPassCreateInfo renderPassCreateInfo{{}, attachments, subpass, dependency};

    renderPass = vk::raii::RenderPass{device, renderPassCreateInfo};
}

void HelloTriangleApplication::createGraphicsPipeline()
{
    auto vertShaderCode{readFile("Shaders/vert.spv")};
    auto fragShaderCode{readFile("Shaders/frag.spv")};

    vk::raii::ShaderModule vertShaderModule{createShaderModule(vertShaderCode, device)};
    vk::raii::ShaderModule fragShaderModule{createShaderModule(fragShaderCode, device)};

    vk::PipelineShaderStageCreateInfo vertShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main", nullptr};
    vk::PipelineShaderStageCreateInfo fragShaderStageCreateInfo{{}, vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main", nullptr};

    std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages{vertShaderStageCreateInfo, fragShaderStageCreateInfo};

    auto bindingDescription{Vertex::getBindingDescription()};
    auto attributeDescriptions{Vertex::getAttributeDescriptions()};
    vk::PipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{{}, bindingDescription, attributeDescriptions};

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{{}, vk::PrimitiveTopology::eTriangleList, false};

    std::array<vk::DynamicState, 2> dynamicStates{
        vk::DynamicState::eViewportWithCount,
        vk::DynamicState::eScissorWithCount
    };
    vk::PipelineDynamicStateCreateInfo dynamicState{{}, dynamicStates};

    vk::PipelineViewportStateCreateInfo viewportStateCreateInfo{{}, nullptr, nullptr}; // TODO: Counts were originally 1, so this might lead to problems

    vk::PipelineRasterizationStateCreateInfo rasterizer{
        {}, false, false, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, false,
        0.f, 0.f, 0.f, 1.f
    };

    vk::PipelineMultisampleStateCreateInfo multisampleState{{}, vk::SampleCountFlagBits::e1, false, 1.f, nullptr, false, false};

    vk::PipelineColorBlendAttachmentState colorBlendAttachment{
        false, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd, vk::BlendFactor::eOne, vk::BlendFactor::eZero, vk::BlendOp::eAdd,
        vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
    };

    vk::PipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{{}, false, vk::LogicOp::eCopy, colorBlendAttachment, {0.f}};

    vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{{}, *descriptorSetLayout, nullptr};

    pipelineLayout = vk::raii::PipelineLayout{device, pipelineLayoutCreateInfo};

    vk::PipelineDepthStencilStateCreateInfo depthStencilInfo{{}, true, true, vk::CompareOp::eLessOrEqual, false, false, {}, {}, 0.f, 1.f};

    vk::GraphicsPipelineCreateInfo pipelineCreateInfo{
        {}, shaderStages, &vertexInputStateCreateInfo, &inputAssemblyStateCreateInfo, nullptr, &viewportStateCreateInfo, &rasterizer, &multisampleState,
        &depthStencilInfo, &colorBlendStateCreateInfo, &dynamicState, pipelineLayout, renderPass, 0, {}, -1
    };

    graphicsPipeline = vk::raii::Pipeline{device, nullptr, pipelineCreateInfo};
}

void HelloTriangleApplication::createFramebuffers()
{
    swapChainFramebuffers.clear();
    swapChainFramebuffers.reserve(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); ++i)
    {
        std::array<vk::ImageView, 2> attachments = {swapChainImageViews[i], depthImageView};
        vk::FramebufferCreateInfo framebufferCreateInfo{{}, renderPass, attachments, swapChainExtent.width, swapChainExtent.height, 1};

        swapChainFramebuffers.emplace_back(device, framebufferCreateInfo);
    }
}

void HelloTriangleApplication::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices{findQueueFamilies(physicalDevice, surface)};

    vk::CommandPoolCreateInfo commandPoolCreateInfo{vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueFamilyIndices.graphicsFamily.value()};

    commandPool = vk::raii::CommandPool{device, commandPoolCreateInfo};
}

void HelloTriangleApplication::createCommandBuffers()
{
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo{commandPool, vk::CommandBufferLevel::ePrimary, maxFramesInFlight};

    commandBuffers = device.allocateCommandBuffers(commandBufferAllocateInfo);
}

void HelloTriangleApplication::recordCommandBuffer(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex)
{
    vk::CommandBufferBeginInfo beginInfo{{}, nullptr};
    commandBuffer.begin(beginInfo);

    std::array<vk::ClearValue, 2> clearValues{
        vk::ClearValue{vk::ClearColorValue{std::array{0.0f, 0.0f, 0.0f, 1.0f}}}, vk::ClearValue{vk::ClearDepthStencilValue{1.0f, 0}}
    };

    vk::RenderPassBeginInfo renderPassInfo{renderPass, swapChainFramebuffers[imageIndex], vk::Rect2D{{0, 0}, swapChainExtent}, clearValues};

    commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

    vk::Viewport viewport{0, 0, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.f, 1.f};
    commandBuffer.setViewportWithCount(viewport);

    vk::Rect2D scissor{{0, 0}, swapChainExtent};
    commandBuffer.setScissorWithCount(scissor);

    commandBuffer.bindVertexBuffers(0, *vertexBuffer, {0});
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, *descriptorSets[currentFrame], nullptr);
    commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);

    commandBuffer.drawIndexed(meshIndices.size(), 1, 0, 0, 0);

    commandBuffer.endRenderPass();

    commandBuffer.end();
}

vk::raii::CommandBuffer HelloTriangleApplication::beginSingleTimeCommands() // TODO: This function screams problem
{
    vk::CommandBufferAllocateInfo commandBufferAllocateInfo{commandPool, vk::CommandBufferLevel::ePrimary, 1};
    vk::raii::CommandBuffer commandBuffer{std::move(device.allocateCommandBuffers(commandBufferAllocateInfo).front())};

    vk::CommandBufferBeginInfo beginInfo{vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
    commandBuffer.begin(beginInfo);

    return commandBuffer;
}

void HelloTriangleApplication::endSindleTimeCommands(vk::raii::CommandBuffer&& commandBuffer) //TODO: Investigate warning
{
    commandBuffer.end();

    vk::SubmitInfo submitInfo{nullptr, nullptr, *commandBuffer, nullptr};

    graphicsQueue.submit(submitInfo, nullptr);

    graphicsQueue.waitIdle();
}

void HelloTriangleApplication::drawFrame()
{
    if (frameBufferResized)
    {
        recreateSwapchain();
        frameBufferResized = false;
    }

    updateUniformBuffer(currentFrame);

    vk::raii::CommandBuffer& commandBuffer{commandBuffers[currentFrame]};
    const vk::raii::Semaphore& imageAvailableSemaphore{imageAvailableSemaphores[currentFrame]};
    const vk::raii::Semaphore& renderFinishedSemaphore{renderFinishedSemaphores[currentFrame]};
    const vk::raii::Fence& inFlightFence{inFlightFences[currentFrame]};

    currentFrame = (currentFrame + 1) % maxFramesInFlight;

    check(device.waitForFences(*inFlightFence, true, UINT64_MAX), "Fence wait failed");

    auto [result, imageIndex]{swapChain.acquireNextImage(UINT64_MAX, imageAvailableSemaphore, nullptr)};
    if (checkForBadSwapchain(result) == vk::Result::eErrorOutOfDateKHR)
    {
        return;
    }

    commandBuffer.reset({});
    recordCommandBuffer(commandBuffer, imageIndex);

    vk::PipelineStageFlags waitStages{vk::PipelineStageFlagBits::eColorAttachmentOutput};
    vk::SubmitInfo submitInfo{*imageAvailableSemaphore, waitStages, *commandBuffer, *renderFinishedSemaphore};

    device.resetFences(*inFlightFence);
    graphicsQueue.submit(submitInfo, inFlightFence);

    vk::PresentInfoKHR presentInfo{*renderFinishedSemaphore, *swapChain, imageIndex, nullptr};
    checkForBadSwapchain(presentQueue.presentKHR(presentInfo));
}

void HelloTriangleApplication::updateUniformBuffer(uint32_t frameIndex)
{
    static auto startTime{std::chrono::high_resolution_clock::now()};

    auto currentTime{std::chrono::high_resolution_clock::now()};
    float time{std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count()};

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4{1.0f}, time * glm::radians(90.f), glm::vec3{0.0f, 0.0f, 1.0f});
    ubo.view = glm::lookAt(glm::vec3{2.0f}, glm::vec3{0.f}, glm::vec3{0.f, 0.f, 1.f});
    ubo.projection = glm::perspective(glm::radians(45.f),
                                      static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.
                                          height), 0.1f, 100.f);
    ubo.projection[1][1] *= -1;
    std::memcpy(uniformBuffersMapped[frameIndex], &ubo, sizeof(ubo));
}

void HelloTriangleApplication::createSyncObjects()
{
    imageAvailableSemaphores.clear();
    renderFinishedSemaphores.clear();
    inFlightFences.clear();

    imageAvailableSemaphores.reserve(maxFramesInFlight);
    renderFinishedSemaphores.reserve(maxFramesInFlight);
    inFlightFences.reserve(maxFramesInFlight);

    vk::SemaphoreCreateInfo semaphoreCreateInfo{};
    vk::FenceCreateInfo fenceCreateInfo{vk::FenceCreateFlagBits::eSignaled};

    for (size_t i = 0; i < maxFramesInFlight; ++i)
    {
        imageAvailableSemaphores.emplace_back(device, semaphoreCreateInfo);
        renderFinishedSemaphores.emplace_back(device, semaphoreCreateInfo);
        inFlightFences.emplace_back(device, fenceCreateInfo);
    }
}

void HelloTriangleApplication::frameBufferResizedCallback(GLFWwindow* window, int inWidth, int inHeight)
{
    auto* app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    app->frameBufferResized = true;
}

void HelloTriangleApplication::createVertexBuffer()
{
    const vk::DeviceSize bufferSize = sizeof(Vertex) * meshVertices.size();

    auto [stagingBuffer, stagingBufferMemory]{
        createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible)
    };

    void* data{stagingBufferMemory.mapMemory(0, bufferSize, {})};
    std::memcpy(data, meshVertices.data(), bufferSize);
    stagingBufferMemory.unmapMemory();

    {
        auto [vertexBufferNew, vertexBufferMemoryNew]{
            createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal)
        };
        vertexBuffer = std::move(vertexBufferNew); // TODO: This is ugly
        vertexBufferMemory = std::move(vertexBufferMemoryNew);
    }

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
}

void HelloTriangleApplication::createIndexBuffer()
{
    const VkDeviceSize bufferSize = sizeof(uint32_t) * meshIndices.size();

    auto [stagingBuffer, stagingBufferMemory]{
        createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible)
    };

    void* data{stagingBufferMemory.mapMemory(0, bufferSize, {})};
    std::memcpy(data, meshIndices.data(), bufferSize);
    stagingBufferMemory.unmapMemory();

    {
        auto [indexBufferNew, indexBufferMemoryNew]{
            createBuffer(bufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal)
        };
        indexBuffer = std::move(indexBufferNew); // TODO
        indexBufferMemory = std::move(indexBufferMemoryNew);
    }

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);
}

void HelloTriangleApplication::createUniformBuffers()
{
    vk::DeviceSize bufferSize{sizeof(UniformBufferObject)};

    uniformBuffers.clear();
    uniformBufferMemories.clear();
    uniformBuffersMapped.clear();

    uniformBuffers.reserve(maxFramesInFlight);
    uniformBufferMemories.reserve(maxFramesInFlight);
    uniformBuffersMapped.reserve(maxFramesInFlight);

    for (size_t i = 0; i < maxFramesInFlight; ++i) // TODO: Ranges
    {
        auto [bufferNew, MemoryNew]{
            createBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
        };
        uniformBuffersMapped.emplace_back(MemoryNew.mapMemory(0, bufferSize, {}));
        uniformBuffers.emplace_back(std::move(bufferNew)); // TODO
        uniformBufferMemories.emplace_back(std::move(MemoryNew));
    }
}

std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> HelloTriangleApplication::createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage,
                                                                                           vk::MemoryPropertyFlags properties)
{
    vk::BufferCreateInfo bufferCreateInfo{{}, size, usage, vk::SharingMode::eExclusive, nullptr};

    vk::raii::Buffer buffer{device, bufferCreateInfo};
    vk::MemoryRequirements memoryRequirements{buffer.getMemoryRequirements()};

    vk::MemoryAllocateInfo memoryAllocateInfo{memoryRequirements.size, findMemoryType(memoryRequirements.memoryTypeBits, properties)};
    vk::raii::DeviceMemory memory{device.allocateMemory(memoryAllocateInfo)};

    buffer.bindMemory(memory, 0);

    return std::make_pair(std::move(buffer), std::move(memory));
}

void HelloTriangleApplication::copyBuffer(vk::Buffer sourceBuffer, vk::Buffer destinationBuffer, vk::DeviceSize size)
{
    vk::raii::CommandBuffer commandBuffer{beginSingleTimeCommands()};

    vk::BufferCopy copyRegion{0, 0, size};

    commandBuffer.copyBuffer(sourceBuffer, destinationBuffer, copyRegion);

    endSindleTimeCommands(std::move(commandBuffer));
}

uint32_t HelloTriangleApplication::findMemoryType(uint32_t typeBits, vk::MemoryPropertyFlags properties) const
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

void HelloTriangleApplication::createDescriptorSetLayout()
{
    vk::DescriptorSetLayoutBinding uboLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex, nullptr};
    vk::DescriptorSetLayoutBinding samplerLayoutBinding{1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment, nullptr};

    std::array<vk::DescriptorSetLayoutBinding, 2> layoutBindings{uboLayoutBinding, samplerLayoutBinding};

    vk::DescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{{}, layoutBindings};
    descriptorSetLayout = vk::raii::DescriptorSetLayout{device, descriptorSetLayoutCreateInfo};
}

void HelloTriangleApplication::createDescriptorPool()
{
    std::array<vk::DescriptorPoolSize, 2> descriptorPoolSizes{
        vk::DescriptorPoolSize{vk::DescriptorType::eUniformBuffer, maxFramesInFlight},
        vk::DescriptorPoolSize{vk::DescriptorType::eCombinedImageSampler, maxFramesInFlight}
    };

    vk::DescriptorPoolCreateInfo poolCreateInfo{vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, maxFramesInFlight, descriptorPoolSizes};
    descriptorPool = vk::raii::DescriptorPool{device, poolCreateInfo};
}

void HelloTriangleApplication::createDescriptorSets()
{
    std::vector<vk::DescriptorSetLayout> layouts(maxFramesInFlight, descriptorSetLayout); // TODO: This could be raii?
    vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{descriptorPool, layouts};

    descriptorSets = device.allocateDescriptorSets(descriptorSetAllocateInfo);

    for (size_t i = 0; i < maxFramesInFlight; ++i)
    {
        vk::DescriptorBufferInfo descriptorBufferInfo{uniformBuffers[i], 0, sizeof(UniformBufferObject)};
        vk::DescriptorImageInfo descriptorImageInfo{textureSampler, textureImageView, vk::ImageLayout::eShaderReadOnlyOptimal};

        std::array<vk::WriteDescriptorSet, 2> descriptorWrites{
            vk::WriteDescriptorSet{descriptorSets[i], 0, 0, vk::DescriptorType::eUniformBuffer, nullptr, descriptorBufferInfo, nullptr},
            vk::WriteDescriptorSet{descriptorSets[i], 1, 0, vk::DescriptorType::eCombinedImageSampler, descriptorImageInfo, nullptr, nullptr}
        };

        device.updateDescriptorSets(descriptorWrites, nullptr);
    }
}

std::pair<vk::raii::Image, vk::raii::DeviceMemory> HelloTriangleApplication::createImage(uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling,
                                                                                         vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties,
                                                                                         uint32_t mipLevels = 1)
{
    vk::ImageCreateInfo imageCreateInfo{
        {}, vk::ImageType::e2D, format, vk::Extent3D{width, height, 1}, mipLevels, 1, vk::SampleCountFlagBits::e1, tiling, usage, vk::SharingMode::eExclusive, nullptr,
        vk::ImageLayout::eUndefined
    };
    vk::raii::Image image{device, imageCreateInfo};

    vk::MemoryRequirements memoryRequirements{image.getMemoryRequirements()};
    vk::MemoryAllocateInfo memoryAllocateInfo{memoryRequirements.size, findMemoryType(memoryRequirements.memoryTypeBits, properties)};

    vk::raii::DeviceMemory imageMemory{device, memoryAllocateInfo};

    image.bindMemory(imageMemory, 0);

    return std::make_pair(std::move(image), std::move(imageMemory));
}

void HelloTriangleApplication::createTextureImage()
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels{stbi_load(texturePath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha)};

    if (!pixels)
    {
        throw std::runtime_error("Failed to load texture image!");
    }

    vk::DeviceSize imageSize{static_cast<uint64_t>(texWidth) * texHeight * 4};
    textureMipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

    auto [stagingBuffer, stagingBufferMemory]{
        createBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent)
    };

    void* data{stagingBufferMemory.mapMemory(0, imageSize, {})};
    std::memcpy(data, pixels, imageSize);
    stagingBufferMemory.unmapMemory();
    stbi_image_free(pixels);

    {
        auto [imageNew, imageMemoryNew]{
            createImage(texWidth, texHeight, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
                        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferSrc,
                        // TODO: Can't we create the mips in the staging one and safe this eTransferSrc?
                        vk::MemoryPropertyFlagBits::eDeviceLocal, textureMipLevels)
        };
        textureImage = std::move(imageNew);
        textureImageMemory = std::move(imageMemoryNew);
    }

    transitionImageLayout(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, textureMipLevels);
    copyBufferToImage(stagingBuffer, textureImage, texWidth, texHeight);

    // We do not need to transition the image layout. This is handled by generateMipMaps()
    //transitionImageLayout(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, textureMipLevels);
    generateMipMaps(textureImage, vk::Format::eR8G8B8A8Srgb, texWidth, texHeight, textureMipLevels);
}

void HelloTriangleApplication::generateMipMaps(const vk::Image& image, const vk::Format& imageFormat, int32_t width, int32_t height, uint32_t mipLevels)
{
    if (!(physicalDevice.getFormatProperties(imageFormat).optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear))
    {
        throw std::runtime_error("Failed to generate mipmaps for image!\n Image format does not support linear blitting!");
    }
    vk::raii::CommandBuffer commandBuffer{beginSingleTimeCommands()};

    vk::ImageMemoryBarrier barrier{
        {}, {}, vk::ImageLayout::eUndefined, vk::ImageLayout::eUndefined, vk::QueueFamilyIgnored, vk::QueueFamilyIgnored, image,
        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
    };

    int32_t mipWidth{width};
    int32_t mipHeight{height};

    for (uint32_t i = 1; i < mipLevels; ++i)
    {
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

        // Prepare next mip for transfer
        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, nullptr, nullptr, barrier);

        vk::ImageBlit blit{
            vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, i - 1, 0, 1}, std::array{vk::Offset3D{0, 0, 0}, vk::Offset3D{mipWidth, mipHeight, 1}},
            vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, i, 0, 1},
            std::array{vk::Offset3D{0, 0, 0,}, vk::Offset3D{mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1}}
        };

        // Blit image
        commandBuffer.blitImage(image, vk::ImageLayout::eTransferSrcOptimal, image, vk::ImageLayout::eTransferDstOptimal, blit, vk::Filter::eLinear);

        barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
        barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        // Make old mip suitable for shaders
        commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr, nullptr, barrier);

        if (mipWidth > 1)
            mipWidth /= 2;
        if (mipHeight > 1)
            mipHeight /= 2;
    }

    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

    // Make last mip suitable for shaders
    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, nullptr, nullptr, barrier);

    endSindleTimeCommands(std::move(commandBuffer));
}

void HelloTriangleApplication::transitionImageLayout(const vk::Image& image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
                                                     uint32_t mipLevels = 1)
{
    vk::raii::CommandBuffer commandBuffer{beginSingleTimeCommands()};

    vk::PipelineStageFlags sourceStage;
    vk::PipelineStageFlags destinationStage;

    vk::ImageMemoryBarrier barrier{
        {}, {}, oldLayout, newLayout, vk::QueueFamilyIgnored, vk::QueueFamilyIgnored, image,
        vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, mipLevels, 0, 1}
    };

    if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
    {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

        if (hasStencilComponent(format))
        {
            barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    }

    if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eTransfer;
    }
    else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        sourceStage = vk::PipelineStageFlagBits::eTransfer;
        destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
    {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;

        sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
        destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    }
    else
    {
        throw std::runtime_error("Failed to transition image layout: Unsupported layout transition");
    }

    commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, nullptr, nullptr, barrier);

    endSindleTimeCommands(std::move(commandBuffer));
}

void HelloTriangleApplication::copyBufferToImage(vk::Buffer buffer, vk::Image image, uint32_t width, uint32_t height)
{
    vk::raii::CommandBuffer commandBuffer{beginSingleTimeCommands()};

    vk::BufferImageCopy copyRegion{0, 0, 0, vk::ImageSubresourceLayers{vk::ImageAspectFlagBits::eColor, 0, 0, 1}, vk::Offset3D{0, 0, 0}, vk::Extent3D{width, height, 1}};

    commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, copyRegion);

    endSindleTimeCommands(std::move(commandBuffer));
}

void HelloTriangleApplication::createTextureImageView()
{
    textureImageView = createImageView(textureImage, vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlagBits::eColor, textureMipLevels);
}

void HelloTriangleApplication::createTextureSampler()
{
    vk::PhysicalDeviceProperties deviceProperties{physicalDevice.getProperties()};

    vk::SamplerCreateInfo samplerInfo{
        {}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat,
        vk::SamplerAddressMode::eRepeat, 0.f, true, deviceProperties.limits.maxSamplerAnisotropy, false, vk::CompareOp::eAlways, 0.f, vk::LodClampNone,
        vk::BorderColor::eIntOpaqueBlack, false
    };

    textureSampler = vk::raii::Sampler{device, samplerInfo};
}

vk::raii::ImageView HelloTriangleApplication::createImageView(const vk::Image& image, const vk::Format format, const vk::ImageAspectFlags aspectFlags,
                                                              uint32_t mipLevels)
{
    vk::ImageViewCreateInfo imageViewCreateInfo{
        {}, image, vk::ImageViewType::e2D, format, vk::ComponentMapping{}, vk::ImageSubresourceRange{aspectFlags, 0, mipLevels, 0, 1}
    };
    return vk::raii::ImageView{device, imageViewCreateInfo};
}

void HelloTriangleApplication::createDepthResources()
{
    vk::Format depthFormat{findDepthFormat()};

    {
        auto [imageNew, imageMemoryNew]{
            createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
                        vk::MemoryPropertyFlagBits::eDeviceLocal)
        };

        depthImage = std::move(imageNew);
        depthImageMemory = std::move(imageMemoryNew);
    }

    depthImageView = createImageView(depthImage, depthFormat, vk::ImageAspectFlagBits::eDepth);
}

vk::Format HelloTriangleApplication::findDepthFormat()
{
    return findSupportedFormat({vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD32Sfloat}, vk::ImageTiling::eOptimal,
                               vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

bool HelloTriangleApplication::hasStencilComponent(vk::Format format)
{
    return format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint;
}

vk::Format HelloTriangleApplication::findSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const
{
    for (const auto& format : candidates)
    {
        vk::FormatProperties formatProperties{physicalDevice.getFormatProperties(format)};

        if (tiling == vk::ImageTiling::eLinear && (formatProperties.linearTilingFeatures & features) == features)
        {
            return format;
        }
        if (tiling == vk::ImageTiling::eOptimal && (formatProperties.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    throw std::runtime_error("Failed to find supported format!");
}

void HelloTriangleApplication::loadModel()
{
    meshVertices.clear();
    meshIndices.clear();

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string error;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &error, modelPath.c_str()))
    {
        throw std::runtime_error(error);
    }

    std::unordered_map<Vertex, uint32_t> uniqueVertices;

    for (const auto& shape : shapes)
    {
        meshVertices.reserve(meshVertices.size() + shape.mesh.indices.size());
        meshIndices.reserve(meshIndices.size() + shape.mesh.indices.size());

        for (const auto& index : shape.mesh.indices)
        {
            Vertex vertex;
            vertex.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };
            vertex.texCoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.f - attrib.texcoords[2 * index.texcoord_index + 1]
            };
            vertex.color = {1.f, 1.f, 1.f};

            if (!uniqueVertices.contains(vertex))
            {
                uniqueVertices[vertex] = meshVertices.size();
                meshVertices.push_back(vertex);
            }

            meshIndices.push_back(uniqueVertices[vertex]);
        }
    }
    meshVertices.shrink_to_fit();
}
