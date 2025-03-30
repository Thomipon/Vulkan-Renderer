#pragma once
#include "Asset/Material.hpp"
#include "Asset/Mesh.hpp"
#include "Core/Transform.hpp"

class Model {
public:
	Transform transform;

	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material; // TODO: Support multiple material slots
};
