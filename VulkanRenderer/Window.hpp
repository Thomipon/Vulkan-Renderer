#pragma once

#include <functional>

#include "VulkanBackend.hpp"
#include <GLFW/glfw3.h>
#include <glm\glm.hpp>

#include "Input/InputHandler.hpp"

class Window
{
public:
    Window(int width, int height, const std::function<void(int, int)>& frameBufferResizedCallback);
    ~Window();
    // TODO: Follow rule of five but I don't know if it should be assignable/copyable/movable

    vk::raii::SurfaceKHR createWindowSurface(const vk::raii::Instance& instance) const;

    bool shouldClose() const;
    static void pollEvents();
    vk::Extent2D getWindowExtent() const;
    glm::vec<2, int> getFramebufferSize() const;
    static void waitEvents();

    void registerInputHandler(InputHandler& newInputHandler);

private:
    GLFWwindow* window;

    std::function<void(int, int)> frameBufferResizedCallback;

    InputHandler* inputHandler{nullptr}; // TODO: This should probably not be a raw pointer but a vector<InputHandler&> just feels too wrong

    static void onFramebufferResized(GLFWwindow* window, int width, int height);
    static void onKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void onMouseMoveCallback(GLFWwindow* window, double xpos, double ypos);
};
