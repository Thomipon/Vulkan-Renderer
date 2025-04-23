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

	ImGui::BeginChild("Camera");

	camera.drawImGui();

	ImGui::EndChild();

	ImGui::BeginChild("Models");

	for (const auto& [index, model] : std::ranges::views::enumerate(models))
	{
		ImGui::PushID(static_cast<int>(index));

		ImGui::PopID();
	}

	ImGui::EndChild();

	ImGui::End();
}
