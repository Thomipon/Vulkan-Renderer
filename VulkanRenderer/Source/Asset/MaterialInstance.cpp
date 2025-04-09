//
// Created by Thomas on 09/04/2025.
//

#include "MaterialInstance.hpp"

#include "Material.hpp"
#include "ShaderCompilation/ShaderCursor.hpp"

MaterialInstance::MaterialInstance(const std::shared_ptr<Material>& parentMaterial)
	: shaderObject(parentMaterial->shaderLayout), parentMaterial(parentMaterial)
{
}

ShaderCursor MaterialInstance::getShaderCursor()
{
	return ShaderCursor{&shaderObject};
}
