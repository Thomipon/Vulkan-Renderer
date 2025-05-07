#pragma once
#include <glm/vec3.hpp>

#include "Asset/MaterialInstance.hpp"
#include "AssetSystem/AssetHandle.hpp"

class DemoMaterialBase
{
public:
	virtual ~DemoMaterialBase() = default;
	std::optional<AssetHandle<MaterialInstance>> materialHandle;

	virtual void DrawImGui() = 0;
	virtual void Initialize(AssetHandle<MaterialInstance>&& material) = 0;
};

class SimplePBRMaterial : public DemoMaterialBase
{
public:
	virtual void DrawImGui() override;
	virtual void Initialize(AssetHandle<MaterialInstance>&& material) override;

private:
	glm::vec3 albedo{.5f};
	glm::vec3 f0{.04f};
	glm::vec3 f90{1.f};
	float roughness{.5};
	glm::vec3 emissiveColor{0.f};
};

class SimpleHorizontalBlendDemo : public DemoMaterialBase
{
public:
	virtual void DrawImGui() override;
	virtual void Initialize(AssetHandle<MaterialInstance>&& material) override;

private:
	glm::vec3 albedo1{.5f};
	float metallic1{1.f};
	float roughness1{.4f};
	glm::vec3 albedo2{.8f};
	float metallic2{0.f};
	float roughness2{.8f};
	float blendScale{100.f};
};
