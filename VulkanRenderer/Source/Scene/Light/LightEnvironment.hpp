#pragma once
#include <string>

struct ShaderCursor;

class LightEnvironment
{
public:
	virtual ~LightEnvironment() = 0;
	virtual std::string getLightTypeName() const = 0;
	virtual void writeToCursor(const ShaderCursor& cursor) const = 0;

	virtual void drawImGui() = 0;
};

#define IMPLEMENT_LIGHT_TYPE(lightTypeName) \
	static std::string getLightTypeNameStatic() {return lightTypeName;} \
	virtual std::string getLightTypeName() const override {return getLightTypeNameStatic();}
