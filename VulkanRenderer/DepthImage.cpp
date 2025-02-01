
#include "DepthImage.h"

#include "PhysicalDeviceHelper.hpp"

vk::Format DepthImage::findDepthFormat(const vk::PhysicalDevice &physicalDevice) {
    auto result{findSupportedFormat(physicalDevice, {vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD32Sfloat}, vk::ImageTiling::eOptimal,
                               vk::FormatFeatureFlagBits::eDepthStencilAttachment)};
    if (result.empty()) {
        throw std::runtime_error("failed to find depth image!");
    }
    return result.value();
}
