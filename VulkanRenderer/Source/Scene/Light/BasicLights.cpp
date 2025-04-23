//
// Created by Thomas on 30/03/2025.
//

#include "BasicLights.hpp"

#include <imgui.h>

#include "ShaderCompilation/ShaderCursor.hpp"

void PointLight::writeToCursor(const ShaderCursor& cursor) const
{
	cursor.field("position").write(transform.translation);
	cursor.field("color").write(color);
	cursor.field("intensity").write(intensity);
}

void PointLight::drawImGui()
{
	ImGui::SeparatorText("PointLight");
	transform.drawImGui(false, false);
	ImGui::Text("Color:");
	ImGui::ColorEdit3("##color", reinterpret_cast<float*>(&color));
	ImGui::Text("Intensity:");
	ImGui::DragFloat("##intensity", &intensity, 0.1f);
}

void DirectionalLight::writeToCursor(const ShaderCursor& cursor) const
{
	cursor.field("direction").write(normalize(direction));
	cursor.field("color").write(color);
	cursor.field("intensity").write(intensity);
}

void DirectionalLight::drawImGui()
{
	ImGui::SeparatorText("DirectionalLight");
	ImGui::Text("Direction:");
	ImGui::DragFloat3("##direction", reinterpret_cast<float*>(&direction), .1f);
	ImGui::Text("Color:");
	ImGui::ColorEdit3("##color", reinterpret_cast<float*>(&color));
	ImGui::Text("Intensity:");
	ImGui::DragFloat("##intensity", &intensity, 0.1f);
}

void AmbientLight::writeToCursor(const ShaderCursor &cursor) const
{
	cursor.field("color").write(color);
	cursor.field("intensity").write(intensity);
}

void AmbientLight::drawImGui()
{
	ImGui::SeparatorText("AmbientLight");
	ImGui::Text("Color:");
	ImGui::ColorEdit3("##color", reinterpret_cast<float*>(&color));
	ImGui::Text("Intensity:");
	ImGui::DragFloat("##intensity", &intensity, 0.1f);
}
