//
// Created by Thomas on 09/04/2025.
//

#include "MaterialInstance.hpp"

#include "Material.hpp"
#include "ShaderCompilation/ShaderCursor.hpp"

MaterialInstance::MaterialInstance(const AssetHandle<Material>& parentMaterial, const std::string_view& name)
	: AssetBase(name), parentMaterial(parentMaterial), shaderObject(parentMaterial->shaderLayout)
{
}

ShaderCursor MaterialInstance::getShaderCursor()
{
	return ShaderCursor{&shaderObject};
}
