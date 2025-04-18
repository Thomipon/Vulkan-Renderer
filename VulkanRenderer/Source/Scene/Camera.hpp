﻿#pragma once
#include "Core/Transform.hpp"

class Camera {
public:
	Transform transform{};
	float fieldOfView = 90.f;

	[[nodiscard]] glm::mat4 getViewProjection(const glm::vec2& viewExtent) const;
};

