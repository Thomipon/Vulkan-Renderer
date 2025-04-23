//
// Created by Thomas on 30/03/2025.
//

#include "Scene.hpp"

#include <imgui.h>
#include <ranges>

void Scene::drawImGui()
{
	ImGui::Begin("Scene");

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

	ImGui::PushID("lights");

	lightEnvironment.drawImGui();

	ImGui::PopID();

	ImGui::End();
}
