#pragma once
#include <memory>
#include <vector>

class Model;
class Light;

class Scene
{
public:
	std::vector<Model> models;
	std::vector<std::unique_ptr<Light>> lights;
};
