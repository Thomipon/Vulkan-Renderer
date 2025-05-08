#pragma once
#include "Core/Transform.hpp"

class Camera {
public:
	Transform transform{};
	float fieldOfView = 70.f;
    float exposureValue = 10.f;

	[[nodiscard]] glm::mat4 getViewProjection(const glm::vec2& viewExtent) const;

	void drawImGui();
};

