#pragma once
#include "VulkanBackend.hpp"

class Window;
struct ImGui_ImplVulkan_InitInfo;

class ImGUI
{
public:
    ImGUI(const Window& window, ImGui_ImplVulkan_InitInfo& initInfo);
    ~ImGUI();

    void newFrame();
    void Render(const vk::CommandBuffer& commandBuffer);
};
