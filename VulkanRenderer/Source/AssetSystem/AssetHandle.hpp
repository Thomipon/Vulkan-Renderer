#pragma once
#include "AssetManager.hpp"
#include "AssetSystemStructs.h"

template <Asset T>
struct AssetHandle
{
public:
	explicit AssetHandle(AssetManager& assetManager);

	T& operator*() const;
	T* operator->() const;

	AssetHandle(const AssetHandle& other);
	AssetHandle(AssetHandle&& other) noexcept;
	AssetHandle& operator=(const AssetHandle& other);
	AssetHandle& operator=(AssetHandle&& other) noexcept;
	~AssetHandle();

private:
	AssetManager& assetManager;

	UUID uuid;

	AssetHandle(AssetManager& assetManager, const UUID& uuid);

	friend AssetManager;
};

template <Asset T>
AssetHandle<T>::AssetHandle(AssetManager& assetManager)
	: assetManager(assetManager)
{
}

template <Asset T>
T& AssetHandle<T>::operator*() const
{
	const AssetInfo assetInfo{assetManager.assetInfosByUUID[uuid.value]};
	return assetManager.assets[assetInfo.type].at<T>(assetInfo.index);
}

template <Asset T>
T* AssetHandle<T>::operator->() const
{
	return **this;
}

template <Asset T>
AssetHandle<T>::AssetHandle(const AssetHandle& other)
	: assetManager(other.assetManager), uuid(other.uuid)
{
	assetManager.increaseRefCount(uuid.value);
}

template <Asset T>
AssetHandle<T>::AssetHandle(AssetHandle&& other) noexcept
	: assetManager(other.assetManager), uuid(other.uuid)
{
	other.uuid = {};
}

template <Asset T>
AssetHandle<T>& AssetHandle<T>::operator=(const AssetHandle& other)
{
	assert(&assetManager == &other.assetManager);
	uuid = other.uuid;
	assetManager.increaseRefCount(uuid.value);
	return *this;
}

template <Asset T>
AssetHandle<T>& AssetHandle<T>::operator=(AssetHandle&& other) noexcept
{
	assert(&assetManager == &other.assetManager);
	uuid = other.uuid;
	other.uuid = {};
	return *this;
}

template <Asset T>
AssetHandle<T>::~AssetHandle()
{
	if (uuid.value != 0)
	{
		assetManager.decreaseRefCount<T>(uuid.value);
	}
}

template <Asset T>
AssetHandle<T>::AssetHandle(AssetManager& assetManager, const UUID& uuid)
	: assetManager(assetManager), uuid(uuid)
{
}
