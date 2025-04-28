#pragma once
#include <glm/vec3.hpp>

#include "LightEnvironment.hpp"
#include "TextureImage.hpp"
#include "Scene/Core/Transform.hpp"

class PointLight : public LightEnvironment
{
public:
	Transform transform;
	glm::vec3 color{1.f};
	float intensity{.5f};

	IMPLEMENT_LIGHT_TYPE("PointLight")

	virtual void writeToCursor(const ShaderCursor& cursor) const override;
	virtual void drawImGui() override;
};

class DirectionalLight : public LightEnvironment
{
public:
	glm::vec3 direction{1.f, 0.f, 0.f};
	glm::vec3 color{1.f};
	float intensity{.5f};

	IMPLEMENT_LIGHT_TYPE("DirectionalLight")

	virtual void writeToCursor(const ShaderCursor& cursor) const override;
	virtual void drawImGui() override;
};

class AmbientLight : public LightEnvironment
{
public:
	glm::vec3 color{1.f};
	float intensity{.1f};

	IMPLEMENT_LIGHT_TYPE("AmbientLight")

	virtual void writeToCursor(const ShaderCursor& cursor) const override;
	virtual void drawImGui() override;
};

class AmbientCubemapLight : public LightEnvironment
{
public:
	std::optional<TextureImage> cubemap;
	float intensity{.1f};

	IMPLEMENT_LIGHT_TYPE("AmbientCubemapLight")

	virtual void writeToCursor(const ShaderCursor& cursor) const override;
	virtual void drawImGui() override;
};
