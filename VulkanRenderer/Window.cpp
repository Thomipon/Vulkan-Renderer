#include "Window.hpp"

#include "check.hpp"

Window::Window(int width, int height, GLFWframebuffersizefun frameBufferResizedCallback, void* userPtr)
{
    glfwInit(); // TODO: We might want to separate the glfw instance
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, userPtr);
    glfwSetFramebufferSizeCallback(window, frameBufferResizedCallback);
}

Window::~Window()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

vk::raii::SurfaceKHR Window::createWindowSurface(const vk::raii::Instance& instance) const
{
    VkSurfaceKHR rawSurface{VK_NULL_HANDLE};
    check(static_cast<vk::Result>(glfwCreateWindowSurface(*instance, window, nullptr, &rawSurface)), "Failed to create window surface");

    return vk::raii::SurfaceKHR{instance, rawSurface};
}

bool Window::shouldClose() const
{
    return glfwWindowShouldClose(window);
}

void Window::pollEvents()
{
    glfwPollEvents();
}

vk::Extent2D Window::getWindowExtent() const
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);

    return {
        static_cast<uint32_t>(width),
        static_cast<uint32_t>(height)
    };
}

glm::vec<2, int> Window::getFramebufferSize() const
{
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    return {width, height};
}

void Window::waitEvents()
{
    glfwWaitEvents();
}
