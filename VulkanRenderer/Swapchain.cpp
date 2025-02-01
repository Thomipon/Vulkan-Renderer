#include "Swapchain.hpp"

#include <ranges>

#include "CommandQueues.hpp"
#include "DeviceExtensions.hpp"
#include "Image.hpp"
#include "Window.hpp"

Swapchain::Swapchain(const vk::raii::Device &device, const vk::PhysicalDevice &physicalDevice,
                     const vk::SurfaceKHR &surface, const Window &window,
                     const QueueFamilyIndices &queueIndices, const vk::raii::SwapchainKHR &oldSwapchain) :
	Swapchain(device, getDefaultCreateInfo(physicalDevice, surface, window, queueIndices, oldSwapchain))
{
}

Swapchain::Swapchain(const vk::raii::Device &device, const vk::SwapchainCreateInfoKHR &createInfo) :
	swapchain(device, createInfo),
	images(swapchain.getImages()),
	imageFormat(createInfo.imageFormat),
	extent(createInfo.imageExtent),
	imageViews(Image::createImageViews(device, images, imageFormat, vk::ImageAspectFlagBits::eColor))
{
}

vk::SurfaceFormatKHR Swapchain::chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats)
{
	for (const auto &availableFormat : availableFormats)
	{
		if (availableFormat.format == vk::Format::eR8G8B8Srgb && availableFormat.colorSpace ==
			vk::ColorSpaceKHR::eSrgbNonlinear)
		{
			return availableFormat;
		}
	}

	return availableFormats[0];
}

vk::PresentModeKHR Swapchain::choosePresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes)
{
	for (const auto &availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == vk::PresentModeKHR::eMailbox)
		{
			return availablePresentMode;
		}
	}

	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Swapchain::chooseExtent(const vk::SurfaceCapabilitiesKHR &capabilities, const Window &window)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return capabilities.currentExtent;
	}
	else
	{
		vk::Extent2D actualExtent{window.getWindowExtent()};

		actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
		                                capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
		                                 capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

vk::SwapchainCreateInfoKHR Swapchain::getDefaultCreateInfo(const vk::PhysicalDevice &physicalDevice,
                                                           const vk::SurfaceKHR &surface, const Window &window,
                                                           const QueueFamilyIndices &queueIndices,
                                                           const vk::SwapchainKHR &oldSwapchain)
{
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice, surface);

	vk::SurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapChainSupport.formats);
	vk::PresentModeKHR presentMode = choosePresentMode(swapChainSupport.presentModes);
	vk::Extent2D extent = chooseExtent(swapChainSupport.capabilities, window);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	//const uint32_t queueFamilyList[]{queueIndices.graphicsFamily.value(), queueIndices.presentFamily.value()};
	std::vector<uint32_t> queueFamilyList{};
	vk::SharingMode sharingMode{vk::SharingMode::eExclusive};
	if (queueIndices.graphicsFamily != queueIndices.presentFamily)
	{
		sharingMode = vk::SharingMode::eConcurrent;
		queueFamilyList.reserve(2);
		queueFamilyList.emplace_back(queueIndices.graphicsFamily.value());
		queueFamilyList.emplace_back(queueIndices.presentFamily.value());
	}

	vk::SwapchainCreateInfoKHR swapchainCreateInfo{
		{}, surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace, extent, 1,
		vk::ImageUsageFlagBits::eColorAttachment, sharingMode, queueFamilyList, swapChainSupport.capabilities.currentTransform, vk::CompositeAlphaFlagBitsKHR::eOpaque, presentMode,
		true, oldSwapchain
	};

	return swapchainCreateInfo;
}
