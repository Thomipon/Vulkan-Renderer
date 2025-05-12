//
// Created by Thomas on 07/05/2025.
//

#include "LayeredMaterialsDemo.hpp"

#include <imgui.h>
#include <glm/common.hpp>
#include <glm/gtc/vec1.hpp>

#include "ShaderCompilation/ShaderCursor.hpp"

void SimplePBRMaterial::DrawImGui()
{
	if (materialHandle)
	{
		ImGui::SeparatorText("Simple PBR Material");

		const ShaderCursor materialCursor{(*materialHandle)->getShaderCursor().field("gMaterial")};

		ImGui::Text("Albedo:");
		if (ImGui::ColorEdit3("##albedo", reinterpret_cast<float*>(&albedo)))
		{
			materialCursor.field("albedo").write(albedo);
		}

		ImGui::Text("F0:");
		if (ImGui::ColorEdit3("##f0", reinterpret_cast<float*>(&f0)))
		{
			materialCursor.field("f0").write(f0);
		}

		ImGui::Text("F90:");
		if (ImGui::ColorEdit3("##f90", reinterpret_cast<float*>(&f90)))
		{
			materialCursor.field("f90").write(f90);
		}

		ImGui::Text("Roughness:");
		if (ImGui::SliderFloat("##roughness", &roughness, 0.f, 1.f))
		{
			materialCursor.field("roughness").write(glm::vec1{roughness});
		}

		ImGui::Text("Emissive:");
		if (ImGui::ColorEdit3("##emissive", reinterpret_cast<float*>(&emissiveColor), ImGuiColorEditFlags_HDR))
		{
			materialCursor.field("emissiveColor").write(emissiveColor);
		}
	}
}

void SimplePBRMaterial::Initialize(AssetHandle<MaterialInstance>&& material)
{
	materialHandle = std::move(material);

	const ShaderCursor materialCursor{(*materialHandle)->getShaderCursor().field("gMaterial")};
	materialCursor.field("albedo").write(albedo);
	materialCursor.field("f0").write(f0);
	materialCursor.field("f90").write(f90);
	materialCursor.field("roughness").write(glm::vec1{roughness});
	materialCursor.field("emissiveColor").write(emissiveColor);
}

void SimpleHorizontalBlendDemo::DrawImGui()
{
	if (materialHandle)
	{
		ImGui::SeparatorText("Simple Horizontal Blend Material");

		const ShaderCursor materialCursor{(*materialHandle)->getShaderCursor().field("gMaterial")};

		ImGui::Text("Albedo1:");
		if (ImGui::ColorEdit3("##albedo1", reinterpret_cast<float*>(&albedo1)))
		{
			materialCursor.field("albedo1").write(albedo1);
		}

		ImGui::Text("Metallic1:");
		if (ImGui::SliderFloat("##metallic1", &metallic1, 0.f, 1.f))
		{
			materialCursor.field("metallic1").write(glm::vec1{metallic1});
		}

		ImGui::Text("Roughness1:");
		if (ImGui::SliderFloat("##roughness1", &roughness1, 0.f, 1.f))
		{
			materialCursor.field("roughness1").write(glm::vec1{roughness1});
		}

		ImGui::Text("Albedo2:");
		if (ImGui::ColorEdit3("##albedo2", reinterpret_cast<float*>(&albedo2)))
		{
			materialCursor.field("albedo2").write(albedo2);
		}

		ImGui::Text("Metallic2:");
		if (ImGui::SliderFloat("##metallic2", &metallic2, 0.f, 1.f))
		{
			materialCursor.field("metallic2").write(glm::vec1{metallic2});
		}

		ImGui::Text("Roughness2:");
		if (ImGui::SliderFloat("##roughness2", &roughness2, 0.f, 1.f))
		{
			materialCursor.field("roughness2").write(glm::vec1{roughness2});
		}

		ImGui::Text("BlendScale:");
		if (ImGui::SliderFloat("##blendScale", &blendScale, 0.f, 1000.f))
		{
			materialCursor.field("blendScale").write(glm::vec1{blendScale});
		}
	}
}

void SimpleHorizontalBlendDemo::Initialize(AssetHandle<MaterialInstance>&& material)
{
	materialHandle = std::move(material);

	const ShaderCursor materialCursor{(*materialHandle)->getShaderCursor().field("gMaterial")};
	materialCursor.field("albedo1").write(albedo1);
	materialCursor.field("metallic1").write(metallic1);
	materialCursor.field("roughness1").write(roughness1);
	materialCursor.field("albedo2").write(albedo2);
	materialCursor.field("metallic2").write(metallic2);
	materialCursor.field("roughness2").write(roughness2);
	materialCursor.field("blendScale").write(glm::vec1{blendScale});
}

void SimpleVerticalBlendDemo::DrawImGui()
{
	if (materialHandle)
	{
		ImGui::PushID("VerticalBlend");

		ImGui::SeparatorText("Simple Vertical Layer Material");

		const ShaderCursor materialCursor{(*materialHandle)->getShaderCursor().field("gMaterial")};

		ImGui::Text("Bottom Albedo:");
		if (ImGui::ColorEdit3("##bottomAlbedo", reinterpret_cast<float*>(&bottomAlbedo)))
		{
			materialCursor.field("bottomAlbedo").write(bottomAlbedo);
		}

		ImGui::Text("Bottom Metallic:");
		if (ImGui::SliderFloat("##bottomMetallic", &bottomMetallic, 0.f, 1.f))
		{
			materialCursor.field("bottomMetallic").write(glm::vec1{bottomMetallic});
		}

		ImGui::Text("Bottom Roughness:");
		if (ImGui::SliderFloat("##bottomRoughness", &bottomRoughness, 0.f, 1.f))
		{
			materialCursor.field("bottomRoughness").write(glm::vec1{bottomRoughness});
		}

		ImGui::Text("Bottom Emissive:");
		if (ImGui::ColorEdit3("##bottomEmissive", reinterpret_cast<float*>(&bottomEmissive), ImGuiColorEditFlags_HDR))
		{
			materialCursor.field("bottomEmissive").write(bottomEmissive);
		}

		ImGui::Text("Top Absorption:");
		if (ImGui::ColorEdit3("##topAbsorption", reinterpret_cast<float*>(&topAbsorption), ImGuiColorEditFlags_HDR))
		{
			topAbsorption = max(topAbsorption, 0.f);
			materialCursor.field("topAbsorption").write(topAbsorption * .1f);
		}

		ImGui::Text("Top Coverage:");
		if (ImGui::SliderFloat("##topCoverage", &topCoverage, 0.f, 1.f))
		{
			materialCursor.field("topCoverage").write(glm::vec1{topCoverage});
		}

		ImGui::Text("Top Thickness:");
		if (ImGui::SliderFloat("##topThickness", &topThickness, 0.f, 100.f))
		{
			materialCursor.field("topThickness").write(glm::vec1{topThickness});
		}

		ImGui::Text("Top Roughness:");
		if (ImGui::SliderFloat("##topRoughness", &topRoughness, 0.f, 1.f))
		{
			materialCursor.field("topRoughness").write(glm::vec1{topRoughness});
		}

		ImGui::Text("Top IOR:");
		if (ImGui::SliderFloat("##topIor", &topIor, .1f, 5.f))
		{
			materialCursor.field("topIor").write(glm::vec1{topIor});
		}

		ImGui::Text("Top F0:");
		if (ImGui::ColorEdit3("##topF0", reinterpret_cast<float*>(&topF0)))
		{
			materialCursor.field("topF0").write(topF0);
		}

		ImGui::PopID();
	}
}

void SimpleVerticalBlendDemo::Initialize(AssetHandle<MaterialInstance>&& material)
{
	materialHandle = std::move(material);

	const ShaderCursor materialCursor{(*materialHandle)->getShaderCursor().field("gMaterial")};
	materialCursor.field("bottomAlbedo").write(bottomAlbedo);
	materialCursor.field("bottomMetallic").write(bottomMetallic);
	materialCursor.field("bottomRoughness").write(bottomRoughness);
	materialCursor.field("bottomEmissive").write(bottomEmissive);
	materialCursor.field("topCoverage").write(topCoverage);
	materialCursor.field("topThickness").write(topThickness);
	materialCursor.field("topRoughness").write(topRoughness);
	materialCursor.field("topAbsorption").write(topAbsorption * .1f);
	materialCursor.field("topIor").write(topIor);
	materialCursor.field("topF0").write(topF0);
}

void OpalDemo::DrawImGui()
{
	if (materialHandle)
	{
		ImGui::PushID("Opal");

		ImGui::SeparatorText("Opal Material");

		const ShaderCursor materialCursor{(*materialHandle)->getShaderCursor().field("gMaterial")};

		ImGui::Text("Texture Tiling:");
		if (ImGui::DragFloat("##textureTiling", &textureTiling, .1f, 0.f, 0.f))
		{
			materialCursor.field("textureTiling").write(glm::vec1{textureTiling});
		}

		ImGui::Text("Hue Shift:");
		if (ImGui::DragFloat("##hueShift", &hueShift, .1f, 0.f, 0.f))
		{
			materialCursor.field("hueShift").write(glm::vec1{hueShift});
		}

		ImGui::Text("Hue Scale:");
		if (ImGui::DragFloat("##hueScale", &hueScale, .1f, 0.f, 0.f))
		{
			materialCursor.field("hueScale").write(glm::vec1{hueScale});
		}

		ImGui::Text("Saturation:");
		if (ImGui::SliderFloat("##saturation", &saturation, 0.f, 1.f))
		{
			materialCursor.field("saturation").write(glm::vec1{saturation});
		}

		ImGui::Text("Brightness:");
		if (ImGui::SliderFloat("##brightness", &brightness, 0.f, 100.f))
		{
			materialCursor.field("brightness").write(glm::vec1{brightness});
		}

		ImGui::Text("Specular Intensity:");
		if (ImGui::SliderFloat("##specularIntensity", &specularIntensity, 0.f, 1.f))
		{
			materialCursor.field("specularIntensity").write(glm::vec1{specularIntensity});
		}

		ImGui::Text("Bottom Albedo:");
		if (ImGui::ColorEdit3("##bottomAlbedo", reinterpret_cast<float*>(&bottomAlbedo)))
		{
			materialCursor.field("bottomAlbedo").write(bottomAlbedo);
		}

		ImGui::Text("Roughness Threshold:");
		if (ImGui::SliderFloat("##roughnessThreshold", &roughnessThreshold, 0.f, 1.f))
		{
			materialCursor.field("roughnessThreshold").write(glm::vec1{roughnessThreshold});
		}

		ImGui::Text("Roughness Center:");
		if (ImGui::SliderFloat("##roughnessCenter", &roughnessCenter, 0.f, 1.f))
		{
			materialCursor.field("roughnessCenter").write(glm::vec1{roughnessCenter});
		}

		ImGui::Text("Height Scale:");
		if (ImGui::SliderFloat("##heightScale", &heightScale, -2.f, 2.f))
		{
			materialCursor.field("heightScale").write(glm::vec1{heightScale});
		}

		ImGui::Text("Height Bias:");
		if (ImGui::SliderFloat("##heightBias", &heightBias, -10.f, 10.f))
		{
			materialCursor.field("heightBias").write(glm::vec1{heightBias});
		}

		ImGui::Text("Coat IOR:");
		if (ImGui::SliderFloat("##coatIOR", &coatIOR, .1f, 5.f))
		{
			materialCursor.field("coatIOR").write(glm::vec1{coatIOR});
		}

		ImGui::Text("Coat inner Thickness:");
		if (ImGui::SliderFloat("##coatInnerThickness", &coatInnerThickness, 0.f, 100.f))
		{
			materialCursor.field("coatInnerThickness").write(glm::vec1{coatInnerThickness});
		}

		ImGui::Text("Coat outer Thickness:");
		if (ImGui::SliderFloat("##coatOuterThickness", &coatOuterThickness, 0.f, 100.f))
		{
			materialCursor.field("coatOuterThickness").write(glm::vec1{coatOuterThickness});
		}

		ImGui::Text("Coat Thickness Exponent:");
		if (ImGui::SliderFloat("##coatThicknessExponent", &coatThicknessExponent, 0.f, 10.f))
		{
			materialCursor.field("coatThicknessExponent").write(glm::vec1{coatThicknessExponent});
		}

		ImGui::Text("Coat Roughness:");
		if (ImGui::SliderFloat("##coatRoughness", &coatRoughness, 0.f, 1.f))
		{
			materialCursor.field("coatRoughness").write(glm::vec1{coatRoughness});
		}

		ImGui::Text("Coat Absorption:");
		if (ImGui::ColorEdit3("##coatAbsorption", reinterpret_cast<float*>(&coatAbsorption), ImGuiColorEditFlags_HDR))
		{
			coatAbsorption = max(coatAbsorption, 0.f);
			materialCursor.field("coatAbsorption").write(coatAbsorption * .1f);
		}

		ImGui::PopID();
	}
}

void OpalDemo::Initialize(AssetHandle<MaterialInstance>&& material)
{
	materialHandle = std::move(material);

	const ShaderCursor materialCursor{(*materialHandle)->getShaderCursor().field("gMaterial")};
	materialCursor.field("textureTiling").write(textureTiling);
	materialCursor.field("normalMap").writeTexture(normalMap);
	materialCursor.field("armMap").writeTexture(armMap);
	materialCursor.field("heightMap").writeTexture(heightMap);
	materialCursor.field("hueShift").write(hueShift);
	materialCursor.field("hueScale").write(hueScale);
	materialCursor.field("saturation").write(saturation);
	materialCursor.field("brightness").write(brightness);
	materialCursor.field("specularIntensity").write(specularIntensity);
	materialCursor.field("bottomAlbedo").write(bottomAlbedo);
	materialCursor.field("roughnessThreshold").write(roughnessThreshold);
	materialCursor.field("roughnessCenter").write(roughnessCenter);
	materialCursor.field("heightScale").write(heightScale);
	materialCursor.field("heightBias").write(heightBias);
	materialCursor.field("coatIOR").write(coatIOR);
	materialCursor.field("coatInnerThickness").write(coatInnerThickness);
	materialCursor.field("coatOuterThickness").write(coatOuterThickness);
	materialCursor.field("coatThicknessExponent").write(coatThicknessExponent);
	materialCursor.field("coatRoughness").write(coatRoughness);
	materialCursor.field("coatAbsorption").write(coatAbsorption * .1f);
}
