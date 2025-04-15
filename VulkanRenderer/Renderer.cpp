#include "Renderer.hpp"

#include <iostream>
#include <ranges>
#include <set>

#include "DepthImage.hpp"
#include "check.hpp"
#include "PhysicalDeviceHelper.hpp"
#include "ValidationLayers.hpp"
#include "Asset/Material.hpp"
#include "Asset/MaterialInstance.hpp"
#include "Renderer/RenderSync.hpp"
#include "Scene/Camera.hpp"
#include "Scene/Model.hpp"
#include "Scene/Scene.hpp"
#include "ShaderCompilation/ShaderCursor.hpp"

using namespace std::placeholders;

Renderer::Renderer()
	: context(),
	  instance(createInstance(context)),
	  debugMessenger(createDebugMessenger(instance)),
	  window(1600, 1200, std::bind(onFrameBufferResized, this, _1, _2)),
	  surface(window.createWindowSurface(instance)),
	  physicalDevice(pickPhysicalDevice(instance, surface)),
	  queueIndices(findQueueFamilies(physicalDevice, surface)),
	  device(createLogicalDevice(physicalDevice, queueIndices)),
	  graphicsQueue(device.getQueue(queueIndices.graphicsFamily.value(), 0)),
	  presentQueue(device.getQueue(queueIndices.presentFamily.value(), 0)),
	  swapchain(device, physicalDevice, surface, window, queueIndices),
	  depthImage(device, physicalDevice, swapchain.extent),
	  renderPass(createRenderPass(device, physicalDevice, swapchain)),
	  commandPool(createCommandPool(device, queueIndices)),
	  commandBuffers(device.allocateCommandBuffers({commandPool, vk::CommandBufferLevel::ePrimary, maxFramesInFlight})),
	  swapChainFramebuffers(createFramebuffers(device, renderPass, depthImage.imageView, swapchain.imageViews, swapchain.extent)),
	  renderSyncObjects(createSyncObjects(device, maxFramesInFlight)),
	  compiler()
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
	depthImage = DepthImage{device, physicalDevice, swapchain.extent};
	swapChainFramebuffers = createFramebuffers(device, renderPass, depthImage.imageView, swapchain.imageViews, swapchain.extent);
	// TODO: There is a slight performance overhead here by not reusing the vector
}

void Renderer::onFrameBufferResized(int inWidth, int inHeight)
{
	framebufferResized = true;
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

void Renderer::drawScene(const Scene& scene)
{
	if (framebufferResized)
	{
		recreateSwapchain();
		framebufferResized = false;
	}

	//updateUniformBuffer(currentFrame);

	vk::raii::CommandBuffer& commandBuffer{commandBuffers[currentFrame]};
	const RenderSync& renderSync{renderSyncObjects[currentFrame]};

	currentFrame = (currentFrame + 1) % maxFramesInFlight;

	check(device.waitForFences(*renderSync.inFlightFence, true, UINT64_MAX), "Fence wait failed");

	auto [result, imageIndex]{swapchain.swapchain.acquireNextImage(UINT64_MAX, renderSync.imageAvailableSemaphore, nullptr)};
	if (checkForBadSwapchain(result) == vk::Result::eErrorOutOfDateKHR)
	{
		return;
	}

	commandBuffer.reset({});
	recordCommandBufferForSceneDraw(commandBuffer, imageIndex, scene);

	vk::PipelineStageFlags waitStages{vk::PipelineStageFlagBits::eColorAttachmentOutput};
	const vk::SubmitInfo submitInfo{*renderSync.imageAvailableSemaphore, waitStages, *commandBuffer, *renderSync.renderFinishedSemaphore};

	device.resetFences(*renderSync.inFlightFence);
	graphicsQueue.submit(submitInfo, renderSync.inFlightFence);

	const vk::PresentInfoKHR presentInfo{*renderSync.renderFinishedSemaphore, *swapchain.swapchain, imageIndex, nullptr};
	checkForBadSwapchain(presentQueue.presentKHR(presentInfo));
}

vk::raii::Instance Renderer::createInstance(const vk::raii::Context& context)
{
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		throw std::runtime_error("validation layers requested, but not available!");
	}

	vk::ApplicationInfo appInfo{
		"Hello Triangle", VK_MAKE_VERSION(1, 0, 0), "No Engine", VK_MAKE_VERSION(1, 0, 0), vk::ApiVersion14
	};

	const auto requiredExtensions{getRequiredExtensions()};

	const std::vector<const char*>& usedValidationLayers{
		enableValidationLayers ? validationLayers : std::vector<const char*>{}
	};
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

vk::raii::PhysicalDevice
Renderer::pickPhysicalDevice(const vk::raii::Instance& instance, const vk::SurfaceKHR& surface)
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

vk::raii::Device Renderer::createLogicalDevice(const vk::raii::PhysicalDevice& physicalDevice,
                                               const QueueFamilyIndices& queueIndices)
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

	const std::vector<const char*>& usedValidationLayers{
		enableValidationLayers ? validationLayers : std::vector<const char*>{}
	};
	vk::DeviceCreateInfo createInfo{{}, queueCreateInfos, usedValidationLayers, deviceExtensions, &deviceFeatures};

	return vk::raii::Device{physicalDevice, createInfo};
}

vk::raii::CommandPool Renderer::createCommandPool(const vk::raii::Device& device,
                                                  const QueueFamilyIndices& queueIndices)
{
	vk::CommandPoolCreateInfo commandPoolCreateInfo{
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer, queueIndices.graphicsFamily.value()
	};

	return vk::raii::CommandPool{device, commandPoolCreateInfo};
}

vk::raii::RenderPass Renderer::createRenderPass(const vk::raii::Device& device, const vk::PhysicalDevice& physicalDevice, const Swapchain& swapchain)
{
	vk::AttachmentDescription colorAttachment{
		{}, swapchain.imageFormat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear,
		vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare,
		vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR
	};
	vk::AttachmentReference colorAttachmentReference{0, vk::ImageLayout::eColorAttachmentOptimal};

	vk::AttachmentDescription depthAttachment{
		{}, DepthImage::findDepthFormat(physicalDevice), vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear,
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

std::vector<vk::raii::Framebuffer> Renderer::createFramebuffers(const vk::raii::Device& device,
                                                                const vk::raii::RenderPass& renderPass,
                                                                const vk::raii::ImageView& depthImageView,
                                                                const std::vector<vk::raii::ImageView>& imageViews,
                                                                const vk::Extent2D& swapchainExtent)
{
	auto framebuffers{
		imageViews | std::ranges::views::transform([&](const auto& imageView) -> vk::raii::Framebuffer
		{
			std::array<vk::ImageView, 2> attachments = {imageView, depthImageView};
			vk::FramebufferCreateInfo framebufferCreateInfo{
				{}, renderPass, attachments, swapchainExtent.width, swapchainExtent.height, 1
			};
			return vk::raii::Framebuffer{device, framebufferCreateInfo};
		})
	};
	return {framebuffers.begin(), framebuffers.end()};
}

std::vector<RenderSync> Renderer::createSyncObjects(const vk::raii::Device& device, uint8_t maxFramesInFlight)
{
	std::vector<RenderSync> renderSyncObjects{};
	renderSyncObjects.reserve(maxFramesInFlight);

	for (size_t i = 0; i < maxFramesInFlight; ++i)
	{
		renderSyncObjects.emplace_back(device);
	}
	return renderSyncObjects;
}

vk::Bool32 Renderer::debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                   vk::DebugUtilsMessageTypeFlagsEXT messageType,
                                   const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	if (messageSeverity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
	{
		// Message is important enough to show
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	}

	return false;
}

void Renderer::recordCommandBufferForSceneDraw(const vk::raii::CommandBuffer& commandBuffer, unsigned imageIndex, const Scene& scene)
{
	vk::CommandBufferBeginInfo beginInfo{{}, nullptr};
	commandBuffer.begin(beginInfo);

	std::array<vk::ClearValue, 2> clearValues{
		vk::ClearValue{vk::ClearColorValue{std::array{0.0f, 0.0f, 0.0f, 1.0f}}}, vk::ClearValue{vk::ClearDepthStencilValue{1.0f, 0}}
	};

	vk::RenderPassBeginInfo renderPassInfo{renderPass, swapChainFramebuffers[imageIndex], vk::Rect2D{{0, 0}, swapchain.extent}, clearValues};

	commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

	vk::Viewport viewport{0, 0, static_cast<float>(swapchain.extent.width), static_cast<float>(swapchain.extent.height), 0.f, 1.f};
	commandBuffer.setViewportWithCount(viewport);

	vk::Rect2D scissor{{0, 0}, swapchain.extent};
	commandBuffer.setScissorWithCount(scissor);

	for (const auto& model : scene.models)
	{
		// TODO: This is extremely inefficient. We should sort the models by material, material instance, mesh to avoid rebinding. Also, we should share descriptor sets

		auto materialInstance{model.material};
		auto material{materialInstance->parentMaterial};
		const vk::raii::Pipeline& pipeline{material->pipeline};
		vk::Pipeline vkPipeline{*pipeline};
		//commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, model.material->parentMaterial->pipeline);

		ShaderCursor globalCursor{model.material->getShaderCursor()};
		ShaderCursor modelCursor{globalCursor.field("gModelData")};
		modelCursor.field("modelTransform").write(model.transform.getMatrix());
		modelCursor.field("inverseTransposeModelTransform").write(inverse(transpose(model.transform.getMatrix())));

		ShaderCursor viewCursor{globalCursor.field("gViewData")};
		viewCursor.field("viewPosition").write(scene.camera.transform.translation);
		viewCursor.field("viewProjection").write(scene.camera.getViewProjection(glm::vec2{swapchain.extent.width, swapchain.extent.height}));

		ShaderCursor lightCursor{globalCursor.field("gLightEnvironment")};
		lightCursor.field("direction").write(glm::vec3{1.f, 1.f, 1.f});
		lightCursor.field("color").write(glm::vec3{1.f, .8f, .6f});
		lightCursor.field("intensity").write(glm::vec1{5.f});

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, vkPipeline);

		commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, model.material->parentMaterial->pipelineLayout, 0, *model.material->shaderObject.getDescriptorSets()[currentFrame], nullptr);

		commandBuffer.bindVertexBuffers(0, *model.mesh->vertexBuffer.vkBuffer, {0});
		commandBuffer.bindIndexBuffer(model.mesh->indexBuffer.vkBuffer, 0, vk::IndexType::eUint32);

		commandBuffer.drawIndexed(model.mesh->rawMesh.indices.size(), 1, 0, 0, 0);
	}

	commandBuffer.endRenderPass();

	commandBuffer.end();
}

vk::Result Renderer::checkForBadSwapchain(vk::Result inResult)
{
	if (inResult == vk::Result::eErrorOutOfDateKHR)
	{
		recreateSwapchain();
		framebufferResized = false;
		return inResult;
	}
	if (inResult == vk::Result::eSuboptimalKHR)
	{
		return inResult;
	}
	return check(inResult);
}
