#include "HelloTriangleApplication.hpp"

#include <chrono>
#include <vector>

#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Buffer.hpp"
#include "check.hpp"
#include "Asset/Mesh.hpp"
#include "Shader.hpp"
#include "ShaderCompiler.hpp"
#include "Swapchain.hpp"
#include "Uniforms.hpp"
#include "Vertex.hpp"
#include "Asset/Material.hpp"
#include "Asset/MaterialInstance.hpp"
#include "Renderer/RenderSync.hpp"
#include "Scene/Camera.hpp"
#include "Scene/Model.hpp"
#include "ShaderCompilation/ShaderCursor.hpp"

void HelloTriangleApplication::run()
{
	initScene();
	mainLoop();
}

void HelloTriangleApplication::mainLoop()
{
	while (!window.shouldClose())
	{
		Window::pollEvents();
		updateCamera();
		drawScene(scene);
	}

	device.waitIdle();
}

void HelloTriangleApplication::initScene()
{
	window.registerInputHandler(inputHandler);
	inputHandler.registerKeyCallback(std::bind(handleCameraMovement, this, std::placeholders::_1));

	auto material{assetManager.createAsset<Material>("BRDF/phong", "ConstantPhongMaterial")};
	material->compile(compiler, *this);

	scene.models.emplace_back(assetManager.createAsset<Mesh>(*this, modelPath), assetManager.createAsset<MaterialInstance>(material));
	Model& model{scene.models[0]};

	model.transform.translation = glm::vec3{0.0f, 0.f, 0.5f};

	ShaderCursor materialCursor{model.material->getShaderCursor().field("gMaterial")};
	materialCursor.field("diffuseColor").write(glm::vec3{.5f, .1f, 1.f});
	materialCursor.field("specularColor").write(glm::vec3{.05f, .5f, 1.f});
	materialCursor.field("specularity").write(glm::vec1{1.f});
}

void HelloTriangleApplication::updateCamera()
{
	static auto startTime{std::chrono::high_resolution_clock::now()};

	const auto currentTime{std::chrono::high_resolution_clock::now()};
	const float time{std::chrono::duration<float>(currentTime - startTime).count()};

	//scene.camera->transform.translation = rotate(glm::vec3{5.f, 0.f, 0.f}, time * glm::radians(90.f), glm::vec3{0.0f, 0.0f, 1.0f});
	//scene.camera->transform.translation = glm::vec3{-2.0f, 0.0f, 0.0f};
	//scene.camera->transform.rotation = quatLookAt(glm::vec3{2.0f} - glm::vec3{0.f}, glm::vec3{0.f, 0.f, 1.f});
	//scene.camera->transform.rotation = quatLookAt(glm::vec3{2.0f, 0.0f, 0.0f}, glm::vec3{0.f, 0.f, 1.f});
	//scene.camera.transform.rotation = glm::rotate(glm::quat{1.f, 0.f, 0.f, 0.f}, glm::radians(time * 180.f), glm::vec3{0.f, 0.f, 1.f});
}

void HelloTriangleApplication::handleCameraMovement(const InputEvent& keyEvent)
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
	}
}
