#pragma once
#define GLM_GTC_QUATERNION
#include <glm/fwd.hpp>
#include <glm/detail/type_quat.hpp>

struct Transform {
	glm::vec3 translation{0.};
	glm::quat rotation{};
	glm::vec3 scale{1.};

	[[nodiscard]] glm::mat4 getMatrix() const;
};
