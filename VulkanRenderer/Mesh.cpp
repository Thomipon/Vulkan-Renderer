
#include "Mesh.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

Mesh::Mesh(const std::filesystem::path& sourcePath)
{
	loadFromFile(sourcePath);
}

void Mesh::loadFromFile(const std::filesystem::path& sourcePath)
{
	vertices.clear();
	indices.clear();

	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn;
	std::string error;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, sourcePath.string().c_str()))
	{
		throw std::runtime_error(error);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices;

	for (const auto& shape : shapes)
	{
		vertices.reserve(vertices.size() + shape.mesh.indices.size());
		indices.reserve(indices.size() + shape.mesh.indices.size());

		for (const auto& index : shape.mesh.indices)
		{
			Vertex vertex{
				.position = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]
				},
				.color = {1.f, 1.f, 1.f},
				.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.f - attrib.texcoords[2 * index.texcoord_index + 1]
				}
			};

			if (!uniqueVertices.contains(vertex))
			{
				uniqueVertices[vertex] = vertices.size();
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}
	vertices.shrink_to_fit();
}
