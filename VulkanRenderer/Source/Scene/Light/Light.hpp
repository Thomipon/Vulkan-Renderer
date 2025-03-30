#pragma once
#include "Scene/Core/Transform.hpp"

class Light {
public:
	Transform transform;

	virtual ~Light() = 0;
};

