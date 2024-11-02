#include "HelloTriangleApplication.h"

#include <chrono>
#include <iostream>
#include <set>
#include <vector>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/gtc/matrix_transform.hpp>

#include "check.h"
#include "CommandQueues.h"
#include "DeviceExtensions.h"
#include "IOHelper.h"
#include "Shader.h"
#include "SwapChain.h"
#include "Uniforms.h"
#include "ValidationLayers.h"
#include "Vertex.h"
#include "Image.h"

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

    vkDeviceWaitIdle(device);
}

void HelloTriangleApplication::cleanup()
{
    for (size_t i = 0; i < maxFramesInFlight; ++i)
    {
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);

    for (size_t i = 0; i < maxFramesInFlight; ++i)
    {
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBufferMemories[i], nullptr);
    }

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    cleanupSwapchain();

    vkDestroySampler(device, textureSampler, nullptr);
    vkDestroyImageView(device, textureImageView, nullptr);
    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, textureImageMemory, nullptr);

    vkDestroyBuffer(device, vertexBuffer, nullptr);
    vkFreeMemory(device, vertexBufferMemory, nullptr);

    vkDestroyBuffer(device, indexBuffer, nullptr);
    vkFreeMemory(device, indexBufferMemory, nullptr);

    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    vkDestroyDevice(device, nullptr);

    if constexpr (enableValidationLayers)
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);

    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);

    glfwDestroyWindow(window);
    glfwTerminate();
}

void HelloTriangleApplication::createInstance()
{
    if (enableValidationLayers && !checkValidationLayerSupport())
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "Hello Triangle",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No Engine",
        .engineVersion = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3
    };

    const auto requiredExtensions{getRequiredExtensions()};
    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = enableValidationLayers ? static_cast<uint32_t>(validationLayers.size()) : 0,
        .ppEnabledLayerNames = enableValidationLayers ? validationLayers.data() : nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
        .ppEnabledExtensionNames = requiredExtensions.data(),
    };

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{makeDebugMessengerCreateInfo(debugCallback)};
    if constexpr (enableValidationLayers)
    {
        createInfo.pNext = &debugCreateInfo;
    }

    check(vkCreateInstance(&createInfo, nullptr, &instance), "Failed to create instance");

    {
        uint32_t extensionsCount;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionsCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionsCount, extensions.data());

        std::cout << "available extensions:\n";

        for (const auto& extension : extensions)
        {
            std::cout << '\t' << extension.extensionName << '\n';
        }
        std::cout << std::flush;
    }
}

VkBool32 HelloTriangleApplication::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                 VkDebugUtilsMessageTypeFlagsEXT messageType,
                                                 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                 void* pUserData)
{
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        // Message is important enough to show
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
    }

    return VK_FALSE;
}

void HelloTriangleApplication::setupDebugMessenger()
{
    if constexpr (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{makeDebugMessengerCreateInfo(debugCallback)};

    check(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger),
          "Failed to create debug messenger");
}

void HelloTriangleApplication::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("Failed to find GPUs with Vulkan support");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& physDevice : devices)
    {
        if (isDeviceSuitable(physDevice))
        {
            physicalDevice = physDevice;
            break; // Pick first suitable device
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Failed to find a suitable GPU!");
    }
}

bool HelloTriangleApplication::isDeviceSuitable(VkPhysicalDevice physDevice) const
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physDevice, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(physDevice, &deviceFeatures);

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

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    float queuePriority = 1.0f;

    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .queueFamilyIndex = queueFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };
        queueCreateInfos.emplace_back(queueCreateInfo);
    }


    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo createInfo{
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos = queueCreateInfos.data(),
        .enabledLayerCount = enableValidationLayers ? static_cast<uint32_t>(validationLayers.size()) : 0,
        .ppEnabledLayerNames = enableValidationLayers ? validationLayers.data() : nullptr,
        .enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = &deviceFeatures,
    };

    check(vkCreateDevice(physicalDevice, &createInfo, nullptr, &device), "Failed to create logical device");

    vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

void HelloTriangleApplication::createSurface()
{
    check(glfwCreateWindowSurface(instance, window, nullptr, &surface), "Failed to create window surface");
}

void HelloTriangleApplication::createSwapchain()
{
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
    {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapChainCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = swapChainSupport.capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE
    };

    QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
    if (indices.graphicsFamily != indices.presentFamily)
    {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapChainCreateInfo.queueFamilyIndexCount = 2;
        const uint32_t queueFamilyList[]{indices.graphicsFamily.value(), indices.presentFamily.value()};
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyList;
    }

    check(vkCreateSwapchainKHR(device, &swapChainCreateInfo, nullptr, &swapChain), "Failed to create swapchain");

    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
}

void HelloTriangleApplication::recreateSwapchain()
{
    int winWidth, winHeight;
    glfwGetFramebufferSize(window, &winWidth, &winHeight);
    while (winWidth == 0 || winHeight == 0)
    {
        glfwGetFramebufferSize(window, &winWidth, &winHeight);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);

    cleanupSwapchain();

    createSwapchain();
    createImageViews();
    createDepthResources();
    createFramebuffers();
}

void HelloTriangleApplication::cleanupSwapchain()
{
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);
    
    for (auto& framebuffer : swapChainFramebuffers)
    {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    for (auto& imageView : swapChainImageViews)
    {
        vkDestroyImageView(device, imageView, nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

VkResult HelloTriangleApplication::checkForBadSwapchain(VkResult inResult)
{
    if (inResult == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapchain();
        frameBufferResized = false;
        return inResult;
    }
    if (inResult == VK_SUBOPTIMAL_KHR)
    {
        return inResult;
    }
    return check(inResult);
}

void HelloTriangleApplication::createImageViews()
{
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++)
    {
        swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

void HelloTriangleApplication::createRenderPass()
{
    VkAttachmentDescription colorAttachment{
        .flags = 0,
        .format = swapChainImageFormat,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
    };

    VkAttachmentReference colorAttachmentReference{
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
    };

    VkAttachmentDescription depthAttachment{
        .flags = 0,
        .format = findDepthFormat(),
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkAttachmentReference depthAttachmentReference{
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
    };

    VkSubpassDescription subpass{
        .flags = 0,
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .inputAttachmentCount = 0,
        .pInputAttachments = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachmentReference,
        .pResolveAttachments = nullptr,
        .pDepthStencilAttachment = &depthAttachmentReference,
        .preserveAttachmentCount = 0,
        .pPreserveAttachments = nullptr
    };

    VkSubpassDependency dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
        .srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = 0
    };

    std::array<VkAttachmentDescription, 2> attachments{colorAttachment, depthAttachment};

    VkRenderPassCreateInfo renderPassCreateInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .attachmentCount = attachments.size(),
        .pAttachments = attachments.data(),
        .subpassCount = 1,
        .pSubpasses = &subpass,
        .dependencyCount = 1,
        .pDependencies = &dependency
    };

    check(vkCreateRenderPass(device, &renderPassCreateInfo, nullptr, &renderPass), "Failed to create Render Pass!");
}

void HelloTriangleApplication::createGraphicsPipeline()
{
    auto vertShaderCode{readFile("Shaders/vert.spv")};
    auto fragShaderCode{readFile("Shaders/frag.spv")};

    VkShaderModule vertShaderModule = createShaderModule(vertShaderCode, device);
    VkShaderModule fragShaderModule = createShaderModule(fragShaderCode, device);

    VkPipelineShaderStageCreateInfo vertShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vertShaderModule,
        .pName = "main",
        .pSpecializationInfo = nullptr
    };

    VkPipelineShaderStageCreateInfo fragShaderStageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = fragShaderModule,
        .pName = "main",
        .pSpecializationInfo = nullptr
    };

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageCreateInfo, fragShaderStageCreateInfo};

    auto bindingDescription{Vertex::getBindingDescription()};
    auto attributeDescriptions{Vertex::getAttributeDescriptions()};
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bindingDescription,
        .vertexAttributeDescriptionCount = attributeDescriptions.size(),
        .pVertexAttributeDescriptions = attributeDescriptions.data()
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE
    };

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
        .pDynamicStates = dynamicStates.data()
    };

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr,
    };

    VkPipelineRasterizationStateCreateInfo rasterizer{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f
    };

    VkPipelineMultisampleStateCreateInfo multisampleState{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .minSampleShading = 1.0f,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachment{
        .blendEnable = VK_FALSE,
        .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
        .alphaBlendOp = VK_BLEND_OP_ADD,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
        VK_COLOR_COMPONENT_A_BIT
    };

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachment,
        .blendConstants = {0.f, 0.f, 0.f, 0.f}
    };
    
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .setLayoutCount = 1,
        .pSetLayouts = &descriptorSetLayout,
        .pushConstantRangeCount = 0,
        .pPushConstantRanges = nullptr
    };

    check(vkCreatePipelineLayout(device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout),
          "Failed to creat pipeline layout!");

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
        .front = {},
        .back = {},
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f
    };
    
    VkGraphicsPipelineCreateInfo pipelineCreateInfo{
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .stageCount = 2,
        .pStages = shaderStages,
        .pVertexInputState = &vertexInputStateCreateInfo,
        .pInputAssemblyState = &inputAssemblyStateCreateInfo,
        .pTessellationState = nullptr,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizer,
        .pMultisampleState = &multisampleState,
        .pDepthStencilState = &depthStencilInfo,
        .pColorBlendState = &colorBlendStateCreateInfo,
        .pDynamicState = &dynamicState,
        .layout = pipelineLayout,
        .renderPass = renderPass,
        .subpass = 0,
        .basePipelineHandle = VK_NULL_HANDLE,
        .basePipelineIndex = -1
    };

    check(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &graphicsPipeline),
          "Failed to create Graphics Pipeline!");

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

void HelloTriangleApplication::createFramebuffers()
{
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); ++i)
    {
        std::array<VkImageView, 2> attachments = {swapChainImageViews[i], depthImageView};
        VkFramebufferCreateInfo framebufferCreateInfo{
            .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .renderPass = renderPass,
            .attachmentCount = attachments.size(),
            .pAttachments = attachments.data(),
            .width = swapChainExtent.width,
            .height = swapChainExtent.height,
            .layers = 1
        };

        check(vkCreateFramebuffer(device, &framebufferCreateInfo, nullptr, &swapChainFramebuffers[i]),
              "Failed to create Framebuffer!");
    }
}

void HelloTriangleApplication::createCommandPool()
{
    QueueFamilyIndices queueFamilyIndices{findQueueFamilies(physicalDevice, surface)};

    VkCommandPoolCreateInfo commandPoolCreateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamilyIndices.graphicsFamily.value()
    };

    check(vkCreateCommandPool(device, &commandPoolCreateInfo, nullptr, &commandPool), "Failed to create Command Pool!");
}

void HelloTriangleApplication::createCommandBuffers()
{
    commandBuffers.resize(maxFramesInFlight);

    VkCommandBufferAllocateInfo commandBufferAllocateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = commandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = maxFramesInFlight,
    };

    check(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, commandBuffers.data()),
          "Failed to allocate Command Buffer!");
}

void HelloTriangleApplication::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pInheritanceInfo = nullptr
    };

    check(vkBeginCommandBuffer(commandBuffer, &beginInfo), "Failed to begin recording command buffer!");

    std::array<VkClearValue, 2> clearValues{VkClearValue{{{0.0f, 0.0f, 0.0f, 1.0f}}}, VkClearValue{}};
    clearValues[1].depthStencil = {1.f, 0};
    
    VkRenderPassBeginInfo renderPassInfo{
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
        .pNext = nullptr,
        .renderPass = renderPass,
        .framebuffer = swapChainFramebuffers[imageIndex],
        .renderArea = {.offset = {0, 0}, .extent = swapChainExtent},
        .clearValueCount = clearValues.size(),
        .pClearValues = clearValues.data(),
    };

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    VkViewport viewport{
        .x = 0.0f,
        .y = 0.0f,
        .width = static_cast<float>(swapChainExtent.width),
        .height = static_cast<float>(swapChainExtent.height),
        .minDepth = 0.f,
        .maxDepth = 1.f
    };

    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{
        .offset = {0, 0},
        .extent = swapChainExtent
    };

    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentFrame], 0, nullptr);

    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(vertindices.size()), 1, 0, 0, 0);

    vkCmdEndRenderPass(commandBuffer);

    check(vkEndCommandBuffer(commandBuffer), "Failed to record command buffer!");
}

VkCommandBuffer HelloTriangleApplication::beginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo commandBufferAllocateInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .pNext = nullptr,
        .commandPool = commandPool, // TODO: make proprietary pool for short lived command buffers
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1
    };

    VkCommandBuffer commandBuffer;
    check(vkAllocateCommandBuffers(device, &commandBufferAllocateInfo, &commandBuffer),
          "Failed to allocate command buffer");

    VkCommandBufferBeginInfo beginInfo{
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .pNext = nullptr,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
        .pInheritanceInfo = nullptr
    };

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void HelloTriangleApplication::endSindleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 0,
        .pWaitSemaphores = nullptr,
        .pWaitDstStageMask = nullptr,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 0,
        .pSignalSemaphores = nullptr
    };

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);

    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void HelloTriangleApplication::drawFrame()
{
    if (frameBufferResized)
    {
        recreateSwapchain();
        frameBufferResized = false;
    }

    updateUniformBuffer(currentFrame);

    VkCommandBuffer& commandBuffer{commandBuffers[currentFrame]};
    const VkSemaphore& imageAvailableSemaphore{imageAvailableSemaphores[currentFrame]};
    const VkSemaphore& renderFinishedSemaphore{renderFinishedSemaphores[currentFrame]};
    const VkFence& inFlightFence{inFlightFences[currentFrame]};

    currentFrame = (currentFrame + 1) % maxFramesInFlight;

    vkWaitForFences(device, 1, &inFlightFence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result{
        vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex)
    };
    if (checkForBadSwapchain(result) == VK_ERROR_OUT_OF_DATE_KHR)
    {
        return;
    }

    vkResetCommandBuffer(commandBuffer, 0);
    recordCommandBuffer(commandBuffer, imageIndex);

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSemaphore signalSemaphores[] = {renderFinishedSemaphore};
    VkSubmitInfo submitInfo{
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = waitSemaphores,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer,
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = signalSemaphores
    };

    vkResetFences(device, 1, &inFlightFence);
    check(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence), "Failed to submit graphics queue!");

    VkSwapchainKHR swapChains[] = {swapChain};
    VkPresentInfoKHR presentInfo{
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = signalSemaphores,
        .swapchainCount = 1,
        .pSwapchains = swapChains,
        .pImageIndices = &imageIndex,
        .pResults = nullptr
    };

    checkForBadSwapchain(vkQueuePresentKHR(presentQueue, &presentInfo));
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
    imageAvailableSemaphores.resize(maxFramesInFlight);
    renderFinishedSemaphores.resize(maxFramesInFlight);
    inFlightFences.resize(maxFramesInFlight);

    VkSemaphoreCreateInfo semaphoreCreateInfo{
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0
    };

    VkFenceCreateInfo fenceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT
    };

    for (size_t i = 0; i < maxFramesInFlight; ++i)
    {
        check(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]),
              "Failed to create semaphore!");
        check(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]),
              "Failed to create semaphore!");
        check(vkCreateFence(device, &fenceCreateInfo, nullptr, &inFlightFences[i]), "Failed to create fence!");
    }
}

void HelloTriangleApplication::frameBufferResizedCallback(GLFWwindow* window, int inWidth, int inHeight)
{
    auto* app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
    app->frameBufferResized = true;
}

void HelloTriangleApplication::createVertexBuffer()
{
    const VkDeviceSize bufferSize = sizeof(Vertex) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBuffer,
                 stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    std::memcpy(data, vertices.data(), bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void HelloTriangleApplication::createIndexBuffer()
{
    const VkDeviceSize bufferSize = sizeof(uint32_t) * vertindices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, stagingBuffer,
                 stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    std::memcpy(data, vertindices.data(), bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void HelloTriangleApplication::createUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    uniformBuffers.resize(maxFramesInFlight);
    uniformBufferMemories.resize(maxFramesInFlight);
    uniformBuffersMapped.resize(maxFramesInFlight);

    for (size_t i = 0; i < maxFramesInFlight; ++i)
    {
        createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i],
                     uniformBufferMemories[i]);

        vkMapMemory(device, uniformBufferMemories[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void HelloTriangleApplication::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage,
                                            VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& memory)
{
    VkBufferCreateInfo bufferCreateInfo{
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr
    };

    check(vkCreateBuffer(device, &bufferCreateInfo, nullptr, &buffer));

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties)
    };

    check(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &memory),
          "Failed to allocate memory for vertex buffer!");

    vkBindBufferMemory(device, buffer, memory, 0);
}

void HelloTriangleApplication::copyBuffer(VkBuffer sourceBuffer, VkBuffer destinationBuffer, VkDeviceSize size)
{
    VkCommandBuffer commandBuffer{beginSingleTimeCommands()};

    VkBufferCopy copyRegion{
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size,
    };
    vkCmdCopyBuffer(commandBuffer, sourceBuffer, destinationBuffer, 1, &copyRegion);

    endSindleTimeCommands(commandBuffer);
}

uint32_t HelloTriangleApplication::findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties) const
{
    VkPhysicalDeviceMemoryProperties physicalMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalMemoryProperties);

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
    VkDescriptorSetLayoutBinding uboLayoutBinding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr
    };

    VkDescriptorSetLayoutBinding samplerLayoutBinding{
        .binding = 1,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr
    };

    std::array<VkDescriptorSetLayoutBinding, 2> layoutBindings{uboLayoutBinding, samplerLayoutBinding};

    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .bindingCount = layoutBindings.size(),
        .pBindings = layoutBindings.data(),
    };

    check(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout),
          "Failed to create descriptor set layout!");
}

void HelloTriangleApplication::createDescriptorPool()
{
    std::array<VkDescriptorPoolSize, 2> descriptorPoolSizes{
        VkDescriptorPoolSize{
            .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = maxFramesInFlight,
        },
        VkDescriptorPoolSize{
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = maxFramesInFlight,
        }
    };

    VkDescriptorPoolCreateInfo poolCreateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .maxSets = maxFramesInFlight,
        .poolSizeCount = descriptorPoolSizes.size(),
        .pPoolSizes = descriptorPoolSizes.data()
    };

    check(vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &descriptorPool),
          "Failed to create descriptor pool");
}

void HelloTriangleApplication::createDescriptorSets()
{
    std::vector<VkDescriptorSetLayout> layouts(maxFramesInFlight, descriptorSetLayout);
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .pNext = nullptr,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = maxFramesInFlight,
        .pSetLayouts = layouts.data()
    };

    descriptorSets.resize(maxFramesInFlight);
    check(vkAllocateDescriptorSets(device, &descriptorSetAllocateInfo, descriptorSets.data()),
          "Failed to allocate descriptor sets!");

    for (size_t i = 0; i < maxFramesInFlight; ++i)
    {
        VkDescriptorBufferInfo descriptorBufferInfo{
            .buffer = uniformBuffers[i],
            .offset = 0,
            .range = sizeof(UniformBufferObject)
        };

        VkDescriptorImageInfo descriptorImageInfo{
            .sampler = textureSampler,
            .imageView = textureImageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
        };

        std::array<VkWriteDescriptorSet, 2> descriptorWrites{
            VkWriteDescriptorSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = descriptorSets[i],
                .dstBinding = 0,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                .pImageInfo = nullptr,
                .pBufferInfo = &descriptorBufferInfo,
                .pTexelBufferView = nullptr
            },
            VkWriteDescriptorSet{
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                .pNext = nullptr,
                .dstSet = descriptorSets[i],
                .dstBinding = 1,
                .dstArrayElement = 0,
                .descriptorCount = 1,
                .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                .pImageInfo = &descriptorImageInfo,
                .pBufferInfo = nullptr,
                .pTexelBufferView = nullptr
            }
        };

        vkUpdateDescriptorSets(device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }
}

void HelloTriangleApplication::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                                           VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
    VkImageCreateInfo imageCreateInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = format,
        .extent = {.width = width, .height = height, .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = tiling,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    };

    check(vkCreateImage(device, &imageCreateInfo, nullptr, &image), "Failed to create image!");

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(device, image, &memoryRequirements);
    VkMemoryAllocateInfo memoryAllocateInfo{
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .pNext = nullptr,
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties)
    };

    check(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &imageMemory), "Failed to allocate texture memory!");

    vkBindImageMemory(device, image, imageMemory, 0);
}

void HelloTriangleApplication::createTextureImage()
{
    int texWidth, texHeight, texChannels;
    stbi_uc* pixels{stbi_load("Textures/Texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha)};

    if (!pixels)
    {
        throw std::runtime_error("Failed to load texture image!");
    }

    VkDeviceSize imageSize = static_cast<uint64_t>(texWidth) * texHeight * 4;

    VkBuffer stagingBuffer{VK_NULL_HANDLE};
    VkDeviceMemory stagingBufferMemory{VK_NULL_HANDLE};

    createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer,
                 stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
    std::memcpy(data, pixels, imageSize);
    vkUnmapMemory(device, stagingBufferMemory);
    stbi_image_free(pixels);

    createImage(texWidth, texHeight, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, textureImage, textureImageMemory);

    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    copyBufferToImage(stagingBuffer, textureImage, texWidth, texHeight);
    transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void HelloTriangleApplication::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer commandBuffer{beginSingleTimeCommands()};

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    VkImageMemoryBarrier barrier{
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = 0,
        .dstAccessMask = 0,
        .oldLayout = oldLayout,
        .newLayout = newLayout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if (hasStencilComponent(format))
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        throw std::runtime_error("Failed to transition image layout: Unsupported layout transition");
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    endSindleTimeCommands(commandBuffer);
}

void HelloTriangleApplication::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer{beginSingleTimeCommands()};

    VkBufferImageCopy copyRegion{
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .imageOffset = {0, 0, 0},
        .imageExtent = {width, height, 1}
    };

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    endSindleTimeCommands(commandBuffer);
}

void HelloTriangleApplication::createTextureImageView()
{
    textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

void HelloTriangleApplication::createTextureSampler()
{
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

    VkSamplerCreateInfo samplerInfo{
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .magFilter = VK_FILTER_LINEAR,
        .minFilter = VK_FILTER_LINEAR,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .mipLodBias = 0.f,
        .anisotropyEnable = VK_TRUE,
        .maxAnisotropy = deviceProperties.limits.maxSamplerAnisotropy,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .minLod = 0.f,
        .maxLod = 0.f,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE
    };

    check(vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler), "Failed to create texture sampler!");
}

VkImageView HelloTriangleApplication::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
    VkImageView imageView{VK_NULL_HANDLE};

    VkImageViewCreateInfo imageViewCreateInfo{
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .components = {
            .r = VK_COMPONENT_SWIZZLE_IDENTITY, .g = VK_COMPONENT_SWIZZLE_IDENTITY,
            .b = VK_COMPONENT_SWIZZLE_IDENTITY, .a = VK_COMPONENT_SWIZZLE_IDENTITY
        },
        .subresourceRange = {
            .aspectMask = aspectFlags,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        }
    };

    check(vkCreateImageView(device, &imageViewCreateInfo, nullptr, &imageView),
          "Failed to create texture image view!");

    return imageView;
}

void HelloTriangleApplication::createDepthResources()
{
    VkFormat depthFormat = findDepthFormat();

    createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

VkFormat HelloTriangleApplication::findDepthFormat()
{
    return findSupportedFormat({VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D32_SFLOAT}, VK_IMAGE_TILING_OPTIMAL,
                               VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

bool HelloTriangleApplication::hasStencilComponent(VkFormat format)
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

VkFormat HelloTriangleApplication::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
{
    for (const auto& format : candidates)
    {
        VkFormatProperties formatProperties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);

        if (tiling == VK_IMAGE_TILING_LINEAR && (formatProperties.linearTilingFeatures & features) == features)
        {
            return format;
        }
        if (tiling == VK_IMAGE_TILING_OPTIMAL && (formatProperties.optimalTilingFeatures & features) == features)
        {
            return format;
        }
    }

    throw std::runtime_error("Failed to find supported format!");
}
