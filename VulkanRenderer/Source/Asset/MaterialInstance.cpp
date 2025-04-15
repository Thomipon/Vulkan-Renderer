//
// Created by Thomas on 09/04/2025.
//

#include "MaterialInstance.hpp"

#include "Material.hpp"
#include "ShaderCompilation/ShaderCursor.hpp"

MaterialInstance::MaterialInstance(const AssetHandle<Material>& parentMaterial)
	: parentMaterial(parentMaterial), shaderObject(parentMaterial->shaderLayout)
{
}

ShaderCursor MaterialInstance::getShaderCursor()
{
	return ShaderCursor{&shaderObject};
}
