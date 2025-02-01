#pragma once
#include "Image.hpp"

class DepthImage : public Image
{
public:
	DepthImage(const vk::raii::Device& device, const vk::PhysicalDevice& physicalDevice, vk::Extent2D swapchainExtent);

	static vk::Format findDepthFormat(const vk::PhysicalDevice& physicalDevice);
};
