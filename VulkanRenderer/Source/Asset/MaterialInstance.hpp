#pragma once
#include "AssetBase.hpp"
#include "Material.hpp"
#include "AssetSystem/AssetHandle.hpp"
#include "ShaderCompilation/VulkanShaderObject.hpp"

struct ShaderCursor;

class MaterialInstance : public AssetBase
{
public:
	explicit MaterialInstance(const AssetHandle<Material>& parentMaterial, const std::string_view& name);

	ShaderCursor getShaderCursor();

	AssetHandle<Material> parentMaterial;
	VulkanShaderObject shaderObject;
};
