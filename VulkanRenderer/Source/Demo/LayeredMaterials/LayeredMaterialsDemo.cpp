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
