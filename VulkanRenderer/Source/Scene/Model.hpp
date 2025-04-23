#pragma once
#include "Asset/MaterialInstance.hpp"
#include "Asset/Mesh.hpp"
#include "AssetSystem/AssetHandle.hpp"
#include "Core/Transform.hpp"


class Model
{
public:
	Model(const AssetHandle<Mesh>& mesh, const AssetHandle<MaterialInstance>& material)
		: mesh(mesh),
		  material(material)
	{
	}

	Transform transform;

	AssetHandle<Mesh> mesh;
	AssetHandle<MaterialInstance> material; // TODO: Support multiple material slots

	void drawImGui();
};
