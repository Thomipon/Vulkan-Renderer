#pragma once

#include "Renderer.hpp"
#include "TextureImage.hpp"
#include "AssetSystem/AssetManager.hpp"
#include "Scene/Scene.hpp"

class Application : public Renderer
{
public:
    void run();

    inline static std::filesystem::path modelPath{"../../VulkanRenderer/Meshes/Mesh.obj"}; // TODO: Awful file management in many ways. These should be in some asset system
    inline static std::filesystem::path texturePath{"../../VulkanRenderer/Textures/Texture.png"};
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
