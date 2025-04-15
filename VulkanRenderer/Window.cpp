#include "Window.hpp"

#include "check.hpp"

Window::Window(int width, int height, const std::function<void(int, int)>& frameBufferResizedCallback)
    : frameBufferResizedCallback(frameBufferResizedCallback)
{
    glfwInit(); // TODO: We might want to separate the glfw instance
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, onFramebufferResized);

    glfwSetKeyCallback(window, onKeyCallback);
    glfwSetCursorPosCallback(window, onMouseMoveCallback);
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

void Window::registerInputHandler(InputHandler& newInputHandler)
{
    inputHandler = &newInputHandler;
}

void Window::onFramebufferResized(GLFWwindow* window, int width, int height)
{
    const Window* self{static_cast<Window*>(glfwGetWindowUserPointer(window))};
    self->frameBufferResizedCallback(width, height);
}

void Window::onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    const Window* self{static_cast<Window*>(glfwGetWindowUserPointer(window))};

    if (self->inputHandler)
    {
        self->inputHandler->glfwOnKeyPressed(key, scancode, action, mods);
    }
}

void Window::onMouseMoveCallback(GLFWwindow* window, double xpos, double ypos)
{
    const Window* self{static_cast<Window*>(glfwGetWindowUserPointer(window))};

    if (self->inputHandler)
    {
        self->inputHandler->glfwOnMouseMove(xpos, ypos);
    }
}
