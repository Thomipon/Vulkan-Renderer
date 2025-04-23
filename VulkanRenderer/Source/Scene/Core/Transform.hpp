#pragma once
#define GLM_GTC_QUATERNION
#include <glm/fwd.hpp>
#include <glm/detail/type_quat.hpp>

struct Transform {
	glm::vec3 translation{0.};
	glm::quat rotation{1., 0., 0., 0.};
	glm::vec3 scale{1.};

	[[nodiscard]] glm::mat4 getMatrix() const;

	void drawImGui(bool drawScale = true, bool drawRotation = true, bool drawLocation = true);
};
