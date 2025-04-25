#include "Application.hpp"

#include <chrono>
#include <vector>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/gtc/matrix_transform.hpp>

#include "Asset/Mesh.hpp"
#include "Shader.hpp"
#include "Swapchain.hpp"
#include "Vertex.hpp"
#include "Asset/Material.hpp"
#include "Asset/MaterialInstance.hpp"
#include "Scene/Camera.hpp"
#include "Scene/Model.hpp"
#include "ShaderCompilation/ShaderCursor.hpp"

void Application::run()
{
	loadAssets();
	initScene();
	mainLoop();
}

void Application::mainLoop()
{
	while (!window.shouldClose())
	{
		Window::pollEvents();
		drawScene(scene);
	}

	device.waitIdle();
}

void Application::initScene()
{
	window.registerInputHandler(inputHandler);
	//inputHandler.registerKeyCallback(std::bind(handleCameraMovement, this, std::placeholders::_1));

	scene.camera.transform.translation = glm::vec3{0.f, .05f, .2f};
	scene.camera.transform.rotation = glm::quat{radians(glm::vec3{-15.f, 0.f, 0.f})};

	DirectionalLight directionalLight{};
	directionalLight.direction = glm::vec3{1.f, 1.f, 1.f};
	directionalLight.color = glm::vec3{1.f, .8f, .6f};
	directionalLight.intensity = 3.f;
	scene.lightEnvironment.second.first.lights.emplace_back(directionalLight);
	directionalLight.direction = glm::vec3{-1.f, 1.f, 1.f};
	scene.lightEnvironment.second.first.lights.emplace_back(directionalLight);

	auto material{assetManager.createAsset<Material>("BRDF/pbr", "ConstantPBRMaterial")};
	material->compile(compiler, *this);

	scene.models.emplace_back(meshes[0], assetManager.createAsset<MaterialInstance>(material, "constant pbr"));
	Model& model{scene.models[0]};

	ShaderCursor materialCursor{model.material->getShaderCursor().field("gMaterial")};
	materialCursor.field("albedo").write(glm::vec3{.5f, .1f, 1.f});
	materialCursor.field("f0").write(glm::vec3{.02f});
	materialCursor.field("f90").write(glm::vec3{1.f});
	materialCursor.field("roughness").write(glm::vec1{.1f});

	/*ShaderCursor materialCursor{model.material->getShaderCursor().field("gMaterial")};
	materialCursor.field("diffuseColor").write(glm::vec3{.5f, .1f, 1.f});
	materialCursor.field("specularColor").write(glm::vec3{.05f, .5f, 1.f});
	materialCursor.field("specularity").write(glm::vec1{1.f});*/
}

void Application::loadAssets()
{
	static const std::filesystem::path assetBasePath{"../../VulkanRenderer/"}; // TODO: This should be improved as asset locations depend on the working directory
	for (const auto& file : std::filesystem::recursive_directory_iterator{assetBasePath / "Meshes"})
	{
		if (file.is_regular_file() && file.path().extension() == ".obj")
		{
			// TODO: This screams for some soft asset references
			meshes.emplace_back(assetManager.createAsset<Mesh>(*this, file.path()));
		}
	}
}

void Application::handleCameraMovement(const InputEvent& keyEvent)
{
	// TODO: Keys should be encapsulated
	if (keyEvent.type == InputType::keyPress)
	{
		if (keyEvent.action == GLFW_PRESS || keyEvent.action == GLFW_REPEAT)
		{
			static auto startTime{std::chrono::high_resolution_clock::now()};

			const auto currentTime{std::chrono::high_resolution_clock::now()};
			const float time{std::chrono::duration<float>(currentTime - startTime).count()};

			glm::vec3 direction{0.0f, 0.0f, 0.0f};
			switch (keyEvent.key)
			{
			case GLFW_KEY_W:
				direction = glm::vec3{1.0f, 0.0f, 0.0f};
				break;
			case GLFW_KEY_S:
				direction = glm::vec3{-1.0f, 0.0f, 0.0f};
				break;
			case GLFW_KEY_A:
				direction = glm::vec3{0.0f, -1.0f, 0.0f};
				break;
			case GLFW_KEY_D:
				direction = glm::vec3{0.0f, 1.0f, 0.0f};
				break;
			case GLFW_KEY_Q:
				direction = glm::vec3{0.0f, 0.0f, -1.0f};
				break;
			case GLFW_KEY_E:
				direction = glm::vec3{0.0f, 0.0f, 1.0f};
				break;
			default:
				break;
			}

			constexpr float speed = 0.1f;
			direction = mat3_cast(scene.camera.transform.rotation) * direction;

			auto& translation{scene.camera.transform.translation};
			translation += direction * speed * time;
		}
	}
	if (keyEvent.type == InputType::mouseMove)
	{
		glm::vec2 normalizedMousePos{keyEvent.mouseX / swapchain.extent.width, keyEvent.mouseY / swapchain.extent.height};
		normalizedMousePos = 2.f * (normalizedMousePos - glm::vec2{0.5f, 0.5f});
		glm::vec2 angles{glm::radians(normalizedMousePos.x * 180), glm::radians(normalizedMousePos.y * 90)};
		scene.camera.transform.rotation = glm::quat{glm::vec3{angles.y, angles.x, 0.0f}};
		scene.camera.transform.translation = mat3_cast(scene.camera.transform.rotation) * glm::vec3{-.5, 0., 0.};
	}
}
