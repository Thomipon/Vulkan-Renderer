//
// Created by Thomas on 30/03/2025.
//

#include "Model.hpp"

#include <imgui.h>

void Model::drawImGui()
{
    transform.drawImGui();

    ImGui::Text("Mesh:");
    if (ImGui::BeginCombo("##meshcombo", mesh->getName().data()))
    {
        std::span<const Mesh> meshes{mesh.getAssetManager().getAssetIterator<const Mesh>()};
        for (auto & meshOption : meshes)
        {
            const bool isSelected = meshOption.getName() == mesh->getName();
            if (ImGui::Selectable(meshOption.getName().data(), isSelected))
            {
                mesh = mesh.getAssetManager().loadFromUUID<Mesh>(meshOption.getUUID());
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    ImGui::Text("Material:");
    if (ImGui::BeginCombo("##matcombo", material->getName().data()))
    {
        std::span<const MaterialInstance> materials{material.getAssetManager().getAssetIterator<const MaterialInstance>()};
        for (auto & mat : materials)
        {
            const bool isSelected = mat.getName() == material->getName();
            if (ImGui::Selectable(mat.getName().data(), isSelected))
            {
                material = material.getAssetManager().loadFromUUID<MaterialInstance>(mat.getUUID());
            }
            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }
}
