#include "HelloTriangleApplication.hpp"

#include <chrono>
#include <vector>
#include <unordered_map>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/gtc/matrix_transform.hpp>

#include "check.hpp"
#include "IOHelper.hpp"
#include "Shader.hpp"
#include "Swapchain.hpp"
#include "Uniforms.hpp"
#include "Vertex.hpp"

void HelloTriangleApplication::run()
{
	initVulkan();
	mainLoop();
}

void HelloTriangleApplication::initVulkan()
{
	createDescriptorSetLayout();
	createGraphicsPipeline();

	loadModel();

	createTextureImage();
	createTextureSampler();

	createVertexBuffer();
	createIndexBuffer();

	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();

	createSyncObjects();
}

void HelloTriangleApplication::mainLoop()
{
	while (!window.shouldClose())
	{
		Window::pollEvents();
		drawFrame();
	}

	device.waitIdle();
}

vk::Result HelloTriangleApplication::checkForBadSwapchain(vk::Result inResult)
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

void HelloTriangleApplication::createGraphicsPipeline()
{
	auto vertShaderCode{readFile("Shaders/default.spv")};
	auto fragShaderCode{readFile("Shaders/default.spv")};

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

void HelloTriangleApplication::recordCommandBuffer(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex)
{
	vk::CommandBufferBeginInfo beginInfo{{}, nullptr};
	commandBuffer.begin(beginInfo);

	std::array<vk::ClearValue, 2> clearValues{
		vk::ClearValue{vk::ClearColorValue{std::array{0.0f, 0.0f, 0.0f, 1.0f}}}, vk::ClearValue{vk::ClearDepthStencilValue{1.0f, 0}}
	};

	vk::RenderPassBeginInfo renderPassInfo{renderPass, swapChainFramebuffers[imageIndex], vk::Rect2D{{0, 0}, swapchain.extent}, clearValues};

	commandBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, graphicsPipeline);

	vk::Viewport viewport{0, 0, static_cast<float>(swapchain.extent.width), static_cast<float>(swapchain.extent.height), 0.f, 1.f};
	commandBuffer.setViewportWithCount(viewport);

	vk::Rect2D scissor{{0, 0}, swapchain.extent};
	commandBuffer.setScissorWithCount(scissor);

	commandBuffer.bindVertexBuffers(0, *vertexBuffer, {0});
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, *descriptorSets[currentFrame], nullptr);
	commandBuffer.bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);

	commandBuffer.drawIndexed(meshIndices.size(), 1, 0, 0, 0);

	commandBuffer.endRenderPass();

	commandBuffer.end();
}

void HelloTriangleApplication::drawFrame()
{
	if (framebufferResized)
	{
		recreateSwapchain();
		framebufferResized = false;
	}

	updateUniformBuffer(currentFrame);

	vk::raii::CommandBuffer& commandBuffer{commandBuffers[currentFrame]};
	const vk::raii::Semaphore& imageAvailableSemaphore{imageAvailableSemaphores[currentFrame]};
	const vk::raii::Semaphore& renderFinishedSemaphore{renderFinishedSemaphores[currentFrame]};
	const vk::raii::Fence& inFlightFence{inFlightFences[currentFrame]};

	currentFrame = (currentFrame + 1) % maxFramesInFlight;

	check(device.waitForFences(*inFlightFence, true, UINT64_MAX), "Fence wait failed");

	auto [result, imageIndex]{swapchain.swapchain.acquireNextImage(UINT64_MAX, imageAvailableSemaphore, nullptr)};
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

	vk::PresentInfoKHR presentInfo{*renderFinishedSemaphore, *swapchain.swapchain, imageIndex, nullptr};
	checkForBadSwapchain(presentQueue.presentKHR(presentInfo));
}

void HelloTriangleApplication::updateUniformBuffer(uint32_t frameIndex)
{
	static auto startTime{std::chrono::high_resolution_clock::now()};

	auto currentTime{std::chrono::high_resolution_clock::now()};
	float time{std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count()};

	UniformBufferObject ubo{};
	ubo.model = glm::rotate(glm::scale(glm::mat4{1.0f}, glm::vec3{5.f}), time * glm::radians(90.f), glm::vec3{0.0f, 0.0f, 1.0f});
	ubo.view = glm::lookAt(glm::vec3{2.0f}, glm::vec3{0.f}, glm::vec3{0.f, 0.f, 1.f});
	ubo.projection = glm::perspective(glm::radians(45.f),
	                                  static_cast<float>(swapchain.extent.width) / static_cast<float>(swapchain.extent.
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

void HelloTriangleApplication::copyBuffer(vk::Buffer sourceBuffer, vk::Buffer destinationBuffer, vk::DeviceSize size)
{
	vk::raii::CommandBuffer commandBuffer{beginSingleTimeCommands()};

	vk::BufferCopy copyRegion{0, 0, size};

	commandBuffer.copyBuffer(sourceBuffer, destinationBuffer, copyRegion);

	endSingleTimeCommands(std::move(commandBuffer));
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
		vk::DescriptorImageInfo descriptorImageInfo{textureSampler, texture.value().imageView, vk::ImageLayout::eShaderReadOnlyOptimal};

		std::array<vk::WriteDescriptorSet, 2> descriptorWrites{
			vk::WriteDescriptorSet{descriptorSets[i], 0, 0, vk::DescriptorType::eUniformBuffer, nullptr, descriptorBufferInfo, nullptr},
			vk::WriteDescriptorSet{descriptorSets[i], 1, 0, vk::DescriptorType::eCombinedImageSampler, descriptorImageInfo, nullptr, nullptr}
		};

		device.updateDescriptorSets(descriptorWrites, nullptr);
	}
}

void HelloTriangleApplication::createTextureImage()
{
	texture = TextureImage{texturePath, *this};
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
