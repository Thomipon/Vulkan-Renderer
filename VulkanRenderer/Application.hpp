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

    void loadAssets();

    AssetManager assetManager;
    InputHandler inputHandler;

    void handleCameraMovement(const InputEvent& keyEvent);

    std::vector<AssetHandle<Mesh>> meshes;

    Scene scene;
};
