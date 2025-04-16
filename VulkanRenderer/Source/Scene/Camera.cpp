//
// Created by Thomas on 30/03/2025.
//

#include "Camera.hpp"

glm::mat4 Camera::getViewProjection(const glm::vec2& viewExtent) const
{
	const glm::mat4 view = inverse(transform.getMatrix());
	glm::mat4 projection = glm::perspective(glm::radians(fieldOfView), viewExtent.x / viewExtent.y, 0.1f, 100.f);
	projection[1][1] *= -1;
	return projection * view;
}
