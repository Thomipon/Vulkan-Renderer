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

		imGui.newFrame();

		ImGui::Begin("Settings");

		scene.drawImGui();

		updateMaterials();

		ImGui::End();

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

	PointLight pointLight{};
	pointLight.transform.translation = glm::vec3{0.28f, -.19f, .05f};
	pointLight.color = glm::vec3{.64f, 1.f, .67f};
	scene.lightEnvironment.first.lights.emplace_back(pointLight);
	DirectionalLight directionalLight{};
	directionalLight.direction = glm::vec3{1.f, 2.f, 1.f};
	directionalLight.color = glm::vec3{.6f, .95f, 1.f};
	directionalLight.intensity = 3.f;
	scene.lightEnvironment.second.first.lights.emplace_back(directionalLight);
	directionalLight.direction = glm::vec3{-2.f, -.7f, 1.f};
	directionalLight.color = glm::vec3{.99f, .85f, .7f};
	scene.lightEnvironment.second.first.lights.emplace_back(directionalLight);

	scene.lightEnvironment.second.second.cubemap = TextureImage{"../../VulkanRenderer/Textures/Cubemap.png", vk::ImageViewType::eCube, *this};

	auto material{assetManager.createAsset<Material>("BRDF/pbr", "ConstantPBRMaterial")};
	material->compile(compiler, *this);

	materialHandle = assetManager.createAsset<MaterialInstance>(material, "constant pbr");

	scene.models.emplace_back(meshes[0], materialHandle);

	ShaderCursor materialCursor{materialHandle->getShaderCursor().field("gMaterial")};
	materialCursor.field("albedo").write(albedo);
	materialCursor.field("f0").write(f0);
	materialCursor.field("f90").write(f90);
	materialCursor.field("roughness").write(glm::vec1{roughness});
	materialCursor.field("emissiveColor").write(emissiveColor);

	auto skyMaterial{assetManager.createAsset<Material>("BRDF/basicBRDFs", "ConstantUnlitMaterial")};
	//auto skyMaterial{assetManager.createAsset<Material>("Materials/basicMaterials", "SkySphereMaterial")};
	skyMaterial->compile(compiler, *this);

	skyMaterialHandle = assetManager.createAsset<MaterialInstance>(skyMaterial, "sky material");
	scene.models.emplace_back(meshes[3], skyMaterialHandle);
	skyMaterialHandle->getShaderCursor().printLayout();
	ShaderCursor skyMaterialCursor{skyMaterialHandle->getShaderCursor().field("gMaterial")};
	//skyMaterialCursor.field("cubemap").write(TextureImage{"../../VulkanRenderer/Textures/Cubemap.png", vk::ImageViewType::eCube, *this}); // TODO: This should be shared with the above
	//skyMaterialCursor.field("emissiveIntensity").write(glm::vec1{1.f});
	skyMaterialCursor.field("emissive").write(glm::vec3{10.f});
}

void Application::updateMaterials()
{
	ImGui::SeparatorText("Materials");

	ShaderCursor materialCursor{materialHandle->getShaderCursor().field("gMaterial")};

	ImGui::Text("Albedo:");
	if (ImGui::ColorEdit3("##albedo", reinterpret_cast<float*>(&albedo)))
	{
		materialCursor.field("albedo").write(albedo);
	}

	ImGui::Text("F0:");
	if (ImGui::ColorEdit3("##f0", reinterpret_cast<float*>(&f0)))
	{
		materialCursor.field("f0").write(f0);
	}

	ImGui::Text("F90:");
	if (ImGui::ColorEdit3("##f90", reinterpret_cast<float*>(&f90)))
	{
		materialCursor.field("f90").write(f90);
	}

	ImGui::Text("Roughness:");
	if (ImGui::SliderFloat("##roughness", &roughness, 0.f, 1.f))
	{
		materialCursor.field("roughness").write(glm::vec1{roughness});
	}

	ImGui::Text("Emissive:");
	if (ImGui::ColorEdit3("##emissive", reinterpret_cast<float*>(&emissiveColor), ImGuiColorEditFlags_HDR))
	{
		materialCursor.field("emissiveColor").write(emissiveColor);
	}
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
