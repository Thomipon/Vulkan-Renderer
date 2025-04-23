//
// Created by Thomas on 30/03/2025.
//

#include "Scene.hpp"

#include <imgui.h>

void Scene::drawImGui()
{
	ImGui::Begin("Scene");

	ImGui::Text("Scene");

	ImGui::BeginChild("Camera");

	camera.drawImGui();

	ImGui::EndChild();

	ImGui::BeginChild("Models");

	for (auto& model : models)
	{

	}

	ImGui::EndChild();

	ImGui::End();
}
