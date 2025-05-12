#pragma once
#include <cstdint>
#include <typeindex>

struct UUID
{
	size_t value{0};
};

struct AssetInfo
{
	AssetInfo(const UUID& uuid, const uint32_t refCount, const std::type_index& type, const size_t index)
		: uuid(uuid),
		  refCount(refCount),
		  type(type),
		  index(index)
	{
	}

	UUID uuid;
	uint32_t refCount;
	std::type_index type;
	size_t index;
};
