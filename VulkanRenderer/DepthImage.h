#pragma once
#include "Texture.hpp"

class DepthImage : public Texture {

    public:

    static vk::Format findDepthFormat(const vk::PhysicalDevice& physicalDevice);
};

