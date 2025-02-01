#pragma once

#include <string>
#include <vector>

#include "VulkanBackend.hpp"

#include "Renderer.hpp"
#include "Texture.hpp"
#include "Vertex.hpp"

class HelloTriangleApplication : public Renderer
{
public:
    void run();

    inline static uint32_t width{1920};
    inline static uint32_t height{1080};
    inline static uint32_t maxFramesInFlight{2};

    inline static std::string modelPath{"Meshes/viking_room.obj"};
    inline static std::string texturePath{"Textures/viking_room.png"};
private:
    void initVulkan();
    void mainLoop();

    void recreateSwapchain();
    vk::Result checkForBadSwapchain(vk::Result inResult);

    vk::raii::DescriptorSetLayout descriptorSetLayout{VK_NULL_HANDLE};
    vk::raii::PipelineLayout pipelineLayout{VK_NULL_HANDLE};
    vk::raii::Pipeline graphicsPipeline{VK_NULL_HANDLE};
    void createGraphicsPipeline();

    std::vector<vk::raii::CommandBuffer> commandBuffers{};
    void createCommandBuffers();

    void recordCommandBuffer(const vk::CommandBuffer& commandBuffer, uint32_t imageIndex);

    uint32_t currentFrame{0};
    void drawFrame();

    void updateUniformBuffer(uint32_t frameIndex);

    std::vector<vk::raii::Semaphore> imageAvailableSemaphores{};
    std::vector<vk::raii::Semaphore> renderFinishedSemaphores{};
    std::vector<vk::raii::Fence> inFlightFences{};
    void createSyncObjects();

    bool frameBufferResized{false};
    
    vk::raii::Buffer vertexBuffer{VK_NULL_HANDLE};
    vk::raii::DeviceMemory vertexBufferMemory{VK_NULL_HANDLE};
    void createVertexBuffer();

    vk::raii::Buffer indexBuffer{VK_NULL_HANDLE};
    vk::raii::DeviceMemory indexBufferMemory{VK_NULL_HANDLE};
    void createIndexBuffer();

    std::vector<vk::raii::Buffer> uniformBuffers;
    std::vector<vk::raii::DeviceMemory> uniformBufferMemories;
    std::vector<void*> uniformBuffersMapped;
    void createUniformBuffers();

    void copyBuffer(vk::Buffer sourceBuffer, vk::Buffer destinationBuffer, vk::DeviceSize size);
    
    void createDescriptorSetLayout();

    vk::raii::DescriptorPool descriptorPool{VK_NULL_HANDLE};
    void createDescriptorPool();

    std::vector<vk::raii::DescriptorSet> descriptorSets{};
    void createDescriptorSets();

    Texture texture;
    void createTextureImage();

    vk::raii::Sampler textureSampler{VK_NULL_HANDLE};
    void createTextureSampler();
    
    Texture depthImage;
    void createDepthResources();

    std::vector<Vertex> meshVertices{};
    std::vector<uint32_t> meshIndices{};
    void loadModel();
};
