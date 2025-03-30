#pragma once
#include <glm/vec3.hpp>

#include "Light.hpp"

class PointLight : public Light{

public:
	float intensity;
	glm::vec3 color;
};

