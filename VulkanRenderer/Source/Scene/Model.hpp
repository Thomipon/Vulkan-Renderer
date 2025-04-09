#pragma once
#include "Asset/Mesh.hpp"
#include "Core/Transform.hpp"

class MaterialInstance;

class Model {
public:
	Transform transform;

	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<MaterialInstance> material; // TODO: Support multiple material slots
};
