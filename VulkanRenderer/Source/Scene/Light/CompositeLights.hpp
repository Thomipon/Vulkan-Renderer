#pragma once

#include <concepts>
#include <cassert>
#include <imgui.h>

#include "LightEnvironment.hpp"
#include "ShaderCompilation/ShaderCursor.hpp"

template <typename T>
concept LightEnv = std::derived_from<T, LightEnvironment> && requires() { T::getLightTypeNameStatic(); };

class EmptyLight : public LightEnvironment
{
public:
	IMPLEMENT_LIGHT_TYPE("EmptyLight")

	virtual void writeToCursor(const ShaderCursor& cursor) const override;
	virtual void drawImGui() override {}
};

template <LightEnv L1, LightEnv L2>
class LightPair : public LightEnvironment
{
public:
	L1 first;
	L2 second;

	IMPLEMENT_LIGHT_TYPE("LightPair<" + L1::getLightTypeNameStatic() + ',' + L2::getLightTypeNameStatic() + '>')

	virtual void writeToCursor(const ShaderCursor& cursor) const override;
	virtual void drawImGui() override;
};

template <LightEnv L, int n>
class LightArray : public LightEnvironment
{
public:
	std::vector<L> lights;

	IMPLEMENT_LIGHT_TYPE("LightArray<" + L::getLightTypeNameStatic() + "," + std::to_string(n) + ">")

	virtual void writeToCursor(const ShaderCursor& cursor) const override;
	virtual void drawImGui() override;
};

template <LightEnv L1, LightEnv L2>
void LightPair<L1, L2>::writeToCursor(const ShaderCursor& cursor) const
{
	first.writeToCursor(cursor.field("first"));
	second.writeToCursor(cursor.field("second"));
}

template<LightEnv L1, LightEnv L2>
void LightPair<L1, L2>::drawImGui()
{
	ImGui::PushID(0);
	first.drawImGui();
	ImGui::PopID();
	ImGui::PushID(1);
	second.drawImGui();
	ImGui::PopID();
}

template <LightEnv L, int n>
void LightArray<L, n>::writeToCursor(const ShaderCursor& cursor) const
{
	assert(lights.size() <= n);

	cursor.field("count").write(lights.size());
	const auto lightCursor{cursor.field("lights")};
	for (int i = 0; i < lights.size(); ++i)
	{
		lights[i].writeToCursor(lightCursor.element(i));
	}
}

template<LightEnv L, int n>
void LightArray<L, n>::drawImGui()
{
	for (int i = 0; i < lights.size(); ++i)
	{
		ImGui::PushID(i);
		lights[i].drawImGui();
		ImGui::PopID();
	}
}
