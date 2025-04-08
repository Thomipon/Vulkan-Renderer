#pragma once
#include <cstdint>


struct ShaderOffset
{
	size_t byteOffset{0};
	uint32_t bindingIndex{0};
	uint32_t bindingArrayElement{0};
};
