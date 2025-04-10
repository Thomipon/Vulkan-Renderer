#include "HelloTriangleApplication.hpp"

#include <chrono>
#include <vector>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Buffer.hpp"
#include "check.hpp"
#include "IOHelper.hpp"
#include "Asset/Mesh.hpp"
#include "Shader.hpp"
#include "ShaderCompiler.hpp"
#include "Swapchain.hpp"
#include "Uniforms.hpp"
#include "Vertex.hpp"
#include "Asset/Material.hpp"
#include "Asset/MaterialInstance.hpp"
#include "Renderer/RenderSync.hpp"
#include "Scene/Camera.hpp"
#include "Scene/Model.hpp"
#include "ShaderCompilation/ShaderCursor.hpp"

void HelloTriangleApplication::run()
{
	initVulkan();
	mainLoop();
}

void HelloTriangleApplication::initVulkan()
{
	/*createDescriptorSetLayout();
	createGraphicsPipeline();

	loadModel();

	createTextureImage();

	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();*/

	initScene();
}

void HelloTriangleApplication::mainLoop()
{
	while (!window.shouldClose())
	{
		Window::pollEvents();
		//drawFrame();
		drawScene(scene);
	}

	device.waitIdle();
}

void HelloTriangleApplication::initScene()
{
	scene.camera = std::make_unique<Camera>();
	scene.models.emplace_back();
	Model& model{scene.models[0]};
	model.mesh = std::make_unique<Mesh>(*this, modelPath);

	auto material = std::make_shared<Material>();
	material->compile(compiler, *this);
	model.material = std::make_shared<MaterialInstance>(material);

	/*ShaderCursor materialCursor{model.material->getShaderCursor().field("gMaterial")};
	materialCursor.field("diffuseColor").write(glm::vec3{.5f, .1f, 1.f});
	materialCursor.field("specularColor").write(glm::vec3{.05f, .5f, 1.f});
	materialCursor.field("specularity").write(glm::vec1{1.f});*/
}

void HelloTriangleApplication::updateCamera()
{
	static auto startTime{std::chrono::high_resolution_clock::now()};

	const auto currentTime{std::chrono::high_resolution_clock::now()};
	const float time{std::chrono::duration<float>(currentTime - startTime).count()};

	scene.camera->transform.translation = rotate(glm::vec3{5.f, 0.f, 0.f}, time * glm::radians(90.f), glm::vec3{0.0f, 0.0f, 1.0f});
	scene.camera->transform.rotation = quatLookAt(glm::vec3{2.0f} - glm::vec3{0.f}, glm::vec3{0.f, 0.f, 1.f});
}

void HelloTriangleApplication::createGraphicsPipeline()
{
	SlangCompiler compiler;
	auto vertBlob{compiler.compile("default", "vertexMain")};
	auto fragBlob{compiler.compile("default", "fragmentMain")};

	Material material;
	material.compile(compiler, *this);

	vk::raii::ShaderModule vertShaderModule{createShaderModule(vertBlob, device)};
	vk::raii::ShaderModule fragShaderModule{createShaderModule(fragBlob, device)};

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

	commandBuffer.bindVertexBuffers(0, *mesh->vertexBuffer.vkBuffer, {0});
	commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, *descriptorSets[currentFrame], nullptr);
	commandBuffer.bindIndexBuffer(mesh->indexBuffer.vkBuffer, 0, vk::IndexType::eUint32);

	commandBuffer.drawIndexed(mesh->rawMesh.indices.size(), 1, 0, 0, 0);

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
	const RenderSync& renderSync{renderSyncObjects[currentFrame]};

	currentFrame = (currentFrame + 1) % maxFramesInFlight;

	check(device.waitForFences(*renderSync.inFlightFence, true, UINT64_MAX), "Fence wait failed");

	auto [result, imageIndex]{swapchain.swapchain.acquireNextImage(UINT64_MAX, renderSync.imageAvailableSemaphore, nullptr)};
	if (checkForBadSwapchain(result) == vk::Result::eErrorOutOfDateKHR)
	{
		return;
	}

	commandBuffer.reset({});
	recordCommandBuffer(commandBuffer, imageIndex);

	vk::PipelineStageFlags waitStages{vk::PipelineStageFlagBits::eColorAttachmentOutput};
	const vk::SubmitInfo submitInfo{*renderSync.imageAvailableSemaphore, waitStages, *commandBuffer, *renderSync.renderFinishedSemaphore};

	device.resetFences(*renderSync.inFlightFence);
	graphicsQueue.submit(submitInfo, renderSync.inFlightFence);

	const vk::PresentInfoKHR presentInfo{*renderSync.renderFinishedSemaphore, *swapchain.swapchain, imageIndex, nullptr};
	checkForBadSwapchain(presentQueue.presentKHR(presentInfo));
}

void HelloTriangleApplication::updateUniformBuffer(uint32_t frameIndex)
{
	static auto startTime{std::chrono::high_resolution_clock::now()};

	auto currentTime{std::chrono::high_resolution_clock::now()};
	float time{std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count()};

	UniformBufferObject ubo{};
	ubo.model = glm::transpose(glm::rotate(glm::scale(glm::mat4{1.0f}, glm::vec3{5.f}), time * glm::radians(90.f), glm::vec3{0.0f, 0.0f, 1.0f}));
	ubo.view = glm::transpose(glm::lookAt(glm::vec3{2.0f}, glm::vec3{0.f}, glm::vec3{0.f, 0.f, 1.f}));
	ubo.projection = glm::transpose(glm::perspective(glm::radians(45.f),
	                                  static_cast<float>(swapchain.extent.width) / static_cast<float>(swapchain.extent.
	                                                                                                            height), 0.1f, 100.f));
	ubo.projection[1][1] *= -1;

	std::memcpy(uniformBuffersMapped[frameIndex], &ubo, sizeof(ubo));
}

void HelloTriangleApplication::createUniformBuffers()
{
	vk::DeviceSize bufferSize{sizeof(UniformBufferObject)};

	uniformBuffers.clear();
	uniformBuffersMapped.clear();

	uniformBuffers.reserve(maxFramesInFlight);
	uniformBuffersMapped.reserve(maxFramesInFlight);

	for (size_t i = 0; i < maxFramesInFlight; ++i) // TODO: Ranges
	{
		Buffer bufferNew{device, physicalDevice, bufferSize, vk::BufferUsageFlagBits::eUniformBuffer, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent};
		uniformBuffersMapped.emplace_back(bufferNew.memory.mapMemory(0, bufferSize, {}));
		uniformBuffers.emplace_back(std::move(bufferNew)); // TODO
	}
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
	std::vector<vk::DescriptorSetLayout> layouts(maxFramesInFlight, descriptorSetLayout);
	vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo{descriptorPool, layouts};

	descriptorSets = device.allocateDescriptorSets(descriptorSetAllocateInfo);

	for (size_t i = 0; i < maxFramesInFlight; ++i)
	{
		vk::DescriptorBufferInfo descriptorBufferInfo{uniformBuffers[i].vkBuffer, 0, sizeof(UniformBufferObject)};
		vk::DescriptorImageInfo descriptorImageInfo{texture->sampler, texture.value().imageView, vk::ImageLayout::eShaderReadOnlyOptimal};

		std::array<vk::WriteDescriptorSet, 2> descriptorWrites{
			vk::WriteDescriptorSet{descriptorSets[i], 0, 0, vk::DescriptorType::eUniformBuffer, nullptr, descriptorBufferInfo, nullptr},
			vk::WriteDescriptorSet{descriptorSets[i], 1, 0, vk::DescriptorType::eCombinedImageSampler, descriptorImageInfo, nullptr, nullptr}
		};

		device.updateDescriptorSets(descriptorWrites, nullptr);
	}
}

void HelloTriangleApplication::loadModel()
{
	mesh = Mesh{*this, modelPath};
}

void HelloTriangleApplication::createTextureImage()
{
	texture = TextureImage{texturePath, *this};
}
