#pragma once
#include <memory>
#include <vector>

#include "Model.hpp"
#include "Light/Light.hpp"

class Camera;

class Scene
{
public:
	std::vector<Model> models;
	std::vector<std::unique_ptr<Light>> lights;
	std::shared_ptr<Camera> camera;
};
