#include "Transform.hpp"

#include <glm/gtc/quaternion.hpp>

glm::mat4 Transform::getMatrix() const
{
	return transpose(translate(glm::scale(glm::mat4{1.0f}, scale) * mat4_cast(rotation), translation));
}
