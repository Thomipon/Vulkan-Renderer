#pragma once

#include <vector>

#include "Mesh.hpp"
#include "VulkanBackend.hpp"

#include "Renderer.hpp"
#include "TextureImage.hpp"
#include "Vertex.hpp"

class Buffer;

class HelloTriangleApplication : public Renderer
{
public:
    void run();
    inline static uint32_t maxFramesInFlight{2}; // TODO: This is duplicate currently

    inline static std::filesystem::path modelPath{"../../VulkanRenderer/Meshes/Mesh.obj"}; // TODO: Awful file management in many ways. These should be in some asset system
    inline static std::filesystem::path texturePath{"../../VulkanRenderer/Textures/Texture.png"};
private:
    void initVulkan();
    void mainLoop();

    vk::Result checkForBadSwapchain(vk::Result inResult);

    vk::raii::DescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
    vk::raii::PipelineLayout pipelineLayout{VK_NULL_HANDLE};
    vk::raii::Pipeline graphicsPipeline{VK_NULL_HANDLE};
    void createGraphicsPipeline();

    void recordCommandBuffer(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex);

    uint32_t currentFrame{0};
    void drawFrame();

    void updateUniformBuffer(uint32_t frameIndex);

    std::vector<vk::raii::Semaphore> imageAvailableSemaphores{};
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores{};
    std::vector<vk::raii::Fence> inFlightFences{};
    void createSyncObjects();

    std::vector<Buffer> uniformBuffers;
    std::vector<void*> uniformBuffersMapped;
    void createUniformBuffers();
    
    void createDescriptorSetLayout();

    vk::raii::DescriptorPool descriptorPool{VK_NULL_HANDLE};
    void createDescriptorPool();

    std::vector<vk::raii::DescriptorSet> descriptorSets{};
    void createDescriptorSets();

    std::optional<Mesh> mesh;
    void loadModel();

    std::optional<TextureImage> texture;
    void createTextureImage();
};
