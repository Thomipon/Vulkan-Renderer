#pragma once
#include <vector>

#include "Camera.hpp"
#include "Model.hpp"
#include "Light/UniversalLightEnvironment.hpp"

class Scene
{
public:
	std::vector<Model> models;
	UniversalLightEnvironment lightEnvironment;
	Camera camera;
};
