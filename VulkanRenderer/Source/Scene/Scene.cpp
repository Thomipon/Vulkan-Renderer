//
// Created by Thomas on 30/03/2025.
//

#include "Scene.hpp"

#include <imgui.h>
#include <ranges>

constexpr std::array lightTypes{"PointLight", "DirectionalLight"};
int selectedLightType;

void Scene::drawImGui()
{
	ImGui::Text("Scene");

	camera.drawImGui();

	ImGui::SeparatorText("Models");

	for (const auto& [index, model] : std::ranges::views::enumerate(models))
	{
		ImGui::PushID(static_cast<int>(index));

		model.drawImGui();

		ImGui::PopID();
	}

	ImGui::SeparatorText("Lights");

	if (ImGui::BeginCombo("##lightcombo", lightTypes[selectedLightType]))
	{
		for (int i = 0; i < lightTypes.size(); ++i)
		{
			const bool isSelected = selectedLightType == i;
			if (ImGui::Selectable(lightTypes[i], isSelected))
			{
				selectedLightType = i;
			}
			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	ImGui::SameLine();
	if (ImGui::Button("Add light"))
	{
		switch (selectedLightType)
		{
			case 0:
			// Point light
			{
				lightEnvironment.first.addLight();
			}
			break;
			case 1:
			// Directional light
			{
				lightEnvironment.second.first.addLight();
			}
			break;
			default:
				break;
		}
	}

	ImGui::PushID("lights");

	lightEnvironment.drawImGui();

	ImGui::PopID();
}
