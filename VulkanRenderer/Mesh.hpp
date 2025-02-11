#pragma once
#include <vector>
#include <filesystem>

#include "Buffer.hpp"
#include "Vertex.hpp"


class Mesh {

public:
	Mesh(const std::filesystem::path& sourcePath);

	std::vector<Vertex> vertices{};
	std::vector<uint32_t> indices{};

	Buffer vertexBuffer;

	Buffer indexBuffer;

private:
	void loadFromFile(const std::filesystem::path& sourcePath);
};
