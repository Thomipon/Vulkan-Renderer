//
// Created by Thomas on 07/05/2025.
//

#include "LayeredMaterialsDemo.hpp"

#include <imgui.h>
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

		ImGui::Text("topTransmittance:");
		if (ImGui::ColorEdit3("##topTransmittance", reinterpret_cast<float*>(&topTransmittance), ImGuiColorEditFlags_HDR))
		{
			materialCursor.field("topTransmittance").write(topTransmittance);
		}

		ImGui::Text("Top Coverage:");
		if (ImGui::SliderFloat("##topCoverage", &topCoverage, 0.f, 1.f))
		{
			materialCursor.field("topCoverage").write(glm::vec1{topCoverage});
		}

		ImGui::Text("Top Thickness:");
		if (ImGui::SliderFloat("##topThickness", &topThickness, 0.f, 1.f))
		{
			materialCursor.field("topThickness").write(glm::vec1{topThickness});
		}

		ImGui::Text("Top Roughness:");
		if (ImGui::SliderFloat("##topRoughness", &topRoughness, 0.f, 1.f))
		{
			materialCursor.field("topRoughness").write(glm::vec1{topRoughness});
		}

		ImGui::Text("Top IOR:");
		if (ImGui::SliderFloat("##topIor", &topIor, 0.f, 1.f))
		{
			materialCursor.field("topIor").write(glm::vec1{topIor});
		}

		ImGui::Text("Top F0:");
		if (ImGui::ColorEdit3("##topF0", reinterpret_cast<float*>(&topF0)))
		{
			materialCursor.field("topF0").write(topF0);
		}
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
	materialCursor.field("topTransmittance").write(topTransmittance);
	materialCursor.field("topIor").write(topIor);
	materialCursor.field("topF0").write(topF0);
}
