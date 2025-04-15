#pragma once
#include <map>
#include <typeindex>

#include "AssetArray.hpp"
#include "AssetSystemStructs.h"

template <Asset T>
struct AssetHandle;

class AssetManager
{
public:
	template <Asset T, typename... Args>
	AssetHandle<T> createAsset(Args... args);

private:
	std::map<std::type_index, AssetArray> assets;
	std::map<size_t, AssetInfo> assetInfosByUUID;

	void increaseRefCount(size_t assetUUID);

	template <Asset T>
	void decreaseRefCount(size_t assetUUID);

	UUID createUUID();

	size_t currentUUID{1};

	template <Asset T>
	friend struct AssetHandle;
};

template <Asset T, typename... Args>
AssetHandle<T> AssetManager::createAsset(Args... args)
{
	const std::type_index typeIndex{typeid(T)};
	auto assetArrayIterator{assets.find(typeIndex)};
	if (assetArrayIterator == assets.end())
	{
		const auto [it, success]{assets.emplace(typeIndex, AssetArray::create<T>())};
		if (!success)
		{
			return {};
		}
		assetArrayIterator = it;
	}
	auto& array{assetArrayIterator->second};
	const size_t index{array.size()};
	const UUID uuid{createUUID()};
	const auto[_, success]{assetInfosByUUID.emplace(uuid.value, AssetInfo{uuid, 1, typeIndex, index})};
	if (!success)
	{
		return {};
	}
	array.emplace<T>(std::forward<Args>(args)...).setUUID(uuid);
	return {*this, uuid};
}

template <Asset T>
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
