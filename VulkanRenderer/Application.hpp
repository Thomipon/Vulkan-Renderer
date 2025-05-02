#pragma once

#include "Renderer.hpp"
#include "AssetSystem/AssetManager.hpp"
#include "Scene/Scene.hpp"

class Application : public Renderer
{
public:
    void run();

private:
    void mainLoop();

    void initScene();

    void updateMaterials();

    void loadAssets();

    AssetManager assetManager;
    InputHandler inputHandler;

    void handleCameraMovement(const InputEvent& keyEvent);

    std::vector<AssetHandle<Mesh>> meshes;

    Scene scene;

    // TODO: All of this is bad!
    AssetHandle<MaterialInstance> materialHandle;
    AssetHandle<MaterialInstance> skyMaterialHandle;
    glm::vec3 albedo{.5f};
    glm::vec3 f0{.04f};
    glm::vec3 f90{1.f};
    float roughness{.5};
    glm::vec3 emissiveColor{0.f};
    std::optional<TextureImage> skyTexture;
public:
    Application() : materialHandle(assetManager), skyMaterialHandle(assetManager) {}

};
