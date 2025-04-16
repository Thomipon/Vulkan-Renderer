#pragma once

#include <vector>

#include "Asset/Mesh.hpp"
#include "VulkanBackend.hpp"

#include "Renderer.hpp"
#include "TextureImage.hpp"
#include "AssetSystem/AssetManager.hpp"
#include "Scene/Scene.hpp"

class RenderSync;
class Buffer;

class HelloTriangleApplication : public Renderer
{
public:
    void run();

    inline static std::filesystem::path modelPath{"../../VulkanRenderer/Meshes/Mesh.obj"}; // TODO: Awful file management in many ways. These should be in some asset system
    inline static std::filesystem::path texturePath{"../../VulkanRenderer/Textures/Texture.png"};
private:
    void mainLoop();

    void initScene();
    void updateCamera();

    AssetManager assetManager;
    InputHandler inputHandler;

    void handleCameraMovement(const InputEvent& keyEvent);

    Scene scene;
};
