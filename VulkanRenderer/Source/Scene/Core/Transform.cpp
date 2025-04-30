#include "Transform.hpp"

#include <imgui.h>
#include <glm/gtc/quaternion.hpp>

glm::mat4 Transform::getMatrix() const
{
	return translate(glm::scale(glm::mat4{1.0f}, scale), translation) * mat4_cast(rotation);
}

void Transform::drawImGui(bool drawScale, bool drawRotation, bool drawLocation)
{
	if (drawLocation)
	{
		ImGui::Text("Position:");
		ImGui::DragFloat3("##0", reinterpret_cast<float*>(&translation), .01f);
	}

	if (drawRotation)
	{
		ImGui::Text("Rotation:");
		glm::vec3 eulerVec = degrees(eulerAngles(rotation));
		if (ImGui::DragFloat3("##1", reinterpret_cast<float*>(&eulerVec)))
		{
			rotation = glm::quat{radians(eulerVec)};
		}
	}

	if (drawScale)
	{
		ImGui::Text("Scale:");
		ImGui::DragFloat3("##2", reinterpret_cast<float*>(&scale), .1f);
	}
}
