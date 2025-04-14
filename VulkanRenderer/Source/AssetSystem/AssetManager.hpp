#pragma once
#include <map>
#include <typeindex>

#include "AssetArray.hpp"
#include "AssetSystemStructs.h"

class AssetManager
{
private:
	std::map<std::type_index, AssetArray> assets;
	std::map<size_t, AssetInfo> assetInfosByUUID;

	void increaseRefCount(size_t assetUUID);

	template <typename T>
	void decreaseRefCount(size_t assetUUID);

	template <typename T>
	friend struct AssetHandle;
};

template <typename T>
void AssetManager::decreaseRefCount(const size_t assetUUID)
{
	if (assetUUID != 0)
	{
		auto& assetInfo{assetInfosByUUID.find(assetUUID)->second};
		assetInfo.refCount -= 1;
		if (assetInfo.refCount == 0)
		{
			const UUID lastAssetUUID{assets[assetInfo.type].destruct<T>(assetInfo.index)};
			auto& lastAssetInfo{assetInfosByUUID.find(lastAssetUUID.value)->second};
			lastAssetInfo.index = assetInfo.index;
		}
	}
}
