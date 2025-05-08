//
// Created by Thomas on 30/03/2025.
//

#include "Camera.hpp"

#include <imgui.h>

glm::mat4 Camera::getViewProjection(const glm::vec2& viewExtent) const
{
	const glm::mat4 view = inverse(transform.getMatrix());
	glm::mat4 projection = glm::perspective(glm::radians(fieldOfView), viewExtent.x / viewExtent.y, 0.01f, 100.f);
	projection[1][1] *= -1;
	return projection * view;
}

void Camera::drawImGui()
{
	ImGui::PushID("camera");
	ImGui::SeparatorText("Camera");
	transform.drawImGui(false);
	ImGui::Text("FOV:");
	ImGui::DragFloat("##fov", &fieldOfView);
    ImGui::Text("Exposure:");
	ImGui::DragFloat("##exposure", &exposureValue, .01f, 0.f, 300.f);
	ImGui::PopID();
}
