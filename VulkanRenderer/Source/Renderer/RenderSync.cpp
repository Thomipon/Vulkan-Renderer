//
// Created by Thomas on 07.04.2025.
//

#include "RenderSync.hpp"

RenderSync::RenderSync(const vk::raii::Device &device)
    : imageAvailableSemaphore(device, {}),
      renderFinishedSemaphore(device, {}),
      inFlightFence(device, {vk::FenceCreateFlagBits::eSignaled})
{
}
