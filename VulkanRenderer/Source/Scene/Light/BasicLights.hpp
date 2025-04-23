#pragma once
#include <glm/vec3.hpp>

#include "LightEnvironment.hpp"
#include "Scene/Core/Transform.hpp"

class PointLight : public LightEnvironment
{
public:
	Transform transform;
	glm::vec3 color;
	float intensity;

	IMPLEMENT_LIGHT_TYPE("PointLight")

	virtual void writeToCursor(const ShaderCursor& cursor) const override;
	virtual void drawImGui() override;
};

class DirectionalLight : public LightEnvironment
{
public:
	glm::vec3 direction;
	glm::vec3 color;
	float intensity;

	IMPLEMENT_LIGHT_TYPE("DirectionalLight")

	virtual void writeToCursor(const ShaderCursor& cursor) const override;
	virtual void drawImGui() override;
};
