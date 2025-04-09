#pragma once
#include "ShaderCompilation/VulkanShaderObject.hpp"

struct ShaderCursor;
class Material;

class MaterialInstance
{
public:
	MaterialInstance(const std::shared_ptr<Material>& parentMaterial);

	ShaderCursor getShaderCursor();

	VulkanShaderObject shaderObject;
	std::shared_ptr<Material> parentMaterial;
};
