#pragma once
#include <cstdint>
#include <typeindex>

template <typename T>
concept Asset = std::derived_from<T, class AssetBase>;

struct UUID
{
	size_t value{0};
};

struct AssetInfo
{
	AssetInfo(const UUID& uuid, const uint32_t refCount, const std::type_index& type, const uint32_t index)
		: uuid(uuid),
		  refCount(refCount),
		  type(type),
		  index(index)
	{
	}

	UUID uuid;
	uint32_t refCount;
	std::type_index type;
	uint32_t index;
};
