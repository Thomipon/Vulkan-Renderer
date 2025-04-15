#pragma once
#include <vector>
#include <filesystem>

#include "AssetBase.hpp"
#include "Buffer.hpp"
#include "Vertex.hpp"

using Index = uint32_t;

struct RawMesh
{
public:
	explicit RawMesh(const std::filesystem::path& sourcePath);

	std::vector<Vertex> vertices{};
	std::vector<Index> indices{};

private:
	static RawMesh loadFromFile(const std::filesystem::path& sourcePath);
	RawMesh(std::vector<Vertex>&& vertices, std::vector<Index>&& indices);
};

class Mesh : public AssetBase
{
public:
	Mesh(const Renderer& app, const std::filesystem::path& sourcePath);

	RawMesh rawMesh;
	Buffer vertexBuffer;
	Buffer indexBuffer;
};
