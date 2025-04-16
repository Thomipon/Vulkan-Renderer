#pragma once

#include <array>
#include <vector>

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include "VulkanBackend.hpp"

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoord;

    static vk::VertexInputBindingDescription getBindingDescription()
    {
        return vk::VertexInputBindingDescription{0, sizeof(Vertex), vk::VertexInputRate::eVertex};
    }

    static std::array<vk::VertexInputAttributeDescription, 3> getAttributeDescriptions() // TODO: These smell constexpr
    {
        return {
            vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position)},
            vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal)},
            vk::VertexInputAttributeDescription{2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, texCoord)}
        };
    }

    bool operator==(const Vertex& other) const
    {
        return position == other.position && normal == other.normal && texCoord == other.texCoord;
    }
};

namespace std
{
    template <>
    struct hash<Vertex>
    {
        size_t operator()(Vertex const& vertex) const noexcept
        {
            return ((hash<glm::vec3>()(vertex.position) ^
                    (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

/*inline const std::vector<Vertex> vertices = {
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
};*/
