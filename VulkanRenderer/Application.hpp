#pragma once

#include "Renderer.hpp"
#include "AssetSystem/AssetManager.hpp"
#include "Demo/LayeredMaterials/LayeredMaterialsDemo.hpp"
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
    AssetHandle<MaterialInstance> skyMaterialHandle;
    std::optional<TextureImage> skyTexture;
    std::vector<std::unique_ptr<DemoMaterialBase>> materials;
public:
    Application() : skyMaterialHandle(assetManager) {}

};
