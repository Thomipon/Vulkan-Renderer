#pragma once
#include <vector>
#include <filesystem>

#include "Vertex.hpp"


class Mesh {

public:
	Mesh(const std::filesystem::path& sourcePath);

	std::vector<Vertex> vertices{};
	std::vector<uint32_t> indices{};

	vk::raii::Buffer vertexBuffer;
	vk::raii::DeviceMemory vertexBufferMemory;

	vk::raii::Buffer indexBuffer;
	vk::raii::DeviceMemory indexBufferMemory;

private:
	void loadFromFile(const std::filesystem::path& sourcePath);
};
