#pragma once

#include "VulkanBackend.hpp"

class RenderSync
{
public:
    RenderSync(const vk::raii::Device& device);

    vk::raii::Semaphore imageAvailableSemaphore;
    vk::raii::Semaphore renderFinishedSemaphore;
    vk::raii::Fence inFlightFence;

};
