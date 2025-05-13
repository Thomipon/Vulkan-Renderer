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
	glm::vec3 albedo{.653f, .052f, .415f};
	glm::vec3 f0{.04f};
	glm::vec3 f90{1.f};
	float roughness{.226};
	glm::vec3 emissiveColor{0.f};
};

class SimpleHorizontalBlendDemo : public DemoMaterialBase
{
public:
	virtual void DrawImGui() override;
	virtual void Initialize(AssetHandle<MaterialInstance>&& material) override;

private:
	glm::vec3 albedo1{.13f, .57f, .643f};
	float metallic1{1.f};
	float roughness1{.313f};
	glm::vec3 albedo2{.196f, .838f, 0.f};
	float metallic2{0.f};
	float roughness2{.5f};
	float blendScale{100.f};
};

class SimpleVerticalBlendDemo : public DemoMaterialBase
{
public:
	virtual void DrawImGui() override;
	virtual void Initialize(AssetHandle<MaterialInstance>&& material) override;

private:
	glm::vec3 bottomAlbedo{.567f, .313f, 0.f};
	float bottomMetallic{1.f};
	float bottomRoughness{.475f};
	glm::vec3 bottomEmissive{0.f};

	float topCoverage{1.f};
	float topThickness{22.f};
	float topRoughness{.197f};
	glm::vec3 topAbsorption{0.f, .235f, .082f};
	float topIor{1.263f};
	glm::vec3 topF0{0.06f};
};

class OpalDemo : public DemoMaterialBase
{
public:
	virtual void DrawImGui() override;
	virtual void Initialize(AssetHandle<MaterialInstance>&& material) override;

	OpalDemo(TextureImage&& normalMap, TextureImage&& armMap, TextureImage&& heightMap)
	: normalMap(std::move(normalMap)), armMap(std::move(armMap)), heightMap(std::move(heightMap)) {}

private:
	float textureTiling{3.2f};
	TextureImage normalMap;
	TextureImage armMap;
	TextureImage heightMap;
	float hueShift{0.f};
	float hueScale{3.2f};
	float saturation{.922f};
	float brightness{.02f};
	float specularIntensity{.839f};
	glm::vec3 bottomAlbedo{.362f};
	float roughnessThreshold{.413f};
	float roughnessCenter{.12f};
	float heightScale{.097f};
	float heightBias{-.691f};
	float coatIOR{1.31f};
	float coatInnerThickness{9.676f};
	float coatOuterThickness{44.239f};
	float coatThicknessExponent{2.181f};
	float coatRoughness{.127f};
	glm::vec3 coatAbsorption{.276f, .264f, .223f};
};
