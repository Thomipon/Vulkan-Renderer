#pragma once

#include "VulkanBackend.hpp"
#include <GLFW/glfw3.h>
#include <glm\glm.hpp>

class Window
{
public:
    Window(int width, int height, GLFWframebuffersizefun frameBufferResizedCallback, void* userPtr);
    ~Window();
    // TODO: Follow rule of five but I don't know if it should be assignable/copyable/movable

    vk::raii::SurfaceKHR createWindowSurface(const vk::raii::Instance& instance) const;

    bool shouldClose() const;
    static void pollEvents();
    vk::Extent2D getWindowExtent() const;
    glm::vec<2, int> getFramebufferSize() const;
    static void waitEvents();

private:
    GLFWwindow* window;
};
