#pragma once
#include <vector>

#include "Camera.hpp"
#include "Model.hpp"
#include "Light/Light.hpp"

class Scene
{
public:
	std::vector<Model> models;
	std::vector<Light> lights;
	Camera camera;
};
