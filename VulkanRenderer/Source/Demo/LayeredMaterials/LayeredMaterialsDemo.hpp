#pragma once
#include <glm/vec3.hpp>

#include "TextureImage.hpp"
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

class SimpleVerticalBlendDemo : public DemoMaterialBase
{
public:
	virtual void DrawImGui() override;
	virtual void Initialize(AssetHandle<MaterialInstance>&& material) override;

private:
	glm::vec3 bottomAlbedo{.5, .8, .9};
	float bottomMetallic{0.f};
	float bottomRoughness{.7f};
	glm::vec3 bottomEmissive{0.f};

	float topCoverage{1.f};
	float topThickness{5.f};
	float topRoughness{.1f};
	glm::vec3 topTransmittance{.1f};
	float topIor{1.06f};
	glm::vec3 topF0{0.04f};
};

class OpalDemo : public DemoMaterialBase
{
public:
	virtual void DrawImGui() override;
	virtual void Initialize(AssetHandle<MaterialInstance>&& material) override;

	OpalDemo(TextureImage&& normalMap, TextureImage&& armMap) : normalMap(std::move(normalMap)), armMap(std::move(armMap)) {}

private:
	float textureTiling{2.9f};
	TextureImage normalMap;
	TextureImage armMap;
	float hueShift{1.f};
	float hueScale{3.2f};
	float saturation{.894f};
	float brightness{.04f};
	float specularIntensity{.735f};
	glm::vec3 bottomAlbedo{.362f};
	float coatIOR{1.31f};
	float coatInnerThickness{24.541f};
	float coatOuterThickness{49.788f};
	float coatThicknessExponent{2.181f};
	float coatRoughness{.09f};
	glm::vec3 coatTransmittance{.409f, .399f, .369f};
};
