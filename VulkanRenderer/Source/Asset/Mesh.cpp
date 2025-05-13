#include "Mesh.hpp"

#include <assimp/Importer.hpp>

//#define TINYOBJLOADER_IMPLEMENTATION
//#include "tiny_obj_loader.h"
#include "assimp/postprocess.h"
#include "assimp/scene.h"

RawMesh::RawMesh(const std::filesystem::path& sourcePath)
	: RawMesh(loadFromFile(sourcePath))
{
}

RawMesh::RawMesh(std::vector<Vertex>&& vertices, std::vector<Index>&& indices)
	: vertices(std::move(vertices)), indices(std::move(indices))
{
}

glm::vec3 assimpVectorToGLM(const aiVector3D& vec)
{
	return glm::vec3{vec.x, vec.y, vec.z};
}

RawMesh RawMesh::loadFromFile(const std::filesystem::path& sourcePath)
{
	std::vector<Vertex> vertices{};
	std::vector<Index> indices{};

	Assimp::Importer importer{};

	const aiScene* scene{importer.ReadFile(sourcePath.string(), aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_JoinIdenticalVertices)};
	if (scene && scene->mNumMeshes > 0)
	{
		const aiMesh* mesh{scene->mMeshes[0]};
		vertices.reserve(mesh->mNumVertices);
		for (unsigned i = 0; i < mesh->mNumVertices; ++i)
		{
			Vertex& vertex{vertices.emplace_back()};
			vertex.position = assimpVectorToGLM(mesh->mVertices[i]);
			vertex.normal = assimpVectorToGLM(mesh->mNormals[i]);
			vertex.tangent = assimpVectorToGLM(mesh->mTangents[i]);
			vertex.texCoord = assimpVectorToGLM(mesh->mTextureCoords[0][i]);
		}
		indices.reserve(mesh->mNumFaces * 3);
		for (unsigned i = 0; i < mesh->mNumFaces; ++i)
		{
			const aiFace& face{mesh->mFaces[i]};
			if (face.mNumIndices == 3)
			{
				indices.emplace_back(face.mIndices[0]);
				indices.emplace_back(face.mIndices[1]);
				indices.emplace_back(face.mIndices[2]);
			}
		}
	}

	/*tinyobj::attrib_t attrib;
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
				.normal = {
					attrib.normals[3 * index.normal_index + 0],
					attrib.normals[3 * index.normal_index + 1],
					attrib.normals.at(3 * index.normal_index + 2)
				},
				.tangent = {1.f, 0.f, 0.f},
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
	}*/

	vertices.shrink_to_fit();

	return {std::move(vertices), std::move(indices)};
}


Mesh::Mesh(const Renderer& app, const std::filesystem::path& sourcePath)
	: AssetBase(sourcePath.filename().string()),
	  rawMesh(sourcePath),
	  vertexBuffer(app, rawMesh.vertices, vk::BufferUsageFlagBits::eVertexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal),
	  indexBuffer(app, rawMesh.indices, vk::BufferUsageFlagBits::eIndexBuffer, vk::MemoryPropertyFlagBits::eDeviceLocal)
{
}
