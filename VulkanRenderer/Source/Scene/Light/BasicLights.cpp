//
// Created by Thomas on 30/03/2025.
//

#include "BasicLights.hpp"

#include "ShaderCompilation/ShaderCursor.hpp"

void PointLight::writeToCursor(const ShaderCursor& cursor) const
{
	cursor.field("position").write(transform.translation);
	cursor.field("color").write(color);
	cursor.field("intensity").write(intensity);
}

void DirectionalLight::writeToCursor(const ShaderCursor& cursor) const
{
	cursor.field("direction").write(direction);
	cursor.field("color").write(color);
	cursor.field("intensity").write(intensity);
}
