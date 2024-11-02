﻿#pragma once

#include <array>
#include <vector>

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{
            .binding = 0,
            .stride = sizeof(Vertex),
            .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
        };

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{
            VkVertexInputAttributeDescription{
                .location = 0,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, position)
            },
            VkVertexInputAttributeDescription{
                .location = 1,
                .binding = 0,
                .format = VK_FORMAT_R32G32B32_SFLOAT,
                .offset = offsetof(Vertex, color)
            },
            VkVertexInputAttributeDescription{
                .location = 2,
                .binding = 0,
                .format = VK_FORMAT_R32G32_SFLOAT,
                .offset = offsetof(Vertex, texCoord)
            }
        };

        return attributeDescriptions;
    }
};

inline const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, .5f}, {1.0f, 0.0f, 0.0f}, {0.f, 1.f}},
    {{0.5f, -0.5f, -.5f}, {0.0f, 1.0f, 0.0f}, {1.f, 1.f}},
    {{0.5f, 0.5f, .5f}, {0.0f, 0.0f, 1.0f}, {0.f, 0.f}},
    {{-0.5f, 0.5f, -.5f}, {1.0f, 1.0f, 1.0f}, {1.f, 0.f}},

    {{1.f, 1.f, 0.f}, {1.0f, 1.0f, 0.0f}, {0.f, 1.f}},
    {{-1.f, 1.f, 0.f}, {1.0f, 0.0f, 1.0f}, {1.f, 1.f}},
    {{1.f, -1.f, 0.f}, {0.0f, 1.0f, 1.0f}, {0.f, 0.f}},
    {{-1.f, -1.f, 0.f}, {0.1f, 0.1f, 0.1f}, {1.f, 0.f}}
};

inline const std::vector<uint32_t> vertindices = {
    0, 1, 2,
    2, 3, 0,
    1, 3, 2,
    3, 1, 0,

    4, 5, 6,
    6, 5, 7
};
