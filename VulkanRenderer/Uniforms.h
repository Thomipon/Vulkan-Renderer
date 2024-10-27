#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

struct UniformBufferObject
{
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};